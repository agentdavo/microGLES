#include "benchmark.h"
#include "gl_framebuffer_object.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include <time.h>

void run_pipeline_test(Framebuffer *fb, BenchmarkResult *result)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);

	GLuint fbo;
	glGenFramebuffersOES(1, &fbo);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);
	glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,
				  GL_TEXTURE_2D, tex, 0);

	GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
	LOG_INFO("Pipeline FBO status 0x%X", status);

	framebuffer_clear(fb, 0x00000000u, 1.0f);
	clock_t start = clock();
	for (int i = 0; i < 10; ++i) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
	clock_t end = clock();

	glGenerateMipmapOES(GL_TEXTURE_2D);

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
	glDeleteFramebuffersOES(1, &fbo);
	glDeleteTextures(1, &tex);

	compute_result(start, end, result);
	LOG_INFO("Pipeline Test: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
}
