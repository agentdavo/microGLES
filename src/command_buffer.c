#include "command_buffer.h"
#include <stdatomic.h>

#define COMMAND_RING_SIZE 1024

static GLCommand g_ring[COMMAND_RING_SIZE];
static atomic_uint g_head;
static atomic_uint g_tail;

void command_buffer_init(void)
{
	atomic_init(&g_head, 0);
	atomic_init(&g_tail, 0);
}

void command_buffer_shutdown(void)
{
	command_buffer_flush();
}

void command_buffer_record_task(task_function_t func, void *data,
				stage_tag_t stage)
{
	uint32_t t = atomic_load_explicit(&g_tail, memory_order_relaxed);
	uint32_t h = atomic_load_explicit(&g_head, memory_order_acquire);
	if (t - h >= COMMAND_RING_SIZE)
		command_buffer_flush();
	t = atomic_load_explicit(&g_tail, memory_order_relaxed);
	h = atomic_load_explicit(&g_head, memory_order_acquire);
	if (t - h >= COMMAND_RING_SIZE)
		return; // drop if still full
	uint32_t slot = t % COMMAND_RING_SIZE;
	g_ring[slot].op = 1; /* submit task */
	g_ring[slot].params.task.func = func;
	g_ring[slot].params.task.data = data;
	g_ring[slot].params.task.stage = stage;
	atomic_store_explicit(&g_tail, t + 1, memory_order_release);
}

void command_buffer_flush(void)
{
	uint32_t h = atomic_load_explicit(&g_head, memory_order_relaxed);
	uint32_t t = atomic_load_explicit(&g_tail, memory_order_acquire);
	while (h < t) {
		uint32_t slot = h % COMMAND_RING_SIZE;
		GLCommand *cmd = &g_ring[slot];
		if (cmd->op == 1) {
			thread_pool_submit(cmd->params.task.func,
					   cmd->params.task.data,
					   cmd->params.task.stage);
			cmd->op = 0;
		}
		h++;
	}
	atomic_store_explicit(&g_head, t, memory_order_release);
	atomic_store_explicit(&g_tail, t, memory_order_release);
}
