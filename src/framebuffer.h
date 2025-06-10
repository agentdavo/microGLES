#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stdalign.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Framebuffer {
	uint32_t width;
	uint32_t height;
	uint32_t *color_buffer;
	float *depth_buffer;
} Framebuffer;

static_assert(sizeof(uint32_t) == 4, "Framebuffer requires 32-bit colors");

Framebuffer *framebuffer_create(uint32_t width, uint32_t height);
void framebuffer_destroy(Framebuffer *fb);
void framebuffer_clear(Framebuffer *fb, uint32_t clear_color,
		       float clear_depth);
void framebuffer_set_pixel(Framebuffer *fb, uint32_t x, uint32_t y,
			   uint32_t color, float depth);
uint32_t framebuffer_get_pixel(const Framebuffer *fb, uint32_t x, uint32_t y);
float framebuffer_get_depth(const Framebuffer *fb, uint32_t x, uint32_t y);
int framebuffer_write_bmp(const Framebuffer *fb, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* FRAMEBUFFER_H */
