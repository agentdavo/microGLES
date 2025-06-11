#include "gl_framebuffer.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#define PIPELINE_USE_GLSTATE 0
_Static_assert(PIPELINE_USE_GLSTATE == 0, "pipeline must not touch gl_state");
#include "gl_utils.h"
#include "gl_thread.h"
#include "../gl_context.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef GL_INCR_WRAP
#define GL_INCR_WRAP 0x8507
#endif
#ifndef GL_DECR_WRAP
#define GL_DECR_WRAP 0x8508
#endif

Framebuffer *framebuffer_create(uint32_t width, uint32_t height)
{
	Framebuffer *fb = (Framebuffer *)tracked_malloc(sizeof(Framebuffer));
	if (!fb)
		return NULL;
	fb->width = width;
	fb->height = height;
	size_t pixels = (size_t)width * height;
	fb->color_buffer = (_Atomic uint32_t *)tracked_malloc(
		pixels * sizeof(_Atomic uint32_t));
	fb->depth_buffer =
		(_Atomic float *)tracked_malloc(pixels * sizeof(_Atomic float));
	fb->stencil_buffer = (_Atomic uint8_t *)tracked_malloc(
		pixels * sizeof(_Atomic uint8_t));
	if (!fb->color_buffer || !fb->depth_buffer || !fb->stencil_buffer) {
		if (fb->color_buffer)
			tracked_free(fb->color_buffer,
				     pixels * sizeof(uint32_t));
		if (fb->depth_buffer)
			tracked_free(fb->depth_buffer, pixels * sizeof(float));
		if (fb->stencil_buffer)
			tracked_free(fb->stencil_buffer,
				     pixels * sizeof(uint8_t));
		tracked_free(fb, sizeof(Framebuffer));
		return NULL;
	}
	framebuffer_clear(fb, 0, 1.0f, 0);
	return fb;
}

void framebuffer_destroy(Framebuffer *fb)
{
	if (!fb)
		return;
	size_t pixels = (size_t)fb->width * fb->height;
	tracked_free((void *)fb->color_buffer,
		     pixels * sizeof(_Atomic uint32_t));
	tracked_free((void *)fb->depth_buffer, pixels * sizeof(_Atomic float));
	tracked_free((void *)fb->stencil_buffer,
		     pixels * sizeof(_Atomic uint8_t));
	tracked_free(fb, sizeof(Framebuffer));
}

void framebuffer_clear(Framebuffer *fb, uint32_t clear_color, float clear_depth,
		       uint8_t clear_stencil)
{
	size_t total = (size_t)fb->width * fb->height;
	size_t i = 0;
	for (; i + 3 < total; i += 4) {
		atomic_store(&fb->color_buffer[i + 0], clear_color);
		atomic_store(&fb->color_buffer[i + 1], clear_color);
		atomic_store(&fb->color_buffer[i + 2], clear_color);
		atomic_store(&fb->color_buffer[i + 3], clear_color);
		atomic_store(&fb->depth_buffer[i + 0], clear_depth);
		atomic_store(&fb->depth_buffer[i + 1], clear_depth);
		atomic_store(&fb->depth_buffer[i + 2], clear_depth);
		atomic_store(&fb->depth_buffer[i + 3], clear_depth);
		atomic_store(&fb->stencil_buffer[i + 0], clear_stencil);
		atomic_store(&fb->stencil_buffer[i + 1], clear_stencil);
		atomic_store(&fb->stencil_buffer[i + 2], clear_stencil);
		atomic_store(&fb->stencil_buffer[i + 3], clear_stencil);
	}
	for (; i < total; ++i) {
		atomic_store(&fb->color_buffer[i], clear_color);
		atomic_store(&fb->depth_buffer[i], clear_depth);
		atomic_store(&fb->stencil_buffer[i], clear_stencil);
	}
}

typedef struct {
	Framebuffer *fb;
	uint32_t color;
	float depth;
	uint8_t stencil;
} ClearTask;

static void clear_task_func(void *arg)
{
	ClearTask *t = (ClearTask *)arg;
	framebuffer_clear(t->fb, t->color, t->depth, t->stencil);
	MT_FREE(t, STAGE_FRAMEBUFFER);
	LOG_DEBUG("framebuffer_clear_async task completed");
}

void framebuffer_clear_async(Framebuffer *fb, uint32_t clear_color,
			     float clear_depth, uint8_t clear_stencil)
{
	ClearTask *task = MT_ALLOC(sizeof(ClearTask), STAGE_FRAMEBUFFER);
	if (!task) {
		LOG_ERROR("Failed to allocate ClearTask");
		return;
	}
	*task = (ClearTask){ fb, clear_color, clear_depth, clear_stencil };
	thread_pool_submit(clear_task_func, task, STAGE_FRAMEBUFFER);
}

void framebuffer_set_pixel(Framebuffer *fb, uint32_t x, uint32_t y,
			   uint32_t color, float depth)
{
	if (x >= fb->width || y >= fb->height)
		return;
	/* TODO: Implement tiled writes for reduced contention */
	size_t idx = (size_t)y * fb->width + x;
	RenderContext *ctx = GetCurrentContext();
	StencilState *ss = &ctx->stencil;
	uint8_t stencil = atomic_load(&fb->stencil_buffer[idx]);
	if (ctx->stencil_test_enabled) {
		uint8_t masked = stencil & ss->mask;
		uint8_t ref = ss->ref & ss->mask;
		bool pass = false;
		switch (ss->func) {
		case GL_NEVER:
			pass = false;
			break;
		case GL_LESS:
			pass = masked < ref;
			break;
		case GL_LEQUAL:
			pass = masked <= ref;
			break;
		case GL_GREATER:
			pass = masked > ref;
			break;
		case GL_GEQUAL:
			pass = masked >= ref;
			break;
		case GL_EQUAL:
			pass = masked == ref;
			break;
		case GL_NOTEQUAL:
			pass = masked != ref;
			break;
		case GL_ALWAYS:
			pass = true;
			break;
		}
		if (!pass) {
			uint8_t new = stencil;
			switch (ss->sfail) {
			case GL_ZERO:
				new = 0;
				break;
			case GL_REPLACE:
				new = ss->ref;
				break;
			case GL_INCR:
				new = (stencil == 0xFF) ? 0xFF : stencil + 1;
				break;
			case GL_DECR:
				new = (stencil == 0) ? 0 : stencil - 1;
				break;
			case GL_INVERT:
				new = ~stencil;
				break;
			case GL_INCR_WRAP:
				new = stencil + 1;
				break;
			case GL_DECR_WRAP:
				new = stencil - 1;
				break;
			case GL_KEEP:
			default:
				break;
			}
			new = (new & ss->writemask) |
			      (stencil & ~ss->writemask);
			atomic_store(&fb->stencil_buffer[idx], new);
			return;
		}
	}
	float current = atomic_load(&fb->depth_buffer[idx]);
	bool depth_pass = false;
	/*
         * Multiple fragment threads may attempt to write the same depth
         * location concurrently. Compare-and-swap ensures only the closest
         * fragment updates the buffer while losers retry with the new value.
         */
	while (depth < current) {
		if (atomic_compare_exchange_weak(&fb->depth_buffer[idx],
						 &current, depth)) {
			depth_pass = true;
			break;
		}
	}
	if (ctx->stencil_test_enabled) {
		uint8_t new = stencil;
		GLenum op = depth_pass ? ss->zpass : ss->zfail;
		switch (op) {
		case GL_ZERO:
			new = 0;
			break;
		case GL_REPLACE:
			new = ss->ref;
			break;
		case GL_INCR:
			new = (stencil == 0xFF) ? 0xFF : stencil + 1;
			break;
		case GL_DECR:
			new = (stencil == 0) ? 0 : stencil - 1;
			break;
		case GL_INVERT:
			new = ~stencil;
			break;
		case GL_INCR_WRAP:
			new = stencil + 1;
			break;
		case GL_DECR_WRAP:
			new = stencil - 1;
			break;
		case GL_KEEP:
		default:
			break;
		}
		new = (new & ss->writemask) | (stencil & ~ss->writemask);
		atomic_store(&fb->stencil_buffer[idx], new);
	}
	if (depth_pass)
		atomic_store(&fb->color_buffer[idx], color);
}

void framebuffer_fill_rect(Framebuffer *fb, uint32_t x0, uint32_t y0,
			   uint32_t x1, uint32_t y1, uint32_t color,
			   float depth)
{
	for (uint32_t y = y0; y <= y1; ++y)
		for (uint32_t x = x0; x <= x1; ++x)
			framebuffer_set_pixel(fb, x, y, color, depth);
}

uint32_t framebuffer_get_pixel(const Framebuffer *fb, uint32_t x, uint32_t y)
{
	if (x >= fb->width || y >= fb->height)
		return 0;
	return atomic_load(&fb->color_buffer[(size_t)y * fb->width + x]);
}

float framebuffer_get_depth(const Framebuffer *fb, uint32_t x, uint32_t y)
{
	if (x >= fb->width || y >= fb->height)
		return 1.0f;
	return atomic_load(&fb->depth_buffer[(size_t)y * fb->width + x]);
}

int framebuffer_write_bmp(const Framebuffer *fb, const char *path)
{
	FILE *f = fopen(path, "wb");
	if (!f) {
		LOG_ERROR("Failed to open %s", path);
		return 0;
	}
	int width = (int)fb->width;
	int height = (int)fb->height;
	int row_bytes = width * 3;
	int row_padded = (row_bytes + 3) & ~3;
	unsigned int filesize = 54 + row_padded * height;
	unsigned char file_header[14] = { 'B', 'M' };
	file_header[2] = (unsigned char)(filesize);
	file_header[3] = (unsigned char)(filesize >> 8);
	file_header[4] = (unsigned char)(filesize >> 16);
	file_header[5] = (unsigned char)(filesize >> 24);
	file_header[10] = 54;
	fwrite(file_header, 1, 14, f);

	unsigned char info_header[40] = { 0 };
	info_header[0] = 40;
	info_header[4] = (unsigned char)(width);
	info_header[5] = (unsigned char)(width >> 8);
	info_header[6] = (unsigned char)(width >> 16);
	info_header[7] = (unsigned char)(width >> 24);
	info_header[8] = (unsigned char)(height);
	info_header[9] = (unsigned char)(height >> 8);
	info_header[10] = (unsigned char)(height >> 16);
	info_header[11] = (unsigned char)(height >> 24);
	info_header[12] = 1;
	info_header[14] = 24;
	fwrite(info_header, 1, 40, f);

	unsigned char *row = (unsigned char *)tracked_malloc(row_padded);
	if (!row) {
		fclose(f);
		return 0;
	}
	for (int y = height - 1; y >= 0; --y) {
		for (int x = 0; x < width; ++x) {
			uint32_t pixel = atomic_load(
				&fb->color_buffer[(size_t)y * fb->width + x]);
			row[x * 3 + 0] = (pixel >> 0) & 0xFF;
			row[x * 3 + 1] = (pixel >> 8) & 0xFF;
			row[x * 3 + 2] = (pixel >> 16) & 0xFF;
		}
		memset(row + row_bytes, 0, row_padded - row_bytes);
		fwrite(row, 1, row_padded, f);
	}
	tracked_free(row, row_padded);
	fclose(f);
	LOG_INFO("Wrote %s", path);
	return 1;
}

int framebuffer_write_rgba(const Framebuffer *fb, const char *path)
{
	FILE *f = fopen(path, "w");
	if (!f) {
		LOG_ERROR("Failed to open %s", path);
		return 0;
	}
	int width = (int)fb->width;
	int height = (int)fb->height;
	fprintf(f, "%d %d\n", width, height);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint32_t pixel = atomic_load(
				&fb->color_buffer[(size_t)y * fb->width + x]);
			unsigned r = (pixel >> 16) & 0xFF;
			unsigned g = (pixel >> 8) & 0xFF;
			unsigned b = pixel & 0xFF;
			fprintf(f, "%u %u %u 255\n", r, g, b);
		}
	}
	fclose(f);
	LOG_INFO("Wrote %s", path);
	return 1;
}
