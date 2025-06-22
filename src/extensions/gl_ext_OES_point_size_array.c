#include "gl_errors.h"
#include "gl_ext_common.h"
#include "../gl_utils.h"
#include "../gl_logger.h"
#include <GLES/glext.h>
EXT_REGISTER("GL_OES_point_size_array")
__attribute__((used)) int ext_link_dummy_OES_point_size_array = 0;

GL_API void GL_APIENTRY glPointSizePointerOES(GLenum type, GLsizei stride,
					      const void *pointer)
{
	gl_state.point_size_array_type = type;
	gl_state.point_size_array_stride = stride;
	gl_state.point_size_array_pointer = pointer;
	RenderContext *ctx = GetCurrentContext();
	ctx->point_size_array_type = type;
	ctx->point_size_array_stride = stride;
	ctx->point_size_array_pointer = pointer;
	if (gl_state.bound_vao) {
		gl_state.bound_vao->point_size_array_type = type;
		gl_state.bound_vao->point_size_array_stride = stride;
		gl_state.bound_vao->point_size_array_pointer = pointer;
	}
	LOG_INFO("glPointSizePointerOES set pointer=%p type=0x%X stride=%d.",
		 pointer, type, stride);
}
const void *getPointSizePointerOES(GLenum *type, GLsizei *stride)
{
	if (type)
		*type = gl_state.point_size_array_type;
	if (stride)
		*stride = gl_state.point_size_array_stride;
	return gl_state.point_size_array_pointer;
}
