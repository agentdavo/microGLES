#include "tests.h"
#include "gl_context.h"
#include <math.h>
#include <stdatomic.h>

int test_enable_disable(void)
{
	glEnable(GL_CULL_FACE);
	CHECK_OK(glIsEnabled(GL_CULL_FACE));
	glDisable(GL_CULL_FACE);
	CHECK_OK(!glIsEnabled(GL_CULL_FACE));
	return 1;
}

int test_viewport(void)
{
	glViewport(2, 3, 100, 200);
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	CHECK_OK(vp[0] == 2 && vp[1] == 3 && vp[2] == 100 && vp[3] == 200);
	return 1;
}

int test_clear_state(void)
{
	glClearColor(0.1f, 0.2f, 0.3f, 0.4f);
	glClearDepthf(0.5f);
	glClearStencil(2);
	GLfloat color[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
	GLint stencil;
	GLfloat depth;
	glGetFloatv(GL_DEPTH_CLEAR_VALUE, &depth);
	glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &stencil);
	CHECK_OK(fabsf(color[0] - 0.1f) < 0.001f &&
		 fabsf(color[1] - 0.2f) < 0.001f &&
		 fabsf(color[2] - 0.3f) < 0.001f &&
		 fabsf(color[3] - 0.4f) < 0.001f);
	CHECK_OK(fabsf(depth - 0.5f) < 0.001f && stencil == 2);
	return 1;
}

int test_blend_func(void)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	GLint src, dst;
	glGetIntegerv(GL_BLEND_SRC, &src);
	glGetIntegerv(GL_BLEND_DST, &dst);
	CHECK_OK(src == GL_SRC_ALPHA && dst == GL_ONE_MINUS_SRC_ALPHA);
	CHECK_OK(glIsEnabled(GL_BLEND));
	glDisable(GL_BLEND);
	return 1;
}

int test_scissor_state(void)
{
	glScissor(1, 2, 3, 4);
	glEnable(GL_SCISSOR_TEST);
	GLint box[4];
	glGetIntegerv(GL_SCISSOR_BOX, box);
	CHECK_OK(glIsEnabled(GL_SCISSOR_TEST));
	CHECK_OK(box[0] == 1 && box[1] == 2 && box[2] == 3 && box[3] == 4);
	glDisable(GL_SCISSOR_TEST);
	return 1;
}

int test_mask_state(void)
{
	glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
	glDepthMask(GL_FALSE);
	GLboolean cm[4];
	glGetBooleanv(GL_COLOR_WRITEMASK, cm);
	GLboolean dm;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &dm);
	CHECK_OK(cm[0] == GL_TRUE && cm[1] == GL_FALSE && cm[2] == GL_TRUE &&
		 cm[3] == GL_FALSE && dm == GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	return 1;
}

int test_depth_alpha_stencil(void)
{
	glAlphaFunc(GL_GREATER, 0.5f);
	glDepthFunc(GL_LEQUAL);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_INCR, GL_DECR);
	GLint func, ref, mask, fail, zfail, zpass;
	GLfloat ref_f;
	glGetIntegerv(GL_STENCIL_FUNC, &func);
	glGetIntegerv(GL_STENCIL_REF, &ref);
	glGetIntegerv(GL_STENCIL_VALUE_MASK, &mask);
	glGetIntegerv(GL_STENCIL_FAIL, &fail);
	glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &zfail);
	glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &zpass);
	glGetFloatv(GL_ALPHA_TEST_REF, &ref_f);
	GLint depth_func;
	glGetIntegerv(GL_DEPTH_FUNC, &depth_func);
	CHECK_OK(func == GL_ALWAYS && ref == 1 && mask == 0xFF);
	CHECK_OK(fail == GL_KEEP && zfail == GL_INCR && zpass == GL_DECR);
	CHECK_OK(depth_func == GL_LEQUAL);
	CHECK_OK(fabsf(ref_f - 0.5f) < 0.001f);
	return 1;
}

int test_state_flag_version(void);

static const struct Test tests[] = {
	{ "enable_disable", test_enable_disable },
	{ "viewport", test_viewport },
	{ "clear_state", test_clear_state },
	{ "blend_func", test_blend_func },
	{ "scissor_state", test_scissor_state },
	{ "mask_state", test_mask_state },
	{ "depth_alpha_stencil", test_depth_alpha_stencil },
	{ "state_flag_version", test_state_flag_version },
};

const struct Test *get_state_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
