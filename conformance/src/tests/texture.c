#include "tests.h"
#include "util.h"
#include "ktx_loader.h"
#include "gl_init.h"
#include "gl_thread.h"
#include "gl_context.h"
#include <string.h>

int test_texture_creation(void)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	glDeleteTextures(1, &tex);
	CHECK_GLError(GL_NO_ERROR);
	return 1;
}

int test_texture_setup(void)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	CHECK_OK(glIsTexture(tex));
	GLint binding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &binding);
	CHECK_OK(binding == (GLint)tex);
	glDeleteTextures(1, &tex);
	return 1;
}

int test_load_ktx(void)
{
	GLuint tex = 0;
	KTXError err = load_ktx_texture("gold/red.ktx", &tex);
	if (err != KTX_SUCCESS) {
		LOG_ERROR("KTX load error %d", err);
		return 0;
	}
	TextureOES *texObj = context_find_texture(tex);
	int ok = texObj && texObj->mip_width[0] == 4 &&
		 texObj->mip_height[0] == 4 &&
		 texObj->levels[0][0] == 0xFF0000FFu;
	glDeleteTextures(1, &tex);
	return ok;
}

static const struct Test tests[] = {
	{ "texture_creation", test_texture_creation },
	{ "texture_setup", test_texture_setup },
	{ "load_ktx", test_load_ktx },
};

const struct Test *get_texture_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
