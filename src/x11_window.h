#ifndef X11_WINDOW_H
#define X11_WINDOW_H

#include <stdint.h>
#include <stdbool.h>
#include <X11/Xlib.h>

struct Framebuffer;

typedef struct X11Window X11Window;

X11Window *x11_window_create(unsigned width, unsigned height,
			     const char *title);
void x11_window_destroy(X11Window *win);
void x11_window_show_image(X11Window *win, const struct Framebuffer *fb);
Display *x11_window_get_display(const X11Window *win);

#endif /* X11_WINDOW_H */
