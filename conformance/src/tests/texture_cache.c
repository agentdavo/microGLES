#include "tests.h"
#include "gl_thread.h"
#include "texture_cache.h"
#include "gl_utils.h"
#include <string.h>

int test_texture_cache_hits(void)
{
	texture_cache_t cache;
	texture_cache_init(&cache);
	unsigned pixels = 128 * 128;
	uint32_t *data = tracked_malloc(sizeof(uint32_t) * pixels);
	if (!data)
		return 0;
	for (unsigned y = 0; y < 128; ++y)
		for (unsigned x = 0; x < 128; ++x)
			data[y * 128 + x] = ((x ^ y) & 1) ? 0xFFFFFFFFu :
							    0xFF000000u;
	TextureOES tex = { 0 };
	tex.mip_width[0] = 128;
	tex.mip_height[0] = 128;
	tex.levels[0] = data;
	tex.mipmap_supported = GL_TRUE;
	for (unsigned y = 0; y < 128; ++y)
		for (unsigned x = 0; x < 128; ++x)
			texture_cache_fetch(&cache, &tex, 0, x, y);
	uint64_t hits0 = cache.hits, miss0 = cache.misses;
	for (unsigned y = 0; y < 128; ++y)
		for (unsigned x = 0; x < 128; ++x)
			texture_cache_fetch(&cache, &tex, 0, x, y);
	uint64_t hits1 = cache.hits - hits0;
	uint64_t miss1 = cache.misses - miss0;
	tracked_free(data, sizeof(uint32_t) * pixels);
	float ratio = (float)hits1 / (float)(hits1 + miss1);
	CHECK_OK(ratio > 0.9f);
	return 1;
}

static const struct Test tests[] = { { "texture_cache_hits",
				       test_texture_cache_hits } };

const struct Test *get_texture_cache_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
