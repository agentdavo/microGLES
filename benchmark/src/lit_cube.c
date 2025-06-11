#include "benchmark.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_thread.h"

void run_lit_cube(int lighting, Framebuffer *fb, BenchmarkResult *result)
{
	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	for (int frame = 0; frame < 100; ++frame) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (lighting) {
			glEnable(GL_LIGHTING);
		} else {
			glDisable(GL_LIGHTING);
		}
	}
	clock_t end = clock();
	compute_result(start, end, result);
	LOG_INFO("Lit Cube (lighting %s): %.2f FPS, %.2f ms/frame",
		 lighting ? "on" : "off", result->fps, result->cpu_time_ms);
}
