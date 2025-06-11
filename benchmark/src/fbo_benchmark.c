#include "benchmark.h"
#include "gl_api_fbo.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_thread.h"

void run_fbo_benchmark(Framebuffer *fb, BenchmarkResult *result)
{
	GLuint rb;
	glGenRenderbuffersOES(1, &rb);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, rb);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_RGBA4_OES, 256, 256);

	GLuint fbo;
	glGenFramebuffersOES(1, &fbo);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES,
				     GL_COLOR_ATTACHMENT0_OES,
				     GL_RENDERBUFFER_OES, rb);

	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	for (int frame = 0; frame < 100; ++frame) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
	clock_t end = clock();

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
	glDeleteFramebuffersOES(1, &fbo);
	glDeleteRenderbuffersOES(1, &rb);
	compute_result(start, end, result);
	LOG_INFO("FBO Benchmark: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
}
