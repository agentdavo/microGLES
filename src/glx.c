#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "x11_window.h"
#include "gl_init.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include <GL/glx.h>
#include <pthread.h>

typedef struct uGLESXContext {
	X11Window *win;
	Display *display;
	Bool double_buffered;
} uGLESXContext;

static uGLESXContext *current_ctx = NULL;
static pthread_mutex_t ctx_mutex = PTHREAD_MUTEX_INITIALIZER;
static int dump_counter = 0;

Bool glXQueryExtension(Display *dpy, int *errorb, int *event)
{
	(void)dpy;
	(void)errorb;
	(void)event;
	return True;
}

XVisualInfo *glXChooseVisual(Display *dpy, int screen, int *attribList)
{
	XVisualInfo vinfo;
	int n;
	int depth = 24;
	int class = TrueColor;

	if (attribList) {
		for (int *attr = attribList; *attr; attr++) {
			switch (*attr) {
			case GLX_RGBA:
				class = TrueColor;
				break;
			case GLX_DEPTH_SIZE:
				attr++;
				depth = *attr;
				break;
			case GLX_DOUBLEBUFFER:
				/* double buffering unsupported */
				break;
			case None:
				break;
			}
		}
	}

	if (!XMatchVisualInfo(dpy, screen, depth, class, &vinfo)) {
		LOG_ERROR("No matching visual for depth=%d, class=%d", depth,
			  class);
		return NULL;
	}

	XVisualInfo *ret = XGetVisualInfo(dpy, VisualIDMask, &vinfo, &n);
	if (!ret) {
		LOG_ERROR("XGetVisualInfo failed");
	}
	return ret;
}

GLXContext glXCreateContext(Display *dpy, XVisualInfo *vis,
			    GLXContext shareList, Bool direct)
{
	(void)vis;
	(void)shareList;
	(void)direct;
	if (!dpy) {
		LOG_ERROR("glXCreateContext: NULL display");
		return NULL;
	}

	uGLESXContext *ctx = tracked_malloc(sizeof(uGLESXContext));
	if (!ctx) {
		LOG_ERROR("glXCreateContext: Failed to allocate context");
		return NULL;
	}

	ctx->win = NULL;
	ctx->display = dpy;
	ctx->double_buffered = False; // Set based on attribList or config
	LOG_DEBUG("Created GLX context %p", ctx);
	return (GLXContext)ctx;
}

void glXDestroyContext(Display *dpy, GLXContext ctx)
{
	(void)dpy;
	if (!ctx) {
		return;
	}

	pthread_mutex_lock(&ctx_mutex);
	uGLESXContext *c = (uGLESXContext *)ctx;
	if (c == current_ctx) {
		current_ctx = NULL;
	}
	tracked_free(c, sizeof(uGLESXContext));
	pthread_mutex_unlock(&ctx_mutex);
	LOG_DEBUG("Destroyed GLX context %p", ctx);
}

Bool glXMakeCurrent(Display *dpy, GLXDrawable drawable, GLXContext ctx)
{
	(void)dpy;
	uGLESXContext *c = (uGLESXContext *)ctx;
	if (!c || !drawable) {
		LOG_ERROR("glXMakeCurrent: Invalid context or drawable");
		return False;
	}

	c->win = (X11Window *)(uintptr_t)drawable;
	c->display = x11_window_get_display(c->win);
	if (!c->display) {
		LOG_ERROR("glXMakeCurrent: Invalid X11Window drawable");
		return False;
	}

	pthread_mutex_lock(&ctx_mutex);
	current_ctx = c;
	pthread_mutex_unlock(&ctx_mutex);
	LOG_DEBUG("Made context %p current on drawable %p", ctx,
		  (void *)drawable);
	return True;
}

void glXSwapBuffers(Display *dpy, GLXDrawable drawable)
{
	(void)dpy;
	(void)drawable;
	pthread_mutex_lock(&ctx_mutex);
	if (!current_ctx || !current_ctx->win || !current_ctx->display) {
		LOG_ERROR("glXSwapBuffers: No current context or window");
		pthread_mutex_unlock(&ctx_mutex);
		return;
	}

	Framebuffer *fb = GL_get_default_framebuffer();
	if (!fb) {
		LOG_ERROR("glXSwapBuffers: No default framebuffer");
		pthread_mutex_unlock(&ctx_mutex);
		return;
	}

	if (current_ctx->double_buffered) {
		GL_swap_buffers();
	}

	if (dump_counter < 2) {
		char fb_path[64];
		snprintf(fb_path, sizeof(fb_path), "framebuffer_%d.bmp",
			 dump_counter);
		LOG_DEBUG("Attempting to save %s", fb_path);
		if (!framebuffer_write_bmp(fb, fb_path)) {
			LOG_ERROR("Failed to save %s", fb_path);
		} else {
			uint32_t c = framebuffer_get_pixel(fb, 0, 0);
			LOG_INFO("Saved %s first pixel 0x%08X", fb_path, c);
		}

		char win_path[64];
		snprintf(win_path, sizeof(win_path), "window_%d.bmp",
			 dump_counter);
		LOG_DEBUG("Attempting to save %s", win_path);
		if (!x11_window_save_bmp(current_ctx->win, win_path)) {
			LOG_ERROR("Failed to save %s", win_path);
		}
		++dump_counter;
	}

	x11_window_show_image(current_ctx->win, fb);
	pthread_mutex_unlock(&ctx_mutex);
}

void glXCopyContext(Display *dpy, GLXContext src, GLXContext dst, GLuint mask)
{
	(void)dpy;
	(void)src;
	(void)dst;
	(void)mask;
	LOG_WARN("glXCopyContext: Not implemented");
}

GLXPixmap glXCreateGLXPixmap(Display *dpy, XVisualInfo *visual, Pixmap pixmap)
{
	(void)dpy;
	(void)visual;
	LOG_DEBUG("glXCreateGLXPixmap: Returning pixmap %lu", pixmap);
	return (GLXPixmap)pixmap;
}

void glXDestroyGLXPixmap(Display *dpy, GLXPixmap pixmap)
{
	(void)dpy;
	(void)pixmap;
	LOG_DEBUG("glXDestroyGLXPixmap: Pixmap %lu", pixmap);
}

Bool glXQueryVersion(Display *dpy, int *maj, int *min)
{
	(void)dpy;
	if (maj) {
		*maj = 1;
	}
	if (min) {
		*min = 4;
	}
	return True;
}

Bool glXIsDirect(Display *dpy, GLXContext ctx)
{
	(void)dpy;
	(void)ctx;
	return True;
}

int glXGetConfig(Display *dpy, XVisualInfo *visual, int attrib, int *value)
{
	(void)dpy;
	if (!visual || !value) {
		return GLX_BAD_VALUE;
	}

	switch (attrib) {
	case GLX_RGBA:
		*value = True;
		return 0;
	case GLX_DEPTH_SIZE:
		*value = 24;
		return 0;
	case GLX_DOUBLEBUFFER:
		*value = False; // Update if double buffering is supported
		return 0;
	default:
		return GLX_BAD_ATTRIBUTE;
	}
}

GLXContext glXGetCurrentContext(void)
{
	pthread_mutex_lock(&ctx_mutex);
	GLXContext ctx = (GLXContext)current_ctx;
	pthread_mutex_unlock(&ctx_mutex);
	return ctx;
}

GLXDrawable glXGetCurrentDrawable(void)
{
	pthread_mutex_lock(&ctx_mutex);
	GLXDrawable drawable =
		current_ctx ? (GLXDrawable)(uintptr_t)current_ctx->win : 0;
	pthread_mutex_unlock(&ctx_mutex);
	return drawable;
}

void glXWaitGL(void)
{
	// Assume GL_finish exists in gl_init.h
	GL_finish();
}

void glXWaitX(void)
{
	pthread_mutex_lock(&ctx_mutex);
	if (current_ctx && current_ctx->display) {
		XSync(current_ctx->display, False);
	}
	pthread_mutex_unlock(&ctx_mutex);
}

void glXUseXFont(Font font, int first, int count, int list)
{
	(void)font;
	(void)first;
	(void)count;
	(void)list;
	LOG_WARN("glXUseXFont: Not implemented");
}

const char *glXQueryExtensionsString(Display *dpy, int screen)
{
	(void)dpy;
	(void)screen;
	return "GLX_MICROGLES"; // Custom extension for microGLES
}

const char *glXQueryServerString(Display *dpy, int screen, int name)
{
	(void)dpy;
	(void)screen;
	(void)name;
	return "";
}

const char *glXGetClientString(Display *dpy, int name)
{
	(void)dpy;
	switch (name) {
	case GLX_VENDOR:
		return "microGLES";
	case GLX_VERSION:
		return "1.4";
	case GLX_EXTENSIONS:
		return "GLX_MICROGLES";
	default:
		return "";
	}
}
