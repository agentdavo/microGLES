#include "tests.h"
#include "pipeline/gl_framebuffer.h"
#include <GLES/gl.h>

static int test_depth_comparisons(void)
{
	Framebuffer *fb = framebuffer_create(1, 1);
	if (!fb)
		return 0;

	struct Case {
		GLenum func;
		float depth;
		int pass;
	} cases[] = {
		{ GL_LESS, 0.4f, 1 },	 { GL_LEQUAL, 0.5f, 1 },
		{ GL_GREATER, 0.4f, 0 }, { GL_GEQUAL, 0.5f, 1 },
		{ GL_EQUAL, 0.5f, 1 },	 { GL_NOTEQUAL, 0.5f, 0 },
		{ GL_NEVER, 0.4f, 0 },	 { GL_ALWAYS, 0.4f, 1 },
	};

	int ok = 1;
	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
		framebuffer_clear(fb, 0x00000000u, 0.5f, 0);
		glDepthFunc(cases[i].func);
		framebuffer_set_pixel(fb, 0, 0, 0x00FF0000u, cases[i].depth);
		uint32_t c = framebuffer_get_pixel(fb, 0, 0);
		if (cases[i].pass)
			ok &= (c == 0x00FF0000u);
		else
			ok &= (c == 0x00000000u);
		if (!ok)
			break;
	}

	framebuffer_destroy(fb);
	return ok;
}

static const struct Test tests[] = {
	{ "depth_comparisons", test_depth_comparisons },
};

const struct Test *get_depth_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
