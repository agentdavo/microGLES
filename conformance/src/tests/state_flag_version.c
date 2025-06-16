#include "tests.h"
#include "gl_context.h"
#include <stdatomic.h>

int test_state_flag_version(void)
{
	RenderContext *ctx = GetCurrentContext();
	glDisable(GL_DITHER);
	unsigned v = atomic_load(&ctx->version_dither);
	glEnable(GL_DITHER);
	unsigned v1 = atomic_load(&ctx->version_dither);
	CHECK_OK(v1 == v + 1);
	glDisable(GL_DITHER);
	unsigned v2 = atomic_load(&ctx->version_dither);
	CHECK_OK(v2 == v1 + 1);
	return 1;
}
