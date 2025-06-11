#include "tests.h"
#include "gl_state.h"
#include "gl_init.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include <stdio.h>

static void run_tests(const struct Test *tests, size_t count, int *pass,
		      int *total)
{
	for (size_t i = 0; i < count; ++i) {
		LOG_INFO("Running %s", tests[i].name);
		int ok = tests[i].fn();
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			LOG_ERROR("GL error 0x%X after %s", err, tests[i].name);
			ok = 0;
		}
		(*total)++;
		if (ok) {
			(*pass)++;
		} else {
			LOG_ERROR("%s failed", tests[i].name);
		}
	}
}

int main()
{
	if (!logger_init("conformance.log", LOG_LEVEL_INFO)) {
		fprintf(stderr, "Failed to init logger.\n");
		return -1;
	}
	if (!memory_tracker_init()) {
		LOG_FATAL("Failed to init Memory Tracker.");
		return -1;
	}
	thread_pool_init(4);
	InitGLState(&gl_state);
	Framebuffer *fb = GL_init_with_framebuffer(64, 64);
	if (!fb) {
		LOG_FATAL("Failed to create framebuffer");
		return -1;
	}

	int pass = 0;
	int total = 0;
	size_t count;
	const struct Test *list;

#define RUN(group)                                     \
	do {                                           \
		list = group(&count);                  \
		run_tests(list, count, &pass, &total); \
	} while (0)

	RUN(get_state_tests);
	RUN(get_matrix_tests);
	RUN(get_texture_tests);
	RUN(get_buffer_tests);
	RUN(get_draw_tests);
	RUN(get_fbo_tests);
	RUN(get_extensions_tests);
	RUN(get_autogen_tests);
	RUN(get_all_calls_tests);

	thread_pool_shutdown();
	GL_cleanup_with_framebuffer(fb);
	CleanupGLState(&gl_state);
	size_t mem = memory_tracker_current();
	memory_tracker_shutdown();
	logger_shutdown();

	printf("%d/%d tests passed\n", pass, total);
	return (pass == total && mem == 0) ? 0 : 1;
}
