#include "benchmark.h"
#include "gl_api_fbo.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_thread.h"
#include "gl_context.h"
#include "pipeline/gl_framebuffer.h"
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

	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	GLfloat light_diff[4] = { 1.f, 1.f, 1.f, 1.f };
	context_set_light(GL_LIGHT0, GL_DIFFUSE, light_diff);
	glEnable(GL_LIGHTING);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glEnable(GL_FOG);
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, (GLfloat[]){ 0.5f, 0.5f, 0.5f, 1.f });
	thread_pool_wait();
	clock_t start = clock();
	for (int i = 0; i < 10; ++i) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);
	}
	clock_t end = clock();

	/* Ensure the FBO is still bound before generating mipmaps. Some
         * benchmark helpers may reset the binding when syncing threads or
         * updating lighting state. */
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);
	glGenerateMipmapOES(GL_TEXTURE_2D);

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
	glDeleteFramebuffersOES(1, &fbo);
	glDeleteTextures(1, &tex);

	compute_result(start, end, result);
	LOG_INFO("Pipeline Test: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
}
