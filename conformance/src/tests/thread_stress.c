#include "tests.h"
#include "command_buffer.h"
#include "gl_thread.h"
#include <stdatomic.h>

static void inc_task(void *data)
{
	atomic_fetch_add_explicit((atomic_uint *)data, 1, memory_order_relaxed);
}

int test_command_buffer_ring(void)
{
	atomic_uint counter;
	atomic_init(&counter, 0);
	const int total = 256;
	for (int i = 0; i < total; ++i) {
		command_buffer_record_task(inc_task, &counter, STAGE_VERTEX);
		if ((i % 16) == 15)
			command_buffer_flush();
	}
	command_buffer_flush();
	thread_pool_wait_timeout(2000);
	return atomic_load_explicit(&counter, memory_order_relaxed) == total;
}

static const struct Test tests[] = {
	{ "command_buffer_ring", test_command_buffer_ring },
};

const struct Test *get_thread_stress_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
