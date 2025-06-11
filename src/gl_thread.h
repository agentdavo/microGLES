#ifndef GL_THREAD_H
#define GL_THREAD_H

#include <threads.h>
#include <stdint.h>

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

void thread_profile_start(void);
void thread_profile_stop(void);
void thread_profile_report(void);

#ifdef __cplusplus
}
#endif

#endif // GL_THREAD_H
