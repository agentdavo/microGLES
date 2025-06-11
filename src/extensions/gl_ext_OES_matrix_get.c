#include "gl_ext_common.h"
#include <GLES/glext.h>
#include <math.h>
EXT_REGISTER("GL_OES_matrix_get")
int ext_link_dummy_OES_matrix_get;

GLbitfield glQueryMatrixxOES(GLfixed *mantissa, GLint *exponent)
{
	if (!mantissa || !exponent)
		return 0;
	GLbitfield status = 0;
	for (int i = 0; i < 16; ++i) {
		float val = gl_state.modelview_matrix.data[i];
		int exp;
		float m = frexpf(val, &exp);
		mantissa[i] = (GLfixed)(m * 65536.0f);
		exponent[i] = exp;
	}
	return status;
}
