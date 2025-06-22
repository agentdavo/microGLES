#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include <GLES/gl.h>
#include "gl_utils.h"

GL_API void GL_APIENTRY glClearDepthf(GLfloat d)
{
	gl_state.clear_depth = d;
}

GL_API void GL_APIENTRY glDepthFunc(GLenum func)
{
	gl_state.depth_func = func;
	context_set_depth_func(func);
}

GL_API void GL_APIENTRY glDepthMask(GLboolean flag)
{
	gl_state.depth_mask = flag;
}

GL_API void GL_APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	gl_state.stencil_func = func;
	gl_state.stencil_ref = ref;
	gl_state.stencil_value_mask = mask;
	context_set_stencil_func(func, ref, mask);
}

GL_API void GL_APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	gl_state.stencil_fail = fail;
	gl_state.stencil_zfail = zfail;
	gl_state.stencil_zpass = zpass;
	context_set_stencil_op(fail, zfail, zpass);
}

GL_API void GL_APIENTRY glStencilMask(GLuint mask)
{
	gl_state.stencil_writemask = mask;
}

GL_API void GL_APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
	gl_state.polygon_offset_factor = factor;
	gl_state.polygon_offset_units = units;
}

GL_API void GL_APIENTRY glPolygonOffsetx(GLfixed factor, GLfixed units)
{
	glPolygonOffset(fixed_to_float(factor), fixed_to_float(units));
}

GL_API void GL_APIENTRY glClearStencil(GLint s)
{
	gl_state.clear_stencil = s;
	context_set_clear_stencil(s);
}
