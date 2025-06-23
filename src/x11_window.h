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
bool x11_window_has_non_monochrome(const X11Window *win);
int x11_window_save_bmp(const X11Window *win, const char *path);
bool x11_window_process_events(X11Window *win, bool *should_close);

#endif /* X11_WINDOW_H */
