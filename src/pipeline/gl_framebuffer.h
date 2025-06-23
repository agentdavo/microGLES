#ifndef PIPELINE_GL_FRAMEBUFFER_H
#define PIPELINE_GL_FRAMEBUFFER_H

#include <stdint.h>
#include <stdio.h>
#include <stdalign.h>
#include <assert.h>
#include <stdatomic.h>
#include "gl_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default tile size used if TILESIZE env var is not set.
 */
#define DEFAULT_TILE_SIZE 16

/** Framebuffer colour specification selected via `FB_COLOR_SPEC` or
 *  `--color-spec`. */
typedef enum {
	FB_COLOR_ARGB8888, /**< Stored as AARRGGBB (default). */
	FB_COLOR_XRGB8888 /**< Alpha ignored, treated as 0xFF. */
} FramebufferColorSpec;

/**
 * @brief Structure representing a tile in a framebuffer. Tile size is
 *        determined at runtime via the TILESIZE environment variable or the
 *        `--tilesize` command line option (see README).
 *
 * Contains color, depth, and stencil data for a tile, aligned to 64 bytes for
 * cache efficiency. Includes an atomic lock for thread-safe access.
 */
typedef struct {
	alignas(64) uint32_t x0, y0; /**< Top-left coordinates of the tile. */
	_Atomic uint32_t *color; /**< Color data (RGBA). */
	_Atomic float *depth; /**< Depth data. */
	_Atomic uint8_t *stencil; /**< Stencil data. */
	atomic_flag lock; /**< Lock for thread-safe tile access. */
} FramebufferTile;

_Static_assert(alignof(FramebufferTile) >= 64,
	       "FramebufferTile must be 64-byte aligned");

/**
 * @brief Structure representing a framebuffer.
 *
 * Contains color, depth, and stencil buffers, a tile array for parallel rendering,
 * and a reference count for memory management. Buffers use atomic types for
 * thread-safe access.
 */
typedef struct Framebuffer {
	uint32_t width; /**< Width in pixels. */
	uint32_t height; /**< Height in pixels. */
	_Atomic int ref_count; /**< Reference count for memory management. */
	_Atomic uint32_t *color_buffer; /**< Front color buffer (RGBA). */
	_Atomic float *depth_buffer; /**< Depth buffer. */
	_Atomic uint8_t *stencil_buffer; /**< Stencil buffer. */
	FramebufferTile *tiles; /**< Array of tiles for parallel rendering. */
	uint32_t tiles_x; /**< Number of tiles along x-axis. */
	uint32_t tiles_y; /**< Number of tiles along y-axis. */
	uint32_t tile_size; /**< Size of each tile (pixels). */
	FramebufferColorSpec color_spec; /**< Colour format. */
} Framebuffer;

_Static_assert(sizeof(uint32_t) == 4, "Framebuffer requires 32-bit colors");

/**
 * @brief Enters a tile for rendering, setting it as the current thread’s tile.
 * @param tile The tile to enter (must not be NULL).
 * @threadsafe
 */
void framebuffer_enter_tile(FramebufferTile *tile);

/**
 * @brief Leaves the current tile, resetting the thread’s tile state.
 * @threadsafe
 */
void framebuffer_leave_tile(void);

/**
 * @brief Creates a framebuffer with the specified dimensions.
 * @param width Width in pixels (must be > 0 and <= 16384).
 * @param height Height in pixels (must be > 0 and <= 16384).
 * @return Pointer to the created framebuffer, or NULL on failure.
 * @threadsafe
 */
Framebuffer *framebuffer_create(uint32_t width, uint32_t height);

/**
 * @brief Destroys a framebuffer, freeing its resources.
 * @param fb Framebuffer to destroy (may be NULL).
 * @threadsafe
 */
void framebuffer_destroy(Framebuffer *fb);

/**
 * @brief Increments the framebuffer’s reference count.
 * @param fb Framebuffer to retain (may be NULL).
 * @threadsafe
 */
void framebuffer_retain(Framebuffer *fb);

/**
 * @brief Decrements the framebuffer’s reference count, freeing if zero.
 * @param fb Framebuffer to release (may be NULL).
 * @threadsafe
 */
void framebuffer_release(Framebuffer *fb);

/**
 * @brief Clears the framebuffer with specified color, depth, and stencil values.
 * @param fb Framebuffer to clear (must not be NULL).
 * @param clear_color RGBA color value (e.g., 0xFF0000FF for red).
 * @param clear_depth Depth value (typically 0.0 to 1.0).
 * @param clear_stencil Stencil value (typically 0).
 * @threadsafe
 */
void framebuffer_clear(Framebuffer *restrict fb, uint32_t clear_color,
		       float clear_depth, uint8_t clear_stencil);

/**
 * @brief Sets a pixel’s color and depth, applying stencil and depth tests.
 * @param fb Framebuffer to modify (must not be NULL).
 * @param x X-coordinate (must be < fb->width).
 * @param y Y-coordinate (must be < fb->height).
 * @param color RGBA color value.
 * @param depth Depth value.
 * @threadsafe
 */
void framebuffer_set_pixel(Framebuffer *restrict fb, uint32_t x, uint32_t y,
			   uint32_t color, float depth);

/**
 * @brief Fills a rectangle with the specified color and depth.
 * @param fb Framebuffer to modify (must not be NULL).
 * @param x0 Starting x-coordinate.
 * @param y0 Starting y-coordinate.
 * @param x1 Ending x-coordinate (x0 <= x1 < fb->width).
 * @param y1 Ending y-coordinate (y0 <= y1 < fb->height).
 * @param color RGBA color value.
 * @param depth Depth value.
 * @threadsafe
 */
void framebuffer_fill_rect(Framebuffer *fb, uint32_t x0, uint32_t y0,
			   uint32_t x1, uint32_t y1, uint32_t color,
			   float depth);

/**
 * @brief Gets the color value of a pixel.
 * @param fb Framebuffer to read (must not be NULL).
 * @param x X-coordinate (must be < fb->width).
 * @param y Y-coordinate (must be < fb->height).
 * @return RGBA color value.
 * @threadsafe
 */
uint32_t framebuffer_get_pixel(const Framebuffer *fb, uint32_t x, uint32_t y);

/**
 * @brief Gets the depth value of a pixel.
 * @param fb Framebuffer to read (must not be NULL).
 * @param x X-coordinate (must be < fb->width).
 * @param y Y-coordinate (must be < fb->height).
 * @return Depth value.
 * @threadsafe
 */
float framebuffer_get_depth(const Framebuffer *fb, uint32_t x, uint32_t y);

/**
 * @brief Writes the framebuffer to a BMP file.
 * @param fb Framebuffer to write (must not be NULL).
 * @param path File path (must not be NULL).
 * @return 1 on success, 0 on failure.
 * @threadsafe
 */
int framebuffer_write_bmp(const Framebuffer *fb, const char *path);

/**
 * @brief Writes the framebuffer to an RGBA text file.
 * @param fb Framebuffer to write (must not be NULL).
 * @param path File path (must not be NULL).
 * @return 1 on success, 0 on failure.
 * @threadsafe
 */
int framebuffer_write_rgba(const Framebuffer *fb, const char *path);

/**
 * @brief Streams the framebuffer as RGBA bytes to a file stream.
 * @param fb Framebuffer to stream (must not be NULL).
 * @param out File stream (must not be NULL).
 * @return 1 on success, 0 on failure.
 * @threadsafe
 */
int framebuffer_stream_rgba(const Framebuffer *fb, FILE *out);

/**
 * @brief Clears the framebuffer asynchronously via the thread pool.
 * @param fb Framebuffer to clear (must not be NULL).
 * @param clear_color RGBA color value.
 * @param clear_depth Depth value.
 * @param clear_stencil Stencil value.
 * @threadsafe
 */
void framebuffer_clear_async(Framebuffer *fb, uint32_t clear_color,
			     float clear_depth, uint8_t clear_stencil);

#ifdef __cplusplus
}
#endif

#endif /* PIPELINE_GL_FRAMEBUFFER_H */
