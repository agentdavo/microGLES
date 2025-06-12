#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include "gl_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MICROGLES_COMMAND_BUFFER

typedef struct {
	uint16_t op;
	union {
		struct {
			task_function_t func;
			void *data;
			stage_tag_t stage;
		} task;
	} params;
} GLCommand;

void command_buffer_init(void);
void command_buffer_shutdown(void);
void command_buffer_record_task(task_function_t func, void *data,
				stage_tag_t stage);
void command_buffer_flush(void);

#else

static inline void command_buffer_init(void)
{
}
static inline void command_buffer_shutdown(void)
{
}
static inline void command_buffer_flush(void)
{
}
static inline void command_buffer_record_task(task_function_t func, void *data,
					      stage_tag_t stage)
{
	thread_pool_submit(func, data, stage);
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* COMMAND_BUFFER_H */
