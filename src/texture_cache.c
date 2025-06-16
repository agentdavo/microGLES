#include "texture_cache.h"
#include "gl_thread.h"
#include <string.h>

void texture_cache_init(texture_cache_t *cache)
{
	memset(cache, 0, sizeof(*cache));
}

static size_t hash_ptr(const TextureOES *tex, unsigned level, unsigned bx,
		       unsigned by)
{
	uintptr_t p = (uintptr_t)tex;
	return ((p >> 4) ^ level ^ bx ^ (by << 1)) & (TEXTURE_CACHE_SETS - 1);
}

uint32_t texture_cache_fetch(texture_cache_t *cache, const TextureOES *tex,
			     unsigned level, unsigned x, unsigned y)
{
	unsigned bx = x / TEXTURE_CACHE_BLOCK;
	unsigned by = y / TEXTURE_CACHE_BLOCK;
	unsigned off = (y % TEXTURE_CACHE_BLOCK) * TEXTURE_CACHE_BLOCK +
		       (x % TEXTURE_CACHE_BLOCK);
	size_t set_idx = hash_ptr(tex, level, bx, by);
	texture_cache_set_t *set = &cache->sets[set_idx];
	cache->counter++;
	uint64_t now = cache->counter;
	for (int i = 0; i < TEXTURE_CACHE_WAYS; ++i) {
		texture_cache_entry_t *e = &set->entry[i];
		if (e->valid && e->tex == tex && e->level == level &&
		    e->bx == bx && e->by == by) {
			e->lru = now;
			cache->hits++;
			thread_profile_cache_hit();
			return e->data[off];
		}
	}
	cache->misses++;
	thread_profile_cache_miss();
	int victim = 0;
	uint64_t oldest = set->entry[0].lru;
	for (int i = 0; i < TEXTURE_CACHE_WAYS; ++i) {
		if (!set->entry[i].valid) {
			victim = i;
			break;
		}
		if (set->entry[i].lru < oldest) {
			oldest = set->entry[i].lru;
			victim = i;
		}
	}
	texture_cache_entry_t *e = &set->entry[victim];
	e->valid = true;
	e->tex = tex;
	e->level = level;
	e->bx = bx;
	e->by = by;
	e->lru = now;
	for (unsigned row = 0; row < TEXTURE_CACHE_BLOCK; ++row) {
		unsigned ty = by * TEXTURE_CACHE_BLOCK + row;
		for (unsigned col = 0; col < TEXTURE_CACHE_BLOCK; ++col) {
			unsigned tx = bx * TEXTURE_CACHE_BLOCK + col;
			uint32_t val = 0;
			if (tx < (unsigned)tex->mip_width[level] &&
			    ty < (unsigned)tex->mip_height[level] &&
			    tex->levels[level])
				val = tex->levels[level]
						 [ty * tex->mip_width[level] +
						  tx];
			e->data[row * TEXTURE_CACHE_BLOCK + col] = val;
		}
	}
	return e->data[off];
}
