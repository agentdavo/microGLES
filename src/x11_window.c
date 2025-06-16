#include "x11_window.h"
#include "pipeline/gl_framebuffer.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>

struct X11Window {
	Display *display;
	Window window;
	GC gc;
	XImage *image;
	unsigned width;
	unsigned height;
};

X11Window *x11_window_create(unsigned width, unsigned height, const char *title)
{
	Display *dpy = XOpenDisplay(NULL);
	if (!dpy)
		return NULL;
	int screen = DefaultScreen(dpy);
	Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen), 0, 0,
					 width, height, 0,
					 BlackPixel(dpy, screen),
					 WhitePixel(dpy, screen));
	XStoreName(dpy, win, title ? title : "microGLES");
	XSelectInput(dpy, win, ExposureMask | KeyPressMask);
	GC gc = XCreateGC(dpy, win, 0, NULL);
	XMapWindow(dpy, win);
	XFlush(dpy);

	X11Window *w = (X11Window *)malloc(sizeof(X11Window));
	if (!w)
		return NULL;
	w->display = dpy;
	w->window = win;
	w->gc = gc;
	w->width = width;
	w->height = height;
	w->image = XCreateImage(dpy, DefaultVisual(dpy, screen), 24, ZPixmap, 0,
				malloc(width * height * 4), width, height, 32,
				0);
	if (!w->image) {
		free(w);
		return NULL;
	}
	return w;
}

void x11_window_destroy(X11Window *w)
{
	if (!w)
		return;
	if (w->image) {
		free(w->image->data);
		w->image->data = NULL;
		XDestroyImage(w->image);
	}
	XFreeGC(w->display, w->gc);
	XDestroyWindow(w->display, w->window);
	XCloseDisplay(w->display);
	free(w);
}

void x11_window_show_image(X11Window *w, const struct Framebuffer *fb)
{
	if (!w || !fb)
		return;
	unsigned width = w->width < fb->width ? w->width : fb->width;
	unsigned height = w->height < fb->height ? w->height : fb->height;
	for (unsigned y = 0; y < height; ++y) {
		for (unsigned x = 0; x < width; ++x) {
			uint32_t pixel = atomic_load(
				&fb->color_buffer[y * fb->width + x]);
			unsigned char *dst = (unsigned char *)w->image->data +
					     (y * w->image->bytes_per_line) +
					     x * 4;
			dst[0] = (pixel >> 16) & 0xFF; // R
			dst[1] = (pixel >> 8) & 0xFF; // G
			dst[2] = pixel & 0xFF; // B
		}
	}
	XPutImage(w->display, w->window, w->gc, w->image, 0, 0, 0, 0, width,
		  height);
	XFlush(w->display);
}
