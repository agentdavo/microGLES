#include "tests.h"
#define GL_GLEXT_PROTOTYPES
#include <GLES/glext.h>
#include <math.h>

#define FIXED(x) ((GLfixed)((x) * 65536))
#define ENUM_X(e) ((GLfixed)((e) << 16))

static int test_clear_colorx(void)
{
	glClearColorxOES(FIXED(0.25f), FIXED(0.5f), FIXED(0.75f), FIXED(1.0f));
	GLfixed val[4] = { 0 };
	glGetFixedvOES(GL_COLOR_CLEAR_VALUE, val);
	CHECK_OK(val[0] == FIXED(0.25f) && val[1] == FIXED(0.5f) &&
		 val[2] == FIXED(0.75f) && val[3] == FIXED(1.0f));
	CHECK_GLError(GL_NO_ERROR);
	return 1;
}

static int test_scalex(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalexOES(FIXED(2.0f), FIXED(3.0f), FIXED(1.0f));
	GLfloat matf[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, matf);
	CHECK_OK(fabsf(matf[0] - 2.0f) < 0.001f &&
		 fabsf(matf[5] - 3.0f) < 0.001f &&
		 fabsf(matf[10] - 1.0f) < 0.001f &&
		 fabsf(matf[15] - 1.0f) < 0.001f);
	CHECK_GLError(GL_NO_ERROR);
	return 1;
}

static int test_tex_parameterx(void)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	glTexParameterxOES(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			   ENUM_X(GL_NEAREST));
	GLfixed vals[4];
	glGetTexParameterxvOES(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, vals);
	glDeleteTextures(1, &tex);
	CHECK_OK(vals[0] == ENUM_X(GL_NEAREST));
	CHECK_GLError(GL_NO_ERROR);
	return 1;
}

static const struct Test tests[] = {
	{ "clear_colorx", test_clear_colorx },
	{ "scalex", test_scalex },
	{ "tex_parameterx", test_tex_parameterx },
};

const struct Test *get_fixed_point_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
