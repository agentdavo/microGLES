#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stdalign.h>
#include <assert.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Framebuffer {
	uint32_t width;
	uint32_t height;
	_Atomic uint32_t *color_buffer;
	_Atomic float *depth_buffer;
	_Atomic uint8_t *stencil_buffer;
} Framebuffer;

static_assert(sizeof(uint32_t) == 4, "Framebuffer requires 32-bit colors");

Framebuffer *framebuffer_create(uint32_t width, uint32_t height);
void framebuffer_destroy(Framebuffer *fb);
void framebuffer_clear(Framebuffer *fb, uint32_t clear_color, float clear_depth,
		       uint8_t clear_stencil);
void framebuffer_set_pixel(Framebuffer *fb, uint32_t x, uint32_t y,
			   uint32_t color, float depth);
void framebuffer_fill_rect(Framebuffer *fb, uint32_t x0, uint32_t y0,
			   uint32_t x1, uint32_t y1, uint32_t color,
			   float depth);
uint32_t framebuffer_get_pixel(const Framebuffer *fb, uint32_t x, uint32_t y);
float framebuffer_get_depth(const Framebuffer *fb, uint32_t x, uint32_t y);
int framebuffer_write_bmp(const Framebuffer *fb, const char *path);
void framebuffer_clear_async(Framebuffer *fb, uint32_t clear_color,
			     float clear_depth, uint8_t clear_stencil);

#ifdef __cplusplus
}
#endif

#endif /* FRAMEBUFFER_H */
