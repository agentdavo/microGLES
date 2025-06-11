#include "benchmark.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_thread.h"
#include "matrix_utils.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Simulate the computations from the classic gears demo without
 * relying on immediate mode rendering. The math roughly mirrors
 * the geometry generation from Brian Paul's gears.c. */
static void gear(float inner_radius, float outer_radius, float width, int teeth,
		 float tooth_depth, volatile float *accum)
{
	float r0 = inner_radius;
	float r1 = outer_radius - tooth_depth / 2.0f;
	float r2 = outer_radius + tooth_depth / 2.0f;
	float da = 2.0f * M_PI / teeth / 4.0f;

	for (int i = 0; i < teeth; ++i) {
		float angle = i * 2.0f * M_PI / teeth;
		/* compute four vertices for the front face of each tooth */
		float x1 = r1 * cosf(angle);
		float y1 = r1 * sinf(angle);
		float x2 = r2 * cosf(angle + da);
		float y2 = r2 * sinf(angle + da);
		float x3 = r2 * cosf(angle + 2.0f * da);
		float y3 = r2 * sinf(angle + 2.0f * da);
		float x4 = r1 * cosf(angle + 3.0f * da);
		float y4 = r1 * sinf(angle + 3.0f * da);
		/* accumulate some computations so the optimizer can't remove them */
		*accum += x1 + y1 + x2 + y2 + x3 + y3 + x4 + y4 + width;
	}
}

void run_spinning_gears(Framebuffer *fb, BenchmarkResult *result)
{
	mat4 model;
	mat4_identity(&model);
	float angle = 0.0f;
	volatile float accum = 0.0f;

	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	for (int frame = 0; frame < 100; ++frame) {
		angle += 2.0f;
		mat4_rotate_z(&model, 2.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gear(1.0f, 4.0f, 1.0f, 20, 0.7f, &accum);
		gear(0.5f, 2.0f, 2.0f, 10, 0.7f, &accum);
		gear(1.3f, 2.0f, 0.5f, 10, 0.7f, &accum);
		(void)angle; /* suppress unused variable warning */
	}
	clock_t end = clock();

	compute_result(start, end, result);
	LOG_INFO("Spinning Gears: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
	LOG_DEBUG("Gear accumulation value: %f", accum);
}
