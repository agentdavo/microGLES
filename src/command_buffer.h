#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H
/**
 * @file command_buffer.h
 * @brief Ring buffer for queuing GL commands before execution.
 */

#include "gl_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* COMMAND_BUFFER_H */
