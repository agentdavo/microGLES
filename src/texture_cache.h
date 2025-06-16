#ifndef TEXTURE_CACHE_H
#define TEXTURE_CACHE_H
/**
 * @file texture_cache.h
 * @brief 4x4 texture block cache for fragment sampling.
 */
#include <stdint.h>
#include <stdbool.h>
#include "gl_types.h"

#define TEXTURE_CACHE_BLOCK 4
#define TEXTURE_CACHE_SETS 256
#define TEXTURE_CACHE_WAYS 4

typedef struct {
	const TextureOES *tex;
	unsigned level;
	unsigned bx, by;
	uint64_t lru;
	bool valid;
	uint32_t data[TEXTURE_CACHE_BLOCK * TEXTURE_CACHE_BLOCK];
} texture_cache_entry_t;

typedef struct {
	texture_cache_entry_t entry[TEXTURE_CACHE_WAYS];
} texture_cache_set_t;

typedef struct {
	texture_cache_set_t sets[TEXTURE_CACHE_SETS];
	uint64_t counter;
	uint64_t hits;
	uint64_t misses;
} texture_cache_t;

void texture_cache_init(texture_cache_t *cache);
uint32_t texture_cache_fetch(texture_cache_t *cache, const TextureOES *tex,
			     unsigned level, unsigned x, unsigned y);
#endif // TEXTURE_CACHE_H
