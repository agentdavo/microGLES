#include "tests.h"
#include <math.h>

int test_matrix_stack(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(1.0f, 2.0f, 3.0f);
	glPushMatrix();
	glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
	glPopMatrix();
	GLfloat mat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mat);
	CHECK_GLError(GL_NO_ERROR);
	(void)mat; /* suppress unused warning */
	return 1;
}

int test_matrix_overflow(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	for (int i = 0; i < 32; ++i)
		glPushMatrix();
	glPushMatrix();
	CHECK_GLError(GL_STACK_OVERFLOW);
	for (int i = 0; i < 31; ++i)
		glPopMatrix();
	CHECK_GLError(GL_NO_ERROR);
	return 1;
}

static const struct Test tests[] = {
	{ "matrix_stack", test_matrix_stack },
	{ "matrix_overflow", test_matrix_overflow },
};

const struct Test *get_matrix_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
