#include "benchmark.h"
#include "gl_types.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_thread.h"
#include <string.h>

void run_multitexture_demo(Framebuffer *fb, BenchmarkResult *result)
{
	const int size = 64;
	const int frames = 100;
	GLubyte *tex1 = tracked_malloc(size * size * 4);
	GLubyte *tex2 = tracked_malloc(size * size * 4);
	memset(tex1, 0xFF, size * size * 4);
	memset(tex2, 0x80, size * size * 4);

	GLuint t1, t2;
	glGenTextures(1, &t1);
	glGenTextures(1, &t2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, t1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, tex1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, t2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, tex2);
	glActiveTexture(GL_TEXTURE0);

	GLfloat verts[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
	GLfloat uv[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	for (int f = 0; f < frames; ++f) {
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, t1);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glClientActiveTexture(GL_TEXTURE0);
		glTexCoordPointer(2, GL_FLOAT, 0, uv);

		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, t2);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glClientActiveTexture(GL_TEXTURE1);
		glTexCoordPointer(2, GL_FLOAT, 0, uv);

		glVertexPointer(2, GL_FLOAT, 0, verts);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	clock_t end = clock();

	glDeleteTextures(1, &t1);
	glDeleteTextures(1, &t2);
	tracked_free(tex1, size * size * 4);
	tracked_free(tex2, size * size * 4);

	compute_result(start, end, result);
	double secs = (double)(end - start) / CLOCKS_PER_SEC;
	result->pixels_per_second = (double)(size * size * frames) / secs;
	LOG_INFO("Multitexture Demo: %.2f MP/s",
		 result->pixels_per_second / 1e6);
}
