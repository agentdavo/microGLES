#include "benchmark.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_thread.h"

void run_toggle_blend(Framebuffer *fb, BenchmarkResult *result)
{
	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	for (int i = 0; i < 1000000; ++i) {
		glEnable(GL_BLEND);
		glDisable(GL_BLEND);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	clock_t end = clock();
	compute_result(start, end, result);
	LOG_INFO("Toggle Blend: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
}
