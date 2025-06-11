#include "gl_framebuffer.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_utils.h"
#include "gl_thread.h"
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

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
	if (!fb->color_buffer || !fb->depth_buffer) {
		if (fb->color_buffer)
			tracked_free(fb->color_buffer,
				     pixels * sizeof(uint32_t));
		if (fb->depth_buffer)
			tracked_free(fb->depth_buffer, pixels * sizeof(float));
		tracked_free(fb, sizeof(Framebuffer));
		return NULL;
	}
	framebuffer_clear(fb, 0, 1.0f);
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
	tracked_free(fb, sizeof(Framebuffer));
}

void framebuffer_clear(Framebuffer *fb, uint32_t clear_color, float clear_depth)
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
	}
	for (; i < total; ++i) {
		atomic_store(&fb->color_buffer[i], clear_color);
		atomic_store(&fb->depth_buffer[i], clear_depth);
	}
}

typedef struct {
	Framebuffer *fb;
	uint32_t color;
	float depth;
} ClearTask;

static void clear_task_func(void *arg)
{
	ClearTask *t = (ClearTask *)arg;
	framebuffer_clear(t->fb, t->color, t->depth);
	MT_FREE(t, STAGE_FRAMEBUFFER);
	LOG_DEBUG("framebuffer_clear_async task completed");
}

void framebuffer_clear_async(Framebuffer *fb, uint32_t clear_color,
			     float clear_depth)
{
	ClearTask *task = MT_ALLOC(sizeof(ClearTask), STAGE_FRAMEBUFFER);
	if (!task) {
		LOG_ERROR("Failed to allocate ClearTask");
		return;
	}
	*task = (ClearTask){ fb, clear_color, clear_depth };
	thread_pool_submit(clear_task_func, task, STAGE_FRAMEBUFFER);
}

void framebuffer_set_pixel(Framebuffer *fb, uint32_t x, uint32_t y,
			   uint32_t color, float depth)
{
	if (x >= fb->width || y >= fb->height)
		return;
	/* TODO: Implement tiled writes for reduced contention */
	size_t idx = (size_t)y * fb->width + x;
	float current = atomic_load(&fb->depth_buffer[idx]);
	while (depth < current) {
		if (atomic_compare_exchange_weak(&fb->depth_buffer[idx],
						 &current, depth)) {
			atomic_store(&fb->color_buffer[idx], color);
			break;
		}
	}
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
