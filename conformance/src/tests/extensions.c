#include "tests.h"

static int test_get_string(void)
{
	const GLubyte *vendor = glGetString(GL_VENDOR);
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *ext = glGetString(GL_EXTENSIONS);
	CHECK_OK(vendor && renderer && version && ext);
	glGetString(0);
	CHECK_GLError(GL_INVALID_ENUM);
	return 1;
}

static const struct Test tests[] = { { "get_string", test_get_string } };

const struct Test *get_extensions_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
