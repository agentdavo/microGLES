#include "x11_window.h"
#include "pipeline/gl_framebuffer.h"
#include "gl_logger.h"
#include "gl_init.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct X11Window {
	Display *display;
	Window window;
	GC gc;
	XImage *image;
	XShmSegmentInfo shm_info; // For shared memory (optional)
	Bool use_shm;
	unsigned width;
	unsigned height;
	unsigned rshift;
	unsigned gshift;
	unsigned bshift;
};

static pthread_mutex_t x11_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool threads_initialized = false;

// Creates an X11 window with the specified dimensions and title.
X11Window *x11_window_create(unsigned width, unsigned height, const char *title)
{
	if (width == 0 || height == 0 || width > 16384 || height > 16384) {
		LOG_ERROR("Invalid window dimensions: %ux%u", width, height);
		return NULL;
	}

	pthread_mutex_lock(&x11_mutex);
	if (!threads_initialized) {
		if (!XInitThreads()) {
			LOG_WARN(
				"XInitThreads failed; X11 may not be thread-safe");
		}
		threads_initialized = true;
	}
	Display *dpy = XOpenDisplay(NULL);
	if (!dpy) {
		const char *display_env = getenv("DISPLAY");
		LOG_ERROR("XOpenDisplay failed. DISPLAY=%s",
			  display_env ? display_env : "unset");
		pthread_mutex_unlock(&x11_mutex);
		return NULL;
	}

	int screen = DefaultScreen(dpy);
	Visual *visual = DefaultVisual(dpy, screen);
	int depth = DefaultDepth(dpy, screen);
	Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen), 0, 0,
					 width, height, 0,
					 BlackPixel(dpy, screen),
					 WhitePixel(dpy, screen));

	// Set window title with length limit
	char title_buf[257];
	if (title && strlen(title) > 256) {
		LOG_WARN("Window title too long, truncating");
		strncpy(title_buf, title, 256);
		title_buf[256] = '\0';
		XStoreName(dpy, win, title_buf);
	} else {
		XStoreName(dpy, win, title ? title : "microGLES");
	}

	// Enable events
	XSelectInput(dpy, win,
		     ExposureMask | KeyPressMask | StructureNotifyMask);
	Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wm_delete, 1);

	GC gc = XCreateGC(dpy, win, 0, NULL);
	XMapWindow(dpy, win);

	X11Window *w = (X11Window *)malloc(sizeof(X11Window));
	if (!w) {
		LOG_ERROR("Failed to allocate X11Window");
		XFreeGC(dpy, gc);
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
		pthread_mutex_unlock(&x11_mutex);
		return NULL;
	}

	// Initialize struct
	w->display = dpy;
	w->window = win;
	w->gc = gc;
	w->width = width;
	w->height = height;
	w->use_shm = False;
	memset(&w->shm_info, 0, sizeof(w->shm_info));

	// Check for XShm support
	int shm_major, shm_minor;
	Bool shm_pixmaps;
	if (XShmQueryVersion(dpy, &shm_major, &shm_minor, &shm_pixmaps)) {
		w->use_shm = True;
	}

	// Allocate image data
	char *image_data = malloc(width * height * 4);
	if (!image_data) {
		LOG_ERROR("Failed to allocate image data");
		free(w);
		XFreeGC(dpy, gc);
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
		pthread_mutex_unlock(&x11_mutex);
		return NULL;
	}

	// Create XImage (try XShm first)
	if (w->use_shm) {
		w->image = XShmCreateImage(dpy, visual, depth, ZPixmap, NULL,
					   &w->shm_info, width, height);
		if (w->image) {
			w->shm_info.shmid = shmget(IPC_PRIVATE,
						   width * height * 4,
						   IPC_CREAT | 0777);
			if (w->shm_info.shmid != -1) {
				w->shm_info.shmaddr =
					shmat(w->shm_info.shmid, NULL, 0);
				if (w->shm_info.shmaddr != (char *)-1) {
					w->image->data = w->shm_info.shmaddr;
					XShmAttach(dpy, &w->shm_info);
				} else {
					w->use_shm = False;
					XDestroyImage(w->image);
					w->image = NULL;
				}
			} else {
				w->use_shm = False;
				XDestroyImage(w->image);
				w->image = NULL;
			}
		} else {
			w->use_shm = False;
		}
	}

	if (!w->use_shm) {
		w->image = XCreateImage(dpy, visual, depth, ZPixmap, 0,
					image_data, width, height, 32, 0);
		if (!w->image) {
			LOG_ERROR("XCreateImage failed");
			free(image_data);
			free(w);
			XFreeGC(dpy, gc);
			XDestroyWindow(dpy, win);
			XCloseDisplay(dpy);
			pthread_mutex_unlock(&x11_mutex);
			return NULL;
		}
	}

	// Validate color masks
	if (!w->image->red_mask || !w->image->green_mask ||
	    !w->image->blue_mask) {
		LOG_ERROR("Invalid color masks");
		if (w->use_shm && w->image->data) {
			XShmDetach(dpy, &w->shm_info);
			shmdt(w->shm_info.shmaddr);
			shmctl(w->shm_info.shmid, IPC_RMID, NULL);
		}
		XDestroyImage(w->image);
		free(w);
		XFreeGC(dpy, gc);
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
		pthread_mutex_unlock(&x11_mutex);
		return NULL;
	}

	w->rshift = __builtin_ctz(w->image->red_mask);
	w->gshift = __builtin_ctz(w->image->green_mask);
	w->bshift = __builtin_ctz(w->image->blue_mask);

	XFlush(dpy);
	pthread_mutex_unlock(&x11_mutex);
	return w;
}

// Destroys the X11 window and its resources.
void x11_window_destroy(X11Window *w)
{
	if (!w) {
		return;
	}

	pthread_mutex_lock(&x11_mutex);
	if (w->image) {
		if (w->use_shm) {
			XShmDetach(w->display, &w->shm_info);
			if (w->shm_info.shmaddr) {
				shmdt(w->shm_info.shmaddr);
			}
			if (w->shm_info.shmid != -1) {
				shmctl(w->shm_info.shmid, IPC_RMID, NULL);
			}
		}
		XDestroyImage(w->image);
	}
	if (w->gc) {
		XFreeGC(w->display, w->gc);
	}
	if (w->window) {
		XDestroyWindow(w->display, w->window);
	}
	if (w->display) {
		XCloseDisplay(w->display);
	}
	free(w);
	pthread_mutex_unlock(&x11_mutex);
}

// Renders the framebuffer to the window.
void x11_window_show_image(X11Window *w, const struct Framebuffer *fb)
{
	if (!w || !fb) {
		return;
	}

	pthread_mutex_lock(&x11_mutex);
	unsigned width = w->width < fb->width ? w->width : fb->width;
	unsigned height = w->height < fb->height ? w->height : fb->height;

	// Optimize for common pixel formats
	if (w->rshift == 0 && w->gshift == 8 && w->bshift == 16 &&
	    w->image->bits_per_pixel == 32) {
		// Direct copy if formats match
		for (unsigned y = 0; y < height; ++y) {
			memcpy(w->image->data + y * w->image->bytes_per_line,
			       fb->color_buffer + y * fb->width, width * 4);
		}
	} else {
		// Per-pixel conversion
		for (unsigned y = 0; y < height; ++y) {
			for (unsigned x = 0; x < width; ++x) {
				uint32_t pixel =
					fb->color_buffer[y * fb->width + x];
				unsigned char r = pixel & 0xFF;
				unsigned char g = (pixel >> 8) & 0xFF;
				unsigned char b = (pixel >> 16) & 0xFF;
				unsigned char *dst =
					(unsigned char *)w->image->data +
					(y * w->image->bytes_per_line) + x * 4;
				uint32_t out = ((uint32_t)r << w->rshift) |
					       ((uint32_t)g << w->gshift) |
					       ((uint32_t)b << w->bshift);
				memcpy(dst, &out, 4);
			}
		}
	}

	if (w->use_shm) {
		XShmPutImage(w->display, w->window, w->gc, w->image, 0, 0, 0, 0,
			     width, height, False);
	} else {
		XPutImage(w->display, w->window, w->gc, w->image, 0, 0, 0, 0,
			  width, height);
	}
	XFlush(w->display);
	pthread_mutex_unlock(&x11_mutex);
}

// Returns the X11 display.
Display *x11_window_get_display(const X11Window *w)
{
	return w ? w->display : NULL;
}

// Checks if the window contains non-monochrome content.
bool x11_window_has_non_monochrome(const X11Window *w)
{
	if (!w) {
		return false;
	}

	pthread_mutex_lock(&x11_mutex);
	XImage *img = XGetImage(w->display, w->window, 0, 0, w->width,
				w->height, AllPlanes, ZPixmap);
	if (!img) {
		pthread_mutex_unlock(&x11_mutex);
		return false;
	}

	bool non_white = false;
	bool non_black = false;
	for (int y = 0; y < img->height && !(non_white && non_black); ++y) {
		for (int x = 0; x < img->width; ++x) {
			unsigned long p = XGetPixel(img, x, y);
			unsigned int rgb = ((p >> 16) & 0xFF) << 16 |
					   ((p >> 8) & 0xFF) << 8 | (p & 0xFF);
			if (rgb != 0xFFFFFFu) {
				non_white = true;
			}
			if (rgb != 0x000000u) {
				non_black = true;
			}
		}
	}
	XDestroyImage(img);
	pthread_mutex_unlock(&x11_mutex);
	return non_white && non_black;
}

// Saves the current window contents to a BMP file.
int x11_window_save_bmp(const X11Window *w, const char *path)
{
	if (!w || !path) {
		return 0;
	}

	pthread_mutex_lock(&x11_mutex);
	XImage *img = XGetImage(w->display, w->window, 0, 0, w->width,
				w->height, AllPlanes, ZPixmap);
	if (!img) {
		pthread_mutex_unlock(&x11_mutex);
		return 0;
	}

	Framebuffer *fb =
		framebuffer_create((uint32_t)img->width, (uint32_t)img->height);
	if (!fb) {
		XDestroyImage(img);
		pthread_mutex_unlock(&x11_mutex);
		return 0;
	}

	unsigned rshift = __builtin_ctz(img->red_mask);
	unsigned gshift = __builtin_ctz(img->green_mask);
	unsigned bshift = __builtin_ctz(img->blue_mask);
	for (int y = 0; y < img->height; ++y) {
		for (int x = 0; x < img->width; ++x) {
			unsigned long p = XGetPixel(img, x, y);
			unsigned char r = (p & img->red_mask) >> rshift;
			unsigned char g = (p & img->green_mask) >> gshift;
			unsigned char b = (p & img->blue_mask) >> bshift;
			uint32_t c = (uint32_t)b | ((uint32_t)g << 8) |
				     ((uint32_t)r << 16);
			atomic_store(
				&fb->color_buffer[(size_t)y * fb->width + x],
				c);
		}
	}

	int ret = framebuffer_write_bmp(fb, path);
	framebuffer_destroy(fb);
	XDestroyImage(img);
	pthread_mutex_unlock(&x11_mutex);
	return ret;
}

// Processes X11 events and updates window state.
bool x11_window_process_events(X11Window *w, bool *should_close)
{
	if (!w || !should_close) {
		return false;
	}

	pthread_mutex_lock(&x11_mutex);
	XEvent event;
	while (XPending(w->display)) {
		XNextEvent(w->display, &event);
		switch (event.type) {
		case Expose:
			x11_window_show_image(w, GL_get_default_framebuffer());
			break;
		case KeyPress:
			if (XLookupKeysym(&event.xkey, 0) == XK_Escape) {
				*should_close = true;
			}
			break;
		case ConfigureNotify:
			if (w->width != (unsigned)event.xconfigure.width ||
			    w->height != (unsigned)event.xconfigure.height) {
				w->width = (unsigned)event.xconfigure.width;
				w->height = (unsigned)event.xconfigure.height;
				LOG_DEBUG("Window resized to %ux%u", w->width,
					  w->height);
				// Resize image (simplified; ideally recreate XImage)
				if (w->image) {
					if (w->use_shm) {
						XShmDetach(w->display,
							   &w->shm_info);
						shmdt(w->shm_info.shmaddr);
						shmctl(w->shm_info.shmid,
						       IPC_RMID, NULL);
					}
					XDestroyImage(w->image);
					w->image = NULL;
				}
				char *image_data =
					malloc(w->width * w->height * 4);
				if (image_data) {
					w->image = XCreateImage(
						w->display,
						DefaultVisual(
							w->display,
							DefaultScreen(
								w->display)),
						DefaultDepth(
							w->display,
							DefaultScreen(
								w->display)),
						ZPixmap, 0, image_data,
						w->width, w->height, 32, 0);
					if (!w->image) {
						free(image_data);
					}
				}
			}
			break;
		case ClientMessage:
			if ((Atom)event.xclient.data.l[0] ==
			    XInternAtom(w->display, "WM_DELETE_WINDOW",
					False)) {
				*should_close = true;
			}
			break;
		}
	}
	pthread_mutex_unlock(&x11_mutex);
	return true;
}
