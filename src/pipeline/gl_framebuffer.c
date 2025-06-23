#include "gl_framebuffer.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_utils.h"
#include "gl_thread.h"
#include "command_buffer.h"
#include "gl_context.h"
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define PIPELINE_USE_GLSTATE 0
_Static_assert(PIPELINE_USE_GLSTATE == 0, "pipeline must not touch gl_state");

#ifndef GL_INCR_WRAP
#define GL_INCR_WRAP 0x8507
#endif
#ifndef GL_DECR_WRAP
#define GL_DECR_WRAP 0x8508
#endif

static _Thread_local FramebufferTile *tls_tile = NULL;
static _Thread_local StencilState tl_stencil;
static _Thread_local unsigned tl_stencil_ver;
static _Thread_local GLboolean tl_stencil_on;
static _Thread_local GLenum tl_depth_func;
static _Thread_local unsigned tl_depth_ver;
static _Thread_local GLboolean tl_depth_test;
static pthread_mutex_t fb_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint32_t g_env_tile_size = DEFAULT_TILE_SIZE;
static bool g_tile_size_initialized = false;

static FramebufferColorSpec g_env_color_spec = FB_COLOR_ARGB8888;
static bool g_color_spec_initialized = false;

static void init_color_spec(void)
{
	if (g_color_spec_initialized)
		return;
	g_color_spec_initialized = true;
	const char *var = getenv("FB_COLOR_SPEC");
	if (!var || !*var)
		return;
	if (strcmp(var, "XRGB8888") == 0)
		g_env_color_spec = FB_COLOR_XRGB8888;
	else if (strcmp(var, "ARGB8888") == 0)
		g_env_color_spec = FB_COLOR_ARGB8888;
}

static inline uint32_t encode_color(const Framebuffer *fb, uint32_t color)
{
	if (fb->color_spec == FB_COLOR_XRGB8888)
		color = (color & 0x00FFFFFFu) | 0xFF000000u;
	return color;
}

static inline uint32_t decode_color(const Framebuffer *fb, uint32_t stored)
{
	if (fb->color_spec == FB_COLOR_XRGB8888)
		stored |= 0xFF000000u;
	return stored;
}

static void init_tile_size(void)
{
	if (g_tile_size_initialized)
		return;
	g_tile_size_initialized = true;
	const char *var = getenv("TILESIZE");
	if (!var || !*var)
		return;
	if (strcmp(var, "fb") == 0) {
		g_env_tile_size = 0; /* use framebuffer size */
		return;
	}
	char *end;
	long val = strtol(var, &end, 10);
	if (*end == '\0' && val > 0 && val <= 4096)
		g_env_tile_size = (uint32_t)val;
}

// Refreshes thread-local depth and stencil state from the current context.
static inline void refresh_depth_stencil(void)
{
	RenderContext *ctx = GetCurrentContext();
	if (!ctx) {
		LOG_ERROR("refresh_depth_stencil: No current context");
		tl_stencil_on = GL_FALSE;
		tl_depth_test = GL_FALSE;
		return;
	}

	unsigned sv = atomic_load(&ctx->stencil.version);
	if (sv != tl_stencil_ver) {
		memcpy(&tl_stencil, &ctx->stencil, sizeof(StencilState));
		tl_stencil_ver = sv;
	}
	tl_stencil_on = ctx->stencil_test_enabled;

	unsigned dv = atomic_load(&ctx->version_depth);
	if (dv != tl_depth_ver) {
		tl_depth_func = ctx->depth_func;
		tl_depth_test = ctx->depth_test_enabled;
		tl_depth_ver = dv;
	} else {
		tl_depth_test = ctx->depth_test_enabled;
	}
}

// Creates a framebuffer with the specified dimensions.
Framebuffer *framebuffer_create(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0 || width > 16384 || height > 16384) {
		LOG_ERROR("framebuffer_create: Invalid dimensions %ux%u", width,
			  height);
		return NULL;
	}

	init_tile_size();
	init_color_spec();
	pthread_mutex_lock(&fb_mutex);
	Framebuffer *fb = (Framebuffer *)tracked_malloc(sizeof(Framebuffer));
	if (!fb) {
		LOG_ERROR("framebuffer_create: Failed to allocate Framebuffer");
		pthread_mutex_unlock(&fb_mutex);
		return NULL;
	}

	fb->width = width;
	fb->height = height;
	fb->color_spec = g_env_color_spec;
	atomic_init(&fb->ref_count, 1);
	size_t pixels = (size_t)width * height;

	fb->color_buffer = (_Atomic uint32_t *)tracked_aligned_alloc(
		64, pixels * sizeof(_Atomic uint32_t));
	fb->depth_buffer = (_Atomic float *)tracked_aligned_alloc(
		64, pixels * sizeof(_Atomic float));
	fb->stencil_buffer = (_Atomic uint8_t *)tracked_aligned_alloc(
		64, pixels * sizeof(_Atomic uint8_t));

	if (!fb->color_buffer || !fb->depth_buffer || !fb->stencil_buffer) {
		LOG_ERROR("framebuffer_create: Failed to allocate buffers");
		if (fb->color_buffer) {
			tracked_free(fb->color_buffer,
				     pixels * sizeof(_Atomic uint32_t));
		}
		if (fb->depth_buffer) {
			tracked_free(fb->depth_buffer,
				     pixels * sizeof(_Atomic float));
		}
		if (fb->stencil_buffer) {
			tracked_free(fb->stencil_buffer,
				     pixels * sizeof(_Atomic uint8_t));
		}
		tracked_free(fb, sizeof(Framebuffer));
		pthread_mutex_unlock(&fb_mutex);
		return NULL;
	}

	fb->tile_size = (g_env_tile_size == 0) ?
				((width > height) ? width : height) :
				g_env_tile_size;
	fb->tiles_x = (width + fb->tile_size - 1) / fb->tile_size;
	fb->tiles_y = (height + fb->tile_size - 1) / fb->tile_size;
	size_t tile_count = (size_t)fb->tiles_x * fb->tiles_y;
	fb->tiles = (FramebufferTile *)tracked_malloc(tile_count *
						      sizeof(FramebufferTile));
	if (!fb->tiles) {
		LOG_ERROR("framebuffer_create: Failed to allocate tiles");
		tracked_free(fb->color_buffer,
			     pixels * sizeof(_Atomic uint32_t));
		tracked_free(fb->depth_buffer, pixels * sizeof(_Atomic float));
		tracked_free(fb->stencil_buffer,
			     pixels * sizeof(_Atomic uint8_t));
		tracked_free(fb, sizeof(Framebuffer));
		pthread_mutex_unlock(&fb_mutex);
		return NULL;
	}

	for (size_t i = 0; i < tile_count; ++i) {
		fb->tiles[i].x0 = (i % fb->tiles_x) * fb->tile_size;
		fb->tiles[i].y0 = (i / fb->tiles_x) * fb->tile_size;
		fb->tiles[i].color = tracked_aligned_alloc(
			64, fb->tile_size * fb->tile_size *
				    sizeof(_Atomic uint32_t));
		fb->tiles[i].depth = tracked_aligned_alloc(
			64,
			fb->tile_size * fb->tile_size * sizeof(_Atomic float));
		fb->tiles[i].stencil = tracked_aligned_alloc(
			64, fb->tile_size * fb->tile_size *
				    sizeof(_Atomic uint8_t));
		if (!fb->tiles[i].color || !fb->tiles[i].depth ||
		    !fb->tiles[i].stencil) {
			LOG_ERROR(
				"framebuffer_create: Failed to allocate tile buffers");
			for (size_t j = 0; j <= i; ++j) {
				if (fb->tiles[j].color)
					tracked_free(
						fb->tiles[j].color,
						fb->tile_size * fb->tile_size *
							sizeof(_Atomic uint32_t));
				if (fb->tiles[j].depth)
					tracked_free(
						fb->tiles[j].depth,
						fb->tile_size * fb->tile_size *
							sizeof(_Atomic float));
				if (fb->tiles[j].stencil)
					tracked_free(
						fb->tiles[j].stencil,
						fb->tile_size * fb->tile_size *
							sizeof(_Atomic uint8_t));
			}
			tracked_free(fb->tiles,
				     tile_count * sizeof(FramebufferTile));
			tracked_free(fb->color_buffer,
				     pixels * sizeof(_Atomic uint32_t));
			tracked_free(fb->depth_buffer,
				     pixels * sizeof(_Atomic float));
			tracked_free(fb->stencil_buffer,
				     pixels * sizeof(_Atomic uint8_t));
			tracked_free(fb, sizeof(Framebuffer));
			pthread_mutex_unlock(&fb_mutex);
			return NULL;
		}
		atomic_flag_clear(&fb->tiles[i].lock);
		memset(fb->tiles[i].color, 0,
		       fb->tile_size * fb->tile_size * sizeof(uint32_t));
		memset(fb->tiles[i].depth, 0,
		       fb->tile_size * fb->tile_size * sizeof(float));
		memset(fb->tiles[i].stencil, 0,
		       fb->tile_size * fb->tile_size * sizeof(uint8_t));
	}

	framebuffer_clear(fb, 0, 1.0f, 0);
	LOG_INFO("Created framebuffer %ux%u with %zu tiles", width, height,
		 tile_count);
	pthread_mutex_unlock(&fb_mutex);
	return fb;
}

// Increments the framebuffer's reference count.
void framebuffer_retain(Framebuffer *fb)
{
	if (!fb) {
		return;
	}
	atomic_fetch_add_explicit(&fb->ref_count, 1, memory_order_relaxed);
}

// Frees the framebuffer's resources.
static void framebuffer_free(Framebuffer *fb)
{
	if (!fb) {
		return;
	}
	size_t pixels = (size_t)fb->width * fb->height;
	size_t tile_count = (size_t)fb->tiles_x * fb->tiles_y;
	if (fb->color_buffer) {
		tracked_free((void *)fb->color_buffer,
			     pixels * sizeof(_Atomic uint32_t));
	}
	if (fb->depth_buffer) {
		tracked_free((void *)fb->depth_buffer,
			     pixels * sizeof(_Atomic float));
	}
	if (fb->stencil_buffer) {
		tracked_free((void *)fb->stencil_buffer,
			     pixels * sizeof(_Atomic uint8_t));
	}
	if (fb->tiles) {
		for (size_t i = 0; i < tile_count; ++i) {
			if (fb->tiles[i].color)
				tracked_free(fb->tiles[i].color,
					     fb->tile_size * fb->tile_size *
						     sizeof(_Atomic uint32_t));
			if (fb->tiles[i].depth)
				tracked_free(fb->tiles[i].depth,
					     fb->tile_size * fb->tile_size *
						     sizeof(_Atomic float));
			if (fb->tiles[i].stencil)
				tracked_free(fb->tiles[i].stencil,
					     fb->tile_size * fb->tile_size *
						     sizeof(_Atomic uint8_t));
		}
		tracked_free(fb->tiles, tile_count * sizeof(FramebufferTile));
	}
	tracked_free(fb, sizeof(Framebuffer));
}

// Decrements the framebuffer's reference count and frees if zero.
void framebuffer_release(Framebuffer *fb)
{
	if (!fb) {
		return;
	}
	if (atomic_fetch_sub_explicit(&fb->ref_count, 1,
				      memory_order_acq_rel) == 1) {
		pthread_mutex_lock(&fb_mutex);
		framebuffer_free(fb);
		pthread_mutex_unlock(&fb_mutex);
	}
}

// Destroys the framebuffer, ensuring thread pool tasks are completed.
void framebuffer_destroy(Framebuffer *fb)
{
	if (!fb) {
		return;
	}
	if (thread_pool_active()) {
		command_buffer_flush();
		thread_pool_wait();
	}
	framebuffer_release(fb);
}

// Clears the framebuffer with specified color, depth, and stencil values.
void framebuffer_clear(Framebuffer *restrict fb, uint32_t clear_color,
		       float clear_depth, uint8_t clear_stencil)
{
	if (!fb) {
		LOG_ERROR("framebuffer_clear: NULL framebuffer");
		return;
	}

	size_t pixels = (size_t)fb->width * fb->height;
	uint32_t enc = encode_color(fb, clear_color);
	// Use memset for faster clearing where possible
	if (enc == 0) {
		memset(fb->color_buffer, 0, pixels * sizeof(_Atomic uint32_t));
	} else {
		for (size_t i = 0; i < pixels; ++i) {
			atomic_store(&fb->color_buffer[i], enc);
		}
	}
	if (clear_depth == 0.0f) {
		memset(fb->depth_buffer, 0, pixels * sizeof(_Atomic float));
	} else {
		for (size_t i = 0; i < pixels; ++i) {
			atomic_store(&fb->depth_buffer[i], clear_depth);
		}
	}
	if (clear_stencil == 0) {
		memset(fb->stencil_buffer, 0, pixels * sizeof(_Atomic uint8_t));
	} else {
		for (size_t i = 0; i < pixels; ++i) {
			atomic_store(&fb->stencil_buffer[i], clear_stencil);
		}
	}
}

// Asynchronous clear task structure.
typedef struct {
	Framebuffer *fb;
	uint32_t color;
	float depth;
	uint8_t stencil;
} ClearTask;

// Executes an asynchronous clear task.
static void clear_task_func(void *arg)
{
	ClearTask *t = (ClearTask *)arg;
	if (!t || !t->fb) {
		LOG_ERROR("clear_task_func: Invalid task or framebuffer");
		if (t) {
			MT_FREE(t, STAGE_FRAMEBUFFER);
		}
		return;
	}
	framebuffer_clear(t->fb, t->color, t->depth, t->stencil);
	framebuffer_release(t->fb);
	MT_FREE(t, STAGE_FRAMEBUFFER);
	LOG_DEBUG("framebuffer_clear_async task completed");
}

// Clears the framebuffer asynchronously via the thread pool.
void framebuffer_clear_async(Framebuffer *fb, uint32_t clear_color,
			     float clear_depth, uint8_t clear_stencil)
{
	if (!fb) {
		LOG_ERROR("framebuffer_clear_async: NULL framebuffer");
		return;
	}
	if (!thread_pool_active()) {
		framebuffer_clear(fb, clear_color, clear_depth, clear_stencil);
		return;
	}

	ClearTask *task = MT_ALLOC(sizeof(ClearTask), STAGE_FRAMEBUFFER);
	if (!task) {
		LOG_ERROR(
			"framebuffer_clear_async: Failed to allocate ClearTask");
		framebuffer_clear(fb, clear_color, clear_depth, clear_stencil);
		return;
	}

	framebuffer_retain(fb);
	task->fb = fb;
	task->color = clear_color;
	task->depth = clear_depth;
	task->stencil = clear_stencil;
	command_buffer_record_task(clear_task_func, task, STAGE_FRAMEBUFFER);
	LOG_DEBUG("Scheduled async clear for framebuffer %p", fb);
}

// Sets a pixel with color and depth, applying stencil and depth tests.
void framebuffer_set_pixel(Framebuffer *restrict fb, uint32_t x, uint32_t y,
			   uint32_t color, float depth)
{
	if (!fb || x >= fb->width || y >= fb->height) {
		return;
	}
	LOG_DEBUG("set_pixel (%u,%u) color=0x%08X", x, y, color);

	_Atomic uint32_t *color_buffer = fb->color_buffer;
	_Atomic float *depth_buffer = fb->depth_buffer;
	_Atomic uint8_t *stencil_buffer = fb->stencil_buffer;
	size_t stride = fb->width;
	uint32_t tile_x = x, tile_y = y;

	if (tls_tile && x >= tls_tile->x0 && x < tls_tile->x0 + fb->tile_size &&
	    y >= tls_tile->y0 && y < tls_tile->y0 + fb->tile_size) {
		color_buffer = (_Atomic uint32_t *)tls_tile->color;
		depth_buffer = (_Atomic float *)tls_tile->depth;
		stencil_buffer = (_Atomic uint8_t *)tls_tile->stencil;
		stride = fb->tile_size;
		tile_x = x - tls_tile->x0;
		tile_y = y - tls_tile->y0;
	}

	size_t idx = (size_t)tile_y * stride + tile_x;
	refresh_depth_stencil();
	GLboolean stencil_on = tl_stencil_on;
	StencilState *ss = &tl_stencil;
	uint8_t stencil = atomic_load(&stencil_buffer[idx]);

	if (stencil_on) {
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
			atomic_store(&stencil_buffer[idx], new);
			return;
		}
	}

	float current = atomic_load(&depth_buffer[idx]);
	bool depth_pass = false;
	if (!tl_depth_test) {
		depth_pass = true;
	} else {
		while (true) {
			bool pass = false;
			switch (tl_depth_func) {
			case GL_NEVER:
				pass = false;
				break;
			case GL_LESS:
				pass = depth < current;
				break;
			case GL_LEQUAL:
				pass = depth <= current;
				break;
			case GL_GREATER:
				pass = depth > current;
				break;
			case GL_GEQUAL:
				pass = depth >= current;
				break;
			case GL_EQUAL:
				pass = depth == current;
				break;
			case GL_NOTEQUAL:
				pass = depth != current;
				break;
			case GL_ALWAYS:
				pass = true;
				break;
			default:
				pass = depth < current;
				break;
			}
			if (!pass) {
				break;
			}
			if (atomic_compare_exchange_weak(&depth_buffer[idx],
							 &current, depth)) {
				depth_pass = true;
				break;
			}
		}
	}

	if (stencil_on) {
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
		atomic_store(&stencil_buffer[idx], new);
	}

	if (depth_pass) {
		atomic_store(&color_buffer[idx], encode_color(fb, color));
	}
}

// Fills a rectangle with the specified color and depth.
void framebuffer_fill_rect(Framebuffer *fb, uint32_t x0, uint32_t y0,
			   uint32_t x1, uint32_t y1, uint32_t color,
			   float depth)
{
	if (!fb || x0 > x1 || y0 > y1 || x1 >= fb->width || y1 >= fb->height) {
		LOG_ERROR("framebuffer_fill_rect: Invalid parameters");
		return;
	}
	LOG_DEBUG("fill_rect (%u,%u)-(%u,%u) color=0x%08X", x0, y0, x1, y1,
		  color);
	for (uint32_t y = y0; y <= y1; ++y) {
		for (uint32_t x = x0; x <= x1; ++x) {
			framebuffer_set_pixel(fb, x, y, color, depth);
		}
	}
}

// Gets the color value of a pixel.
uint32_t framebuffer_get_pixel(const Framebuffer *fb, uint32_t x, uint32_t y)
{
	if (!fb || x >= fb->width || y >= fb->height) {
		return 0;
	}
	uint32_t v = atomic_load(&fb->color_buffer[(size_t)y * fb->width + x]);
	return decode_color(fb, v);
}

// Gets the depth value of a pixel.
float framebuffer_get_depth(const Framebuffer *fb, uint32_t x, uint32_t y)
{
	if (!fb || x >= fb->width || y >= fb->height) {
		return 1.0f;
	}
	return atomic_load(&fb->depth_buffer[(size_t)y * fb->width + x]);
}

// Writes the framebuffer to a BMP file.
int framebuffer_write_bmp(const Framebuffer *fb, const char *path)
{
	if (!fb || !path) {
		LOG_ERROR("framebuffer_write_bmp: NULL framebuffer or path");
		return 0;
	}

	LOG_DEBUG("framebuffer_write_bmp: writing %s", path);

	FILE *f = fopen(path, "wb");
	if (!f) {
		LOG_ERROR("framebuffer_write_bmp: Failed to open %s", path);
		return 0;
	}

	int width = (int)fb->width;
	int height = (int)fb->height;
	int row_bytes = width * 3;
	int row_padded = (row_bytes + 3) & ~3;
	uint32_t filesize = 54 + row_padded * height;

	unsigned char file_header[14] = { 'B', 'M' };
	file_header[2] = (unsigned char)(filesize);
	file_header[3] = (unsigned char)(filesize >> 8);
	file_header[4] = (unsigned char)(filesize >> 16);
	file_header[5] = (unsigned char)(filesize >> 24);
	file_header[10] = 54;
	if (fwrite(file_header, 1, 14, f) != 14) {
		LOG_ERROR("framebuffer_write_bmp: Failed to write file header");
		fclose(f);
		return 0;
	}

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
	if (fwrite(info_header, 1, 40, f) != 40) {
		LOG_ERROR("framebuffer_write_bmp: Failed to write info header");
		fclose(f);
		return 0;
	}

	unsigned char *row = (unsigned char *)tracked_malloc(row_padded);
	if (!row) {
		LOG_ERROR(
			"framebuffer_write_bmp: Failed to allocate row buffer");
		fclose(f);
		return 0;
	}

	for (int y = height - 1; y >= 0; --y) {
		for (int x = 0; x < width; ++x) {
			uint32_t pixel = decode_color(
				fb,
				atomic_load(
					&fb->color_buffer[(size_t)y * fb->width +
							  x]));
			row[x * 3 + 0] = (pixel >> 0) & 0xFF; // B
			row[x * 3 + 1] = (pixel >> 8) & 0xFF; // G
			row[x * 3 + 2] = (pixel >> 16) & 0xFF; // R
		}
		memset(row + row_bytes, 0, row_padded - row_bytes);
		if (fwrite(row, 1, row_padded, f) != (size_t)row_padded) {
			LOG_ERROR(
				"framebuffer_write_bmp: Failed to write row data");
			tracked_free(row, row_padded);
			fclose(f);
			return 0;
		}
	}

	tracked_free(row, row_padded);
	if (fclose(f) != 0) {
		LOG_ERROR("framebuffer_write_bmp: Failed to close %s", path);
		return 0;
	}
	LOG_INFO("Wrote BMP to %s", path);
	return 1;
}

// Writes the framebuffer to an RGBA text file.
int framebuffer_write_rgba(const Framebuffer *fb, const char *path)
{
	if (!fb || !path) {
		LOG_ERROR("framebuffer_write_rgba: NULL framebuffer or path");
		return 0;
	}

	FILE *f = fopen(path, "w");
	if (!f) {
		LOG_ERROR("framebuffer_write_rgba: Failed to open %s", path);
		return 0;
	}

	int width = (int)fb->width;
	int height = (int)fb->height;
	if (fprintf(f, "%d %d\n", width, height) < 0) {
		LOG_ERROR("framebuffer_write_rgba: Failed to write header");
		fclose(f);
		return 0;
	}

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint32_t pixel = decode_color(
				fb,
				atomic_load(
					&fb->color_buffer[(size_t)y * fb->width +
							  x]));
			unsigned r = (pixel >> 16) & 0xFF;
			unsigned g = (pixel >> 8) & 0xFF;
			unsigned b = pixel & 0xFF;
			if (fprintf(f, "%u %u %u 255\n", r, g, b) < 0) {
				LOG_ERROR(
					"framebuffer_write_rgba: Failed to write pixel data");
				fclose(f);
				return 0;
			}
		}
	}

	if (fclose(f) != 0) {
		LOG_ERROR("framebuffer_write_rgba: Failed to close %s", path);
		return 0;
	}
	LOG_INFO("Wrote RGBA to %s", path);
	return 1;
}

// Streams the framebuffer as RGBA bytes to a file stream.
int framebuffer_stream_rgba(const Framebuffer *fb, FILE *out)
{
	if (!fb || !out) {
		LOG_ERROR(
			"framebuffer_stream_rgba: NULL framebuffer or stream");
		return 0;
	}

	int width = (int)fb->width;
	int height = (int)fb->height;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint32_t pixel = decode_color(
				fb,
				atomic_load(
					&fb->color_buffer[(size_t)y * fb->width +
							  x]));
			unsigned char bytes[4] = {
				(unsigned char)((pixel >> 16) & 0xFF), // R
				(unsigned char)((pixel >> 8) & 0xFF), // G
				(unsigned char)(pixel & 0xFF), // B
				(unsigned char)((pixel >> 24) & 0xFF) // A
			};
			if (fwrite(bytes, 1, 4, out) != 4) {
				LOG_ERROR(
					"framebuffer_stream_rgba: Failed to write pixel data");
				return 0;
			}
		}
	}

	if (fflush(out) != 0) {
		LOG_ERROR("framebuffer_stream_rgba: Failed to flush stream");
		return 0;
	}
	return 1;
}

// Enters a tile for rendering.
void framebuffer_enter_tile(FramebufferTile *tile)
{
	if (!tile) {
		LOG_ERROR("framebuffer_enter_tile: NULL tile");
		return;
	}
	tls_tile = tile;
}

// Leaves the current tile.
void framebuffer_leave_tile(void)
{
	tls_tile = NULL;
}
