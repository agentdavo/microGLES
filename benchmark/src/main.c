#include "benchmark.h"
#include "gl_state.h"
#include "gl_init.h"
#include "logger.h"
#include "memory_tracker.h"
#include <stdio.h>

int main()
{
	if (!InitLogger("benchmark.log", LOG_LEVEL_INFO)) {
		fprintf(stderr, "Failed to initialize logger.\n");
		return -1;
	}
	if (!InitMemoryTracker()) {
		LOG_FATAL("Failed to initialize Memory Tracker.");
		return -1;
	}
	InitGLState(&gl_state);
	Framebuffer *fb = GL_init_with_framebuffer(256, 256);
	if (!fb) {
		LOG_FATAL("Failed to init framebuffer");
		return -1;
	}

	BenchmarkResult result;
	run_triangle_strip(100, fb, &result);
	run_triangle_strip(1000, fb, &result);
	run_triangle_strip(10000, fb, &result);
	run_textured_quad(fb, &result);
	run_lit_cube(1, fb, &result);
	run_lit_cube(0, fb, &result);
	run_fbo_benchmark(fb, &result);
	run_spinning_gears(fb, &result);
	run_spinning_cubes(fb, &result);
	run_multitexture_demo(fb, &result);
	run_alpha_blend_demo(fb, &result);

	GL_cleanup_with_framebuffer(fb);
	CleanupGLState(&gl_state);
	ShutdownMemoryTracker();
	PrintMemoryUsage();
	ShutdownLogger();
	return 0;
}
