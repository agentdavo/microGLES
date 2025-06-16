#include "benchmark.h"
#include "gl_state.h"
#include "gl_init.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include "command_buffer.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	bool profile = false;
	bool stream_fb = false;
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--profile") == 0)
			profile = true;
		else if (strcmp(argv[i], "--stream-fb") == 0)
			stream_fb = true;
	}
	if (!logger_init("stress_test.log", LOG_LEVEL_INFO)) {
		fprintf(stderr, "Failed to initialize logger.\n");
		return -1;
	}
	if (!memory_tracker_init()) {
		LOG_FATAL("Failed to initialize Memory Tracker.");
		return -1;
	}
	if (!thread_pool_init_from_env()) {
		LOG_FATAL("Failed to init thread pool");
		return -1;
	}
	command_buffer_init();
	if (profile)
		thread_profile_start();
	InitGLState(&gl_state);
	Framebuffer *fb = GL_init_with_framebuffer(256, 256);
	if (!fb) {
		LOG_FATAL("Failed to init framebuffer");
		return -1;
	}

	BenchmarkResult result;
	run_stress_test(fb, &result, stream_fb, 60);
	printf("Stress Test: %.2f FPS\n", result.fps);

	thread_pool_wait();
	command_buffer_shutdown();
	thread_pool_shutdown();
	GL_cleanup_with_framebuffer(fb);
	CleanupGLState(&gl_state);
	memory_tracker_shutdown();
	logger_shutdown();
	return 0;
}
