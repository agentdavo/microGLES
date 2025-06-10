#include "benchmark.h"
#include "gl_utils.h"
#include "logger.h"

void run_alpha_blend_demo(Framebuffer *fb, BenchmarkResult *result)
{
	framebuffer_clear(fb, 0x00000000u, 1.0f);
	clock_t start = clock();
	for (int frame = 0; frame < 100; ++frame) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	clock_t end = clock();
	compute_result(start, end, result);
	LOG_INFO("Alpha Blend Demo: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
}
