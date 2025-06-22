#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stdio.h>
#include <stdalign.h>
#include <assert.h>
#include <stdatomic.h>
#include "gl_thread.h"

#define TILE_SIZE 16

typedef struct {
	uint32_t x0, y0;
	uint32_t color[TILE_SIZE * TILE_SIZE];
	float depth[TILE_SIZE * TILE_SIZE];
	uint8_t stencil[TILE_SIZE * TILE_SIZE];
	atomic_flag lock;
} FramebufferTile;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Framebuffer {
	uint32_t width;
	uint32_t height;
	_Atomic int ref_count;
	_Atomic uint32_t *color_buffer;
	_Atomic float *depth_buffer;
	_Atomic uint8_t *stencil_buffer;
	FramebufferTile *tiles;
	uint32_t tiles_x;
	uint32_t tiles_y;
} Framebuffer;

void framebuffer_enter_tile(FramebufferTile *tile);
void framebuffer_leave_tile(void);

static_assert(sizeof(uint32_t) == 4, "Framebuffer requires 32-bit colors");

Framebuffer *framebuffer_create(uint32_t width, uint32_t height);
void framebuffer_destroy(Framebuffer *fb);
void framebuffer_retain(Framebuffer *fb);
void framebuffer_release(Framebuffer *fb);
void framebuffer_clear(Framebuffer *restrict fb, uint32_t clear_color,
		       float clear_depth, uint8_t clear_stencil);
void framebuffer_set_pixel(Framebuffer *restrict fb, uint32_t x, uint32_t y,
			   uint32_t color, float depth);
void framebuffer_fill_rect(Framebuffer *fb, uint32_t x0, uint32_t y0,
			   uint32_t x1, uint32_t y1, uint32_t color,
			   float depth);
uint32_t framebuffer_get_pixel(const Framebuffer *fb, uint32_t x, uint32_t y);
float framebuffer_get_depth(const Framebuffer *fb, uint32_t x, uint32_t y);
int framebuffer_write_bmp(const Framebuffer *fb, const char *path);
int framebuffer_write_rgba(const Framebuffer *fb, const char *path);
int framebuffer_stream_rgba(const Framebuffer *fb, FILE *out);
void framebuffer_clear_async(Framebuffer *fb, uint32_t clear_color,
			     float clear_depth, uint8_t clear_stencil);

#ifdef __cplusplus
}
#endif

#endif /* FRAMEBUFFER_H */
