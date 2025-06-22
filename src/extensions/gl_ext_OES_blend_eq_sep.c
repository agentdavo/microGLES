#include "gl_ext_common.h"
#include "gl_errors.h"
#include "../gl_state.h"
#include "../gl_utils.h"

EXT_REGISTER("GL_OES_blend_equation_separate")
__attribute__((used)) int ext_link_dummy_OES_blend_eq_sep = 0;

static GLboolean valid_blend_equation(GLenum mode)
{
	switch (mode) {
	case GL_FUNC_ADD_OES:
	case GL_FUNC_SUBTRACT_OES:
	case GL_FUNC_REVERSE_SUBTRACT_OES:
		return GL_TRUE;
	default:
		return GL_FALSE;
	}
}

GL_API void GL_APIENTRY glBlendEquationSeparateOES(GLenum modeRGB,
						   GLenum modeAlpha)
{
	if (!valid_blend_equation(modeRGB) ||
	    !valid_blend_equation(modeAlpha)) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	gl_state.blend_equation_rgb = modeRGB;
	gl_state.blend_equation_alpha = modeAlpha;
}

GL_API void GL_APIENTRY glBlendFuncSeparateOES(GLenum srcRGB, GLenum dstRGB,
					       GLenum srcAlpha, GLenum dstAlpha)
{
	gl_state.blend_sfactor = srcRGB;
	gl_state.blend_dfactor = dstRGB;
	gl_state.blend_sfactor_alpha = srcAlpha;
	gl_state.blend_dfactor_alpha = dstAlpha;
	glBlendFunc(srcRGB, dstRGB);
}
