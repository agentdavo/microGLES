#include "gl_context.h"
#include "gl_state.h"
#include "gl_utils.h"
#include <GLES/gl.h>

GL_API void GL_APIENTRY glAlphaFunc(GLenum func, GLfloat ref)
{
	gl_state.alpha_func = func;
	gl_state.alpha_ref = ref;
	context_set_alpha_func(func, ref);
}

GL_API void GL_APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	gl_state.blend_sfactor = sfactor;
	gl_state.blend_dfactor = dfactor;
	gl_state.blend_sfactor_alpha = sfactor;
	gl_state.blend_dfactor_alpha = dfactor;
	context_set_blend_func(sfactor, dfactor);
}

GL_API void GL_APIENTRY glLogicOp(GLenum opcode)
{
	gl_state.logic_op_mode = opcode;
}
