#include "tests.h"
#include "util.h"
#include "gl_api_fbo.h"
#include "gl_thread.h"
#include "pipeline/gl_framebuffer.h"
#include <string.h>

int test_framebuffer_complete(void)
{
	GLuint rb;
	glGenRenderbuffersOES(1, &rb);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, rb);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_RGBA4_OES, 16, 16);

	GLuint fb;
	glGenFramebuffersOES(1, &fb);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES,
				     GL_COLOR_ATTACHMENT0_OES,
				     GL_RENDERBUFFER_OES, rb);
	GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
	glDeleteFramebuffersOES(1, &fb);
	glDeleteRenderbuffersOES(1, &rb);
	CHECK_OK(status == GL_FRAMEBUFFER_COMPLETE_OES);
	return 1;
}

int test_framebuffer_module(void)
{
	Framebuffer *fb = framebuffer_create(4, 4);
	if (!fb)
		return 0;
	framebuffer_clear(fb, 0x0000FF00u, 1.0f, 0); /* green */
	framebuffer_set_pixel(fb, 1, 1, 0x00FF0000u, 0.0f); /* red */
	int ok = framebuffer_write_rgba(fb, "fb_test_out.rgba");
	thread_pool_wait_timeout(1000);
	framebuffer_destroy(fb);
	if (ok)
		ok = compare_rgba("gold/fb_test.rgba", "fb_test_out.rgba");
	return ok;
}

int test_async_clear_destroy(void)
{
	Framebuffer *fb = framebuffer_create(8, 8);
	if (!fb)
		return 0;
	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	framebuffer_destroy(fb);
	thread_pool_wait();
	return 1;
}

static const struct Test tests[] = {
	{ "framebuffer_complete", test_framebuffer_complete },
	{ "framebuffer_module", test_framebuffer_module },
	{ "async_clear_destroy", test_async_clear_destroy },
};

const struct Test *get_fbo_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
