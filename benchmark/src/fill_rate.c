#include "benchmark.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include <string.h>

static void run_fill_clear(Framebuffer *fb, BenchmarkResult *result)
{
	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	for (int frame = 0; frame < 100; ++frame)
		glClear(GL_COLOR_BUFFER_BIT);
	clock_t end = clock();
	compute_result(start, end, result);
	double secs = (double)(end - start) / CLOCKS_PER_SEC;
	result->pixels_per_second =
		(double)(fb->width * fb->height * 100) / secs;
}

static void run_fill_textured(Framebuffer *fb, BenchmarkResult *result)
{
	GLubyte *tex_data = tracked_malloc(fb->width * fb->height * 4);
	memset(tex_data, 0x80, fb->width * fb->height * 4);
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb->width, fb->height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, tex_data);

	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	for (int frame = 0; frame < 100; ++frame) {
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	clock_t end = clock();

	glDeleteTextures(1, &tex);
	tracked_free(tex_data, fb->width * fb->height * 4);

	compute_result(start, end, result);
	double secs = (double)(end - start) / CLOCKS_PER_SEC;
	result->pixels_per_second =
		(double)(fb->width * fb->height * 100) / secs;
}

static void run_fill_upload(Framebuffer *fb, BenchmarkResult *result)
{
	GLubyte *data = tracked_malloc(fb->width * fb->height * 4);
	memset(data, 0xFF, fb->width * fb->height * 4);
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb->width, fb->height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	clock_t start = clock();
	for (int frame = 0; frame < 10; ++frame) {
		for (int i = 0; i < 100; ++i)
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fb->width,
					fb->height, GL_RGBA, GL_UNSIGNED_BYTE,
					data);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	clock_t end = clock();

	glDeleteTextures(1, &tex);
	tracked_free(data, fb->width * fb->height * 4);

	compute_result(start, end, result);
	double secs = (double)(end - start) / CLOCKS_PER_SEC;
	result->pixels_per_second =
		(double)(fb->width * fb->height * 1000) / secs;
}

void run_fill_rate_suite(Framebuffer *fb, BenchmarkResult results[3])
{
	run_fill_clear(fb, &results[0]);
	LOG_INFO("Clear Fill: %.2f MP/s", results[0].pixels_per_second / 1e6);

	run_fill_textured(fb, &results[1]);
	LOG_INFO("Textured Fill: %.2f MP/s",
		 results[1].pixels_per_second / 1e6);

	run_fill_upload(fb, &results[2]);
	LOG_INFO("Texture Upload: %.2f MP/s",
		 results[2].pixels_per_second / 1e6);

	LOG_INFO("| Fill Test | MP/s |");
	LOG_INFO("|-----------|------|");
	LOG_INFO("| Clear     | %.2f |", results[0].pixels_per_second / 1e6);
	LOG_INFO("| Textured  | %.2f |", results[1].pixels_per_second / 1e6);
	LOG_INFO("| Upload    | %.2f |", results[2].pixels_per_second / 1e6);
}
