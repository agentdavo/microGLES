#ifndef GL_THREAD_H
#define GL_THREAD_H
/**
 * @file gl_thread.h
 * @brief Thread pool helpers used by the renderer.
 */

#include <threads.h>
#include <stdint.h>
#include <stdbool.h>
#include "texture_cache.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	STAGE_VERTEX,
	STAGE_PRIMITIVE,
	STAGE_RASTER,
	STAGE_FRAGMENT,
	STAGE_FRAMEBUFFER,
	STAGE_COUNT
} stage_tag_t;

typedef void (*task_function_t)(void *task_data);

void thread_pool_init(int num_threads);
void thread_pool_submit(task_function_t func, void *task_data,
			stage_tag_t stage);
void thread_pool_wait(void);
int thread_pool_wait_timeout(uint32_t ms);
void thread_pool_shutdown(void);

/* Returns true if worker threads are processing tasks. */
bool thread_pool_active(void);

void thread_profile_start(void);
void thread_profile_stop(void);
void thread_profile_report(void);

/* texture cache helpers */
texture_cache_t *thread_get_texture_cache(void);
void thread_profile_cache_hit(void);
void thread_profile_cache_miss(void);
void thread_profile_get_cache_stats(uint64_t *hits, uint64_t *misses);

#ifdef __cplusplus
}
#endif

#endif // GL_THREAD_H
