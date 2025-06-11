#include "tests.h"
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

static const struct Test tests[] = {
	{ "texture_creation", test_texture_creation },
	{ "texture_setup", test_texture_setup },
};

const struct Test *get_texture_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
