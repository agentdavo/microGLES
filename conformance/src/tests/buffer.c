#include "tests.h"

int test_buffer_objects(void)
{
	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	int data = 1234;
	glBufferData(GL_ARRAY_BUFFER, sizeof(int), &data, GL_STATIC_DRAW);
	CHECK_OK(glIsBuffer(buf));
	GLint size = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	CHECK_OK(size == (GLint)sizeof(int));
	glDeleteBuffers(1, &buf);
	return 1;
}

static const struct Test tests[] = {
	{ "buffer_objects", test_buffer_objects },
};

const struct Test *get_buffer_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
