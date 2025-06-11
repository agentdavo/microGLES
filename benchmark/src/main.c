#include "benchmark.h"
#include "gl_state.h"
#include "gl_init.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include <stdio.h>
#include <assert.h>

int main()
{
	if (!logger_init("benchmark.log", LOG_LEVEL_INFO)) {
		fprintf(stderr, "Failed to initialize logger.\n");
		return -1;
	}
	if (!memory_tracker_init()) {
		LOG_FATAL("Failed to initialize Memory Tracker.");
		return -1;
	}
	thread_pool_init(4);
	thread_profile_start();
	InitGLState(&gl_state);
	Framebuffer *fb = GL_init_with_framebuffer(256, 256);
	if (!fb) {
		LOG_FATAL("Failed to init framebuffer");
		return -1;
	}

	BenchmarkResult result;
	run_triangle_strip(100, fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_triangle_strip(1000, fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_triangle_strip(10000, fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_textured_quad(fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_lit_cube(1, fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_lit_cube(0, fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_fbo_benchmark(fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_pipeline_test(fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_spinning_gears(fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_spinning_cubes(fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_multitexture_demo(fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_alpha_blend_demo(fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_miletostone1(fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif
	run_milestone2(fb, &result);
#ifdef DEBUG
	assert(glGetError() == GL_NO_ERROR);
#endif

	thread_pool_wait();
	thread_pool_shutdown();
	GL_cleanup_with_framebuffer(fb);
	CleanupGLState(&gl_state);
	memory_tracker_shutdown();
	memory_tracker_report();
	logger_shutdown();
	return 0;
}
