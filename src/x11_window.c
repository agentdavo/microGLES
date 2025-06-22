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
	unsigned rshift;
	unsigned gshift;
	unsigned bshift;
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
	w->rshift = __builtin_ctz(w->image->red_mask);
	w->gshift = __builtin_ctz(w->image->green_mask);
	w->bshift = __builtin_ctz(w->image->blue_mask);
	return w;
}

void x11_window_destroy(X11Window *w)
{
	if (!w)
		return;
	if (w->image)
		XDestroyImage(w->image);
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
			unsigned char r = pixel & 0xFF;
			unsigned char g = (pixel >> 8) & 0xFF;
			unsigned char b = (pixel >> 16) & 0xFF;
			unsigned char *dst = (unsigned char *)w->image->data +
					     (y * w->image->bytes_per_line) +
					     x * 4;
			if (w->rshift == 16 && w->bshift == 0) {
				dst[0] = b;
				dst[1] = g;
				dst[2] = r;
			} else if (w->rshift == 0 && w->bshift == 16) {
				dst[0] = r;
				dst[1] = g;
				dst[2] = b;
			} else {
				uint32_t out = ((uint32_t)r << w->rshift) |
					       ((uint32_t)g << w->gshift) |
					       ((uint32_t)b << w->bshift);
				memcpy(dst, &out, 4);
			}
			dst[3] = 0xFF; // alpha
		}
	}
	XPutImage(w->display, w->window, w->gc, w->image, 0, 0, 0, 0, width,
		  height);
	XFlush(w->display);
}

Display *x11_window_get_display(const X11Window *w)
{
	return w ? w->display : NULL;
}
