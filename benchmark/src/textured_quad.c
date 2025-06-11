#include "benchmark.h"
#include "gl_types.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include <string.h>

void run_textured_quad(Framebuffer *fb, BenchmarkResult *result)
{
	GLubyte *tex_data = tracked_malloc(256 * 256 * 3);
	memset(tex_data, 0xFF, 256 * 256 * 3);
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB,
		     GL_UNSIGNED_BYTE, tex_data);
	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	for (int frame = 0; frame < 100; ++frame) {
		glClear(GL_COLOR_BUFFER_BIT);
		volatile GLubyte v = tex_data[frame % (256 * 256 * 3)];
		(void)v;
	}
	clock_t end = clock();
	glDeleteTextures(1, &tex);
	tracked_free(tex_data, 256 * 256 * 3);
	compute_result(start, end, result);
	LOG_INFO("Textured Quad: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
}
