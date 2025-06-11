#include "tests.h"

static const struct Test tests[] = {};

const struct Test *get_extensions_tests(size_t *count)
{
	*count = 0;
	return tests;
}
