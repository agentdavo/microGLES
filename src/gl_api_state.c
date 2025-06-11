#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include <GLES/gl.h>
#include <string.h>

GL_API void GL_APIENTRY glEnable(GLenum cap)
{
	switch (cap) {
	case GL_ALPHA_TEST:
		gl_state.alpha_test_enabled = GL_TRUE;
		GetCurrentContext()->alpha_test.enabled = GL_TRUE;
		break;
	case GL_BLEND:
		gl_state.blend_enabled = GL_TRUE;
		GetCurrentContext()->blend_enabled = GL_TRUE;
		break;
	case GL_COLOR_LOGIC_OP:
		gl_state.color_logic_op_enabled = GL_TRUE;
		break;
	case GL_COLOR_MATERIAL:
		gl_state.color_material_enabled = GL_TRUE;
		break;
	case GL_CULL_FACE:
		gl_state.cull_face_enabled = GL_TRUE;
		break;
	case GL_DEPTH_TEST:
		gl_state.depth_test_enabled = GL_TRUE;
		break;
	case GL_DITHER:
		gl_state.dither_enabled = GL_TRUE;
		break;
	case GL_FOG:
		gl_state.fog_enabled = GL_TRUE;
		GetCurrentContext()->fog.enabled = GL_TRUE;
		break;
	case GL_LIGHTING:
		gl_state.lighting_enabled = GL_TRUE;
		break;
	case GL_LINE_SMOOTH:
		gl_state.line_smooth_enabled = GL_TRUE;
		break;
	case GL_MULTISAMPLE:
		gl_state.multisample_enabled = GL_TRUE;
		break;
	case GL_NORMALIZE:
		gl_state.normalize_enabled = GL_TRUE;
		break;
	case GL_POINT_SMOOTH:
		gl_state.point_smooth_enabled = GL_TRUE;
		break;
	case GL_POINT_SPRITE_OES:
		gl_state.point_sprite_enabled = GL_TRUE;
		break;
	case GL_POLYGON_OFFSET_FILL:
		gl_state.polygon_offset_fill_enabled = GL_TRUE;
		break;
	case GL_RESCALE_NORMAL:
		gl_state.rescale_normal_enabled = GL_TRUE;
		break;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		gl_state.sample_alpha_to_coverage_enabled = GL_TRUE;
		break;
	case GL_SAMPLE_ALPHA_TO_ONE:
		gl_state.sample_alpha_to_one_enabled = GL_TRUE;
		break;
	case GL_SAMPLE_COVERAGE:
		gl_state.sample_coverage_enabled = GL_TRUE;
		break;
	case GL_SCISSOR_TEST:
		gl_state.scissor_test_enabled = GL_TRUE;
		break;
	case GL_STENCIL_TEST:
		gl_state.stencil_test_enabled = GL_TRUE;
		GetCurrentContext()->stencil_test_enabled = GL_TRUE;
		break;
	case GL_TEXTURE_2D:
		gl_state.texture_2d_enabled = GL_TRUE;
		break;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:
		gl_state.clip_plane_enabled[cap - GL_CLIP_PLANE0] = GL_TRUE;
		break;
	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		gl_state.light_enabled[cap - GL_LIGHT0] = GL_TRUE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glDisable(GLenum cap)
{
	switch (cap) {
	case GL_ALPHA_TEST:
		gl_state.alpha_test_enabled = GL_FALSE;
		GetCurrentContext()->alpha_test.enabled = GL_FALSE;
		break;
	case GL_BLEND:
		gl_state.blend_enabled = GL_FALSE;
		GetCurrentContext()->blend_enabled = GL_FALSE;
		break;
	case GL_COLOR_LOGIC_OP:
		gl_state.color_logic_op_enabled = GL_FALSE;
		break;
	case GL_COLOR_MATERIAL:
		gl_state.color_material_enabled = GL_FALSE;
		break;
	case GL_CULL_FACE:
		gl_state.cull_face_enabled = GL_FALSE;
		break;
	case GL_DEPTH_TEST:
		gl_state.depth_test_enabled = GL_FALSE;
		break;
	case GL_DITHER:
		gl_state.dither_enabled = GL_FALSE;
		break;
	case GL_FOG:
		gl_state.fog_enabled = GL_FALSE;
		GetCurrentContext()->fog.enabled = GL_FALSE;
		break;
	case GL_LIGHTING:
		gl_state.lighting_enabled = GL_FALSE;
		break;
	case GL_LINE_SMOOTH:
		gl_state.line_smooth_enabled = GL_FALSE;
		break;
	case GL_MULTISAMPLE:
		gl_state.multisample_enabled = GL_FALSE;
		break;
	case GL_NORMALIZE:
		gl_state.normalize_enabled = GL_FALSE;
		break;
	case GL_POINT_SMOOTH:
		gl_state.point_smooth_enabled = GL_FALSE;
		break;
	case GL_POINT_SPRITE_OES:
		gl_state.point_sprite_enabled = GL_FALSE;
		break;
	case GL_POLYGON_OFFSET_FILL:
		gl_state.polygon_offset_fill_enabled = GL_FALSE;
		break;
	case GL_RESCALE_NORMAL:
		gl_state.rescale_normal_enabled = GL_FALSE;
		break;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		gl_state.sample_alpha_to_coverage_enabled = GL_FALSE;
		break;
	case GL_SAMPLE_ALPHA_TO_ONE:
		gl_state.sample_alpha_to_one_enabled = GL_FALSE;
		break;
	case GL_SAMPLE_COVERAGE:
		gl_state.sample_coverage_enabled = GL_FALSE;
		break;
	case GL_SCISSOR_TEST:
		gl_state.scissor_test_enabled = GL_FALSE;
		break;
	case GL_STENCIL_TEST:
		gl_state.stencil_test_enabled = GL_FALSE;
		GetCurrentContext()->stencil_test_enabled = GL_FALSE;
		break;
	case GL_TEXTURE_2D:
		gl_state.texture_2d_enabled = GL_FALSE;
		break;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:
		gl_state.clip_plane_enabled[cap - GL_CLIP_PLANE0] = GL_FALSE;
		break;
	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		gl_state.light_enabled[cap - GL_LIGHT0] = GL_FALSE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GLboolean glIsEnabled(GLenum cap)
{
	switch (cap) {
	case GL_ALPHA_TEST:
		return gl_state.alpha_test_enabled;
	case GL_BLEND:
		return gl_state.blend_enabled;
	case GL_COLOR_LOGIC_OP:
		return gl_state.color_logic_op_enabled;
	case GL_COLOR_MATERIAL:
		return gl_state.color_material_enabled;
	case GL_CULL_FACE:
		return gl_state.cull_face_enabled;
	case GL_DEPTH_TEST:
		return gl_state.depth_test_enabled;
	case GL_DITHER:
		return gl_state.dither_enabled;
	case GL_FOG:
		return gl_state.fog_enabled;
	case GL_LIGHTING:
		return gl_state.lighting_enabled;
	case GL_LINE_SMOOTH:
		return gl_state.line_smooth_enabled;
	case GL_MULTISAMPLE:
		return gl_state.multisample_enabled;
	case GL_NORMALIZE:
		return gl_state.normalize_enabled;
	case GL_POINT_SMOOTH:
		return gl_state.point_smooth_enabled;
	case GL_POINT_SPRITE_OES:
		return gl_state.point_sprite_enabled;
	case GL_POLYGON_OFFSET_FILL:
		return gl_state.polygon_offset_fill_enabled;
	case GL_RESCALE_NORMAL:
		return gl_state.rescale_normal_enabled;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		return gl_state.sample_alpha_to_coverage_enabled;
	case GL_SAMPLE_ALPHA_TO_ONE:
		return gl_state.sample_alpha_to_one_enabled;
	case GL_SAMPLE_COVERAGE:
		return gl_state.sample_coverage_enabled;
	case GL_SCISSOR_TEST:
		return gl_state.scissor_test_enabled;
	case GL_STENCIL_TEST:
		return gl_state.stencil_test_enabled;
	case GL_TEXTURE_2D:
		return gl_state.texture_2d_enabled;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:
		return gl_state.clip_plane_enabled[cap - GL_CLIP_PLANE0];
	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		return gl_state.light_enabled[cap - GL_LIGHT0];
	default:
		glSetError(GL_INVALID_ENUM);
		return GL_FALSE;
	}
}

GL_API void GL_APIENTRY glEnableClientState(GLenum array)
{
	switch (array) {
	case GL_VERTEX_ARRAY:
		gl_state.vertex_array_enabled = GL_TRUE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->vertex_array_enabled = GL_TRUE;
		break;
	case GL_COLOR_ARRAY:
		gl_state.color_array_enabled = GL_TRUE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->color_array_enabled = GL_TRUE;
		break;
	case GL_NORMAL_ARRAY:
		gl_state.normal_array_enabled = GL_TRUE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->normal_array_enabled = GL_TRUE;
		break;
	case GL_TEXTURE_COORD_ARRAY:
		gl_state.texcoord_array_enabled = GL_TRUE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->texcoord_array_enabled = GL_TRUE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glDisableClientState(GLenum array)
{
	switch (array) {
	case GL_VERTEX_ARRAY:
		gl_state.vertex_array_enabled = GL_FALSE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->vertex_array_enabled = GL_FALSE;
		break;
	case GL_COLOR_ARRAY:
		gl_state.color_array_enabled = GL_FALSE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->color_array_enabled = GL_FALSE;
		break;
	case GL_NORMAL_ARRAY:
		gl_state.normal_array_enabled = GL_FALSE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->normal_array_enabled = GL_FALSE;
		break;
	case GL_TEXTURE_COORD_ARRAY:
		gl_state.texcoord_array_enabled = GL_FALSE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->texcoord_array_enabled = GL_FALSE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glClientActiveTexture(GLenum texture)
{
	gl_state.client_active_texture = texture;
}

GL_API void GL_APIENTRY glHint(GLenum target, GLenum mode)
{
	if (mode != GL_FASTEST && mode != GL_NICEST && mode != GL_DONT_CARE) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	switch (target) {
	case GL_FOG_HINT:
		gl_state.fog_hint = mode;
		break;
	case GL_GENERATE_MIPMAP_HINT:
		gl_state.generate_mipmap_hint = mode;
		break;
	case GL_LINE_SMOOTH_HINT:
		gl_state.line_smooth_hint = mode;
		break;
	case GL_PERSPECTIVE_CORRECTION_HINT:
		gl_state.perspective_correction_hint = mode;
		break;
	case GL_POINT_SMOOTH_HINT:
		gl_state.point_smooth_hint = mode;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glCullFace(GLenum mode)
{
	gl_state.cull_face_mode = mode;
}

GL_API void GL_APIENTRY glFrontFace(GLenum mode)
{
	gl_state.front_face = mode;
}

GL_API void GL_APIENTRY glShadeModel(GLenum mode)
{
	if (mode != GL_FLAT && mode != GL_SMOOTH) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	gl_state.shade_model = mode;
}

GL_API void GL_APIENTRY glViewport(GLint x, GLint y, GLsizei width,
				   GLsizei height)
{
	if (width < 0 || height < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	gl_state.viewport[0] = x;
	gl_state.viewport[1] = y;
	gl_state.viewport[2] = width;
	gl_state.viewport[3] = height;
}

GL_API void GL_APIENTRY glScissor(GLint x, GLint y, GLsizei width,
				  GLsizei height)
{
	if (width < 0 || height < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	gl_state.scissor_box[0] = x;
	gl_state.scissor_box[1] = y;
	gl_state.scissor_box[2] = width;
	gl_state.scissor_box[3] = height;
}

GL_API void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean *data)
{
	if (!data)
		return;
	switch (pname) {
	case GL_COLOR_WRITEMASK:
		data[0] = gl_state.color_mask[0];
		data[1] = gl_state.color_mask[1];
		data[2] = gl_state.color_mask[2];
		data[3] = gl_state.color_mask[3];
		break;
	case GL_DEPTH_WRITEMASK:
		*data = gl_state.depth_mask;
		break;
	case GL_LIGHT_MODEL_TWO_SIDE:
		*data = gl_state.light_model_two_side;
		break;
	case GL_SAMPLE_COVERAGE_INVERT:
		*data = gl_state.sample_coverage_invert;
		break;
	default:
		*data = glIsEnabled(pname);
		break;
	}
}

GL_API void GL_APIENTRY glGetFloatv(GLenum pname, GLfloat *data)
{
	if (!data)
		return;
	switch (pname) {
	case GL_COLOR_CLEAR_VALUE:
		data[0] = gl_state.clear_color[0];
		data[1] = gl_state.clear_color[1];
		data[2] = gl_state.clear_color[2];
		data[3] = gl_state.clear_color[3];
		break;
	case GL_DEPTH_CLEAR_VALUE:
		data[0] = gl_state.clear_depth;
		break;
	case GL_MODELVIEW_MATRIX:
		memcpy(data, gl_state.modelview_matrix.data,
		       sizeof(GLfloat) * 16);
		break;
	case GL_PROJECTION_MATRIX:
		memcpy(data, gl_state.projection_matrix.data,
		       sizeof(GLfloat) * 16);
		break;
	case GL_TEXTURE_MATRIX:
		memcpy(data, gl_state.texture_matrix.data,
		       sizeof(GLfloat) * 16);
		break;
	case GL_LIGHT_MODEL_AMBIENT:
		memcpy(data, gl_state.light_model_ambient, sizeof(GLfloat) * 4);
		break;
	case GL_DEPTH_RANGE:
		data[0] = gl_state.depth_range_near;
		data[1] = gl_state.depth_range_far;
		break;
	case GL_SAMPLE_COVERAGE_VALUE:
		data[0] = gl_state.sample_coverage_value;
		break;
	case GL_LINE_WIDTH:
		data[0] = gl_state.line_width;
		break;
	default:
		break;
	}
}

GL_API void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *data)
{
	if (!data)
		return;
	switch (pname) {
	case GL_VIEWPORT:
		data[0] = gl_state.viewport[0];
		data[1] = gl_state.viewport[1];
		data[2] = gl_state.viewport[2];
		data[3] = gl_state.viewport[3];
		break;
	case GL_ACTIVE_TEXTURE:
		*data = gl_state.active_texture;
		break;
	case GL_CLIENT_ACTIVE_TEXTURE:
		*data = gl_state.client_active_texture;
		break;
	case GL_ARRAY_BUFFER_BINDING:
		*data = gl_state.array_buffer_binding;
		break;
	case GL_ELEMENT_ARRAY_BUFFER_BINDING:
		*data = gl_state.element_array_buffer_binding;
		break;
	case GL_TEXTURE_BINDING_2D:
		*data = gl_state.bound_texture;
		break;
	case GL_TEXTURE_BINDING_EXTERNAL_OES:
		*data = gl_state.bound_texture_external;
		break;
	case GL_CULL_FACE_MODE:
		*data = gl_state.cull_face_mode;
		break;
	case GL_FRONT_FACE:
		*data = gl_state.front_face;
		break;
	case GL_MATRIX_MODE:
		*data = gl_state.matrix_mode;
		break;
	case GL_BLEND_SRC:
		*data = gl_state.blend_sfactor;
		break;
	case GL_BLEND_DST:
		*data = gl_state.blend_dfactor;
		break;
	case GL_DEPTH_FUNC:
		*data = gl_state.depth_func;
		break;
	case GL_DEPTH_RANGE:
		data[0] = (GLint)gl_state.depth_range_near;
		data[1] = (GLint)gl_state.depth_range_far;
		break;
	case GL_LOGIC_OP_MODE:
		*data = gl_state.logic_op_mode;
		break;
	case GL_STENCIL_FUNC:
		*data = gl_state.stencil_func;
		break;
	case GL_STENCIL_REF:
		*data = gl_state.stencil_ref;
		break;
	case GL_STENCIL_VALUE_MASK:
		*data = (GLint)gl_state.stencil_value_mask;
		break;
	case GL_STENCIL_WRITEMASK:
		*data = (GLint)gl_state.stencil_writemask;
		break;
	case GL_STENCIL_FAIL:
		*data = gl_state.stencil_fail;
		break;
	case GL_STENCIL_PASS_DEPTH_FAIL:
		*data = gl_state.stencil_zfail;
		break;
	case GL_STENCIL_PASS_DEPTH_PASS:
		*data = gl_state.stencil_zpass;
		break;
	case GL_STENCIL_CLEAR_VALUE:
		*data = gl_state.clear_stencil;
		break;
	case GL_LINE_WIDTH:
		*data = (GLint)gl_state.line_width;
		break;
	default:
		break;
	}
}

GL_API void GL_APIENTRY glGetPointerv(GLenum pname, void **params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	switch (pname) {
	case GL_VERTEX_ARRAY_POINTER:
		*params = (void *)gl_state.vertex_array_pointer;
		break;
	case GL_COLOR_ARRAY_POINTER:
		*params = (void *)gl_state.color_array_pointer;
		break;
	case GL_NORMAL_ARRAY_POINTER:
		*params = (void *)gl_state.normal_array_pointer;
		break;
	case GL_TEXTURE_COORD_ARRAY_POINTER:
		*params = (void *)gl_state.texcoord_array_pointer;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GLboolean glIsBuffer(GLuint buffer)
{
	for (GLint i = 0; i < gl_state.buffer_count; ++i) {
		if (gl_state.buffers[i]->id == buffer)
			return GL_TRUE;
	}
	return GL_FALSE;
}

GLboolean glIsTexture(GLuint texture)
{
	return context_find_texture(texture) != NULL;
}
