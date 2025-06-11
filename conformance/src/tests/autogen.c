#include "tests.h"
int test_call_glLogicOp(void)
{
	glLogicOp(0);
	CHECK_GLError(GL_NO_ERROR);
	return 1;
}

static const struct Test tests[] = {
	{ "call_glLogicOp", test_call_glLogicOp },
};
const struct Test *get_autogen_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
