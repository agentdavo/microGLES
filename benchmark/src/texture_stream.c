#include "benchmark.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include <string.h>

void run_texture_stream(Framebuffer *fb, BenchmarkResult *result)
{
	GLubyte *data = tracked_malloc(256 * 256 * 4);
	memset(data, 0xFF, 256 * 256 * 4);
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);

	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	clock_t start = clock();
	for (int frame = 0; frame < 10; ++frame) {
		for (int i = 0; i < 500; ++i)
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256,
					GL_RGBA, GL_UNSIGNED_BYTE, data);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	clock_t end = clock();

	glDeleteTextures(1, &tex);
	tracked_free(data, 256 * 256 * 4);

	compute_result(start, end, result);
	LOG_INFO("Texture Stream: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
}
