#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "x11_window.h"
#include "gl_init.h"
#include "gl_utils.h"
#include <GL/glx.h>

typedef struct uGLESXContext {
	X11Window *win;
} uGLESXContext;

static uGLESXContext *current_ctx;

Bool glXQueryExtension(Display *dpy, int *errorb, int *event)
{
	(void)dpy;
	(void)errorb;
	(void)event;
	return True;
}

XVisualInfo *glXChooseVisual(Display *dpy, int screen, int *attribList)
{
	(void)attribList;
	XVisualInfo vinfo;
	int n;
	if (!XMatchVisualInfo(dpy, screen, 24, TrueColor, &vinfo))
		return NULL;
	return XGetVisualInfo(dpy, VisualIDMask, &vinfo, &n);
}

GLXContext glXCreateContext(Display *dpy, XVisualInfo *vis,
			    GLXContext shareList, Bool direct)
{
	(void)dpy;
	(void)vis;
	(void)shareList;
	(void)direct;
	uGLESXContext *ctx = tracked_malloc(sizeof(uGLESXContext));
	if (!ctx)
		return NULL;
	ctx->win = NULL;
	return (GLXContext)ctx;
}

void glXDestroyContext(Display *dpy, GLXContext ctx)
{
	(void)dpy;
	uGLESXContext *c = (uGLESXContext *)ctx;
	tracked_free(c, sizeof(uGLESXContext));
	if (current_ctx == c)
		current_ctx = NULL;
}

Bool glXMakeCurrent(Display *dpy, GLXDrawable drawable, GLXContext ctx)
{
	(void)dpy;
	uGLESXContext *c = (uGLESXContext *)ctx;
	c->win = (X11Window *)(uintptr_t)drawable;
	current_ctx = c;
	return True;
}

void glXSwapBuffers(Display *dpy, GLXDrawable drawable)
{
	(void)dpy;
	(void)drawable;
	if (!current_ctx || !current_ctx->win)
		return;
	x11_window_show_image(current_ctx->win, GL_get_default_framebuffer());
}

void glXCopyContext(Display *dpy, GLXContext src, GLXContext dst, GLuint mask)
{
	(void)dpy;
	(void)src;
	(void)dst;
	(void)mask;
}

GLXPixmap glXCreateGLXPixmap(Display *dpy, XVisualInfo *visual, Pixmap pixmap)
{
	(void)dpy;
	(void)visual;
	return (GLXPixmap)pixmap;
}

void glXDestroyGLXPixmap(Display *dpy, GLXPixmap pixmap)
{
	(void)dpy;
	(void)pixmap;
}

Bool glXQueryVersion(Display *dpy, int *maj, int *min)
{
	(void)dpy;
	if (maj)
		*maj = 1;
	if (min)
		*min = 1;
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
	(void)visual;
	(void)attrib;
	(void)value;
	return 0;
}

GLXContext glXGetCurrentContext(void)
{
	return (GLXContext)current_ctx;
}

GLXDrawable glXGetCurrentDrawable(void)
{
	return current_ctx ? (GLXDrawable)(uintptr_t)current_ctx->win : 0;
}

void glXWaitGL(void)
{
}
void glXWaitX(void)
{
}

void glXUseXFont(Font font, int first, int count, int list)
{
	(void)font;
	(void)first;
	(void)count;
	(void)list;
}

const char *glXQueryExtensionsString(Display *dpy, int screen)
{
	(void)dpy;
	(void)screen;
	return "";
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
	(void)name;
	return "";
}
