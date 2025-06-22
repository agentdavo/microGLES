#include "gl_state.h"
#include "gl_context.h"
#include "gl_state_helpers.h"
#include "gl_errors.h"
#include "gl_thread.h"
#include "gl_utils.h"
#include "function_profile.h"
#include <GLES/gl.h>
#include <string.h>

GL_API void GL_APIENTRY glEnable(GLenum cap)
{
	PROFILE_START("glEnable");
	switch (cap) {
	case GL_ALPHA_TEST:
		GetCurrentContext()->alpha_test.enabled = GL_TRUE;
		break;
	case GL_BLEND:
		SET_BOOL_AND_BUMP(GetCurrentContext(), blend_enabled, GL_TRUE,
				  GetCurrentContext()->blend.version);
		break;
	case GL_COLOR_LOGIC_OP:
		SET_BOOL_AND_BUMP(GetCurrentContext(), color_logic_op_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_color_logic_op);
		break;
	case GL_COLOR_MATERIAL:
		SET_BOOL_AND_BUMP(GetCurrentContext(), color_material_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_color_material);
		break;
	case GL_CULL_FACE:
		SET_BOOL_AND_BUMP(GetCurrentContext(), cull_face_enabled,
				  GL_TRUE, GetCurrentContext()->version_cull);
		break;
	case GL_DEPTH_TEST:
		SET_BOOL_AND_BUMP(GetCurrentContext(), depth_test_enabled,
				  GL_TRUE, GetCurrentContext()->version_depth);
		break;
	case GL_DITHER:
		SET_BOOL_AND_BUMP(GetCurrentContext(), dither_enabled, GL_TRUE,
				  GetCurrentContext()->version_dither);
		break;
	case GL_FOG:
		SET_BOOL_AND_BUMP(GetCurrentContext(), fog.enabled, GL_TRUE,
				  GetCurrentContext()->fog.version);
		break;
	case GL_LIGHTING:
		SET_BOOL_AND_BUMP(GetCurrentContext(), lighting_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_lighting);
		break;
	case GL_LINE_SMOOTH:

		SET_BOOL_AND_BUMP(GetCurrentContext(), line_smooth_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_line_smooth);
		break;
	case GL_MULTISAMPLE:

		SET_BOOL_AND_BUMP(GetCurrentContext(), multisample_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_multisample);
		break;
	case GL_NORMALIZE:

		SET_BOOL_AND_BUMP(GetCurrentContext(), normalize_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_normalize);
		break;
	case GL_POINT_SMOOTH:

		SET_BOOL_AND_BUMP(GetCurrentContext(), point_smooth_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_point_smooth);
		break;
	case GL_POINT_SPRITE_OES:

		SET_BOOL_AND_BUMP(GetCurrentContext(), point_sprite_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_point_sprite);
		break;
	case GL_POLYGON_OFFSET_FILL:

		SET_BOOL_AND_BUMP(
			GetCurrentContext(), polygon_offset_fill_enabled,
			GL_TRUE,
			GetCurrentContext()->version_polygon_offset_fill);
		break;
	case GL_RESCALE_NORMAL:

		SET_BOOL_AND_BUMP(GetCurrentContext(), rescale_normal_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_rescale_normal);
		break;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:

		SET_BOOL_AND_BUMP(
			GetCurrentContext(), sample_alpha_to_coverage_enabled,
			GL_TRUE,
			GetCurrentContext()->version_sample_alpha_to_coverage);
		break;
	case GL_SAMPLE_ALPHA_TO_ONE:

		SET_BOOL_AND_BUMP(
			GetCurrentContext(), sample_alpha_to_one_enabled,
			GL_TRUE,
			GetCurrentContext()->version_sample_alpha_to_one);
		break;
	case GL_SAMPLE_COVERAGE:

		SET_BOOL_AND_BUMP(GetCurrentContext(), sample_coverage_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_sample_coverage);
		break;
	case GL_SCISSOR_TEST:
		SET_BOOL_AND_BUMP(GetCurrentContext(), scissor_test_enabled,
				  GL_TRUE,
				  GetCurrentContext()->version_scissor);
		break;
	case GL_STENCIL_TEST:
		GetCurrentContext()->stencil_test_enabled = GL_TRUE;
		break;
	case GL_TEXTURE_2D:
		SET_BOOL(GetCurrentContext()->texture_2d_enabled, GL_TRUE,
			 GetCurrentContext()->version_tex_enable);
		break;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:

		SET_BOOL_AND_BUMP(GetCurrentContext(),
				  clip_plane_enabled[cap - GL_CLIP_PLANE0],
				  GL_TRUE,
				  GetCurrentContext()->version_clip_plane);
		break;
	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		SET_BOOL(GetCurrentContext()->lights[cap - GL_LIGHT0].enabled,
			 GL_TRUE,
			 GetCurrentContext()->lights[cap - GL_LIGHT0].version);
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
	PROFILE_END("glEnable");
}

GL_API void GL_APIENTRY glDisable(GLenum cap)
{
	PROFILE_START("glDisable");
	switch (cap) {
	case GL_ALPHA_TEST:
		GetCurrentContext()->alpha_test.enabled = GL_FALSE;
		break;
	case GL_BLEND:
		SET_BOOL_AND_BUMP(GetCurrentContext(), blend_enabled, GL_FALSE,
				  GetCurrentContext()->blend.version);
		break;
	case GL_COLOR_LOGIC_OP:

		SET_BOOL_AND_BUMP(GetCurrentContext(), color_logic_op_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_color_logic_op);
		break;
	case GL_COLOR_MATERIAL:

		SET_BOOL_AND_BUMP(GetCurrentContext(), color_material_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_color_material);
		break;
	case GL_CULL_FACE:
		SET_BOOL_AND_BUMP(GetCurrentContext(), cull_face_enabled,
				  GL_FALSE, GetCurrentContext()->version_cull);
		break;
	case GL_DEPTH_TEST:
		SET_BOOL_AND_BUMP(GetCurrentContext(), depth_test_enabled,
				  GL_FALSE, GetCurrentContext()->version_depth);
		break;
	case GL_DITHER:
		SET_BOOL_AND_BUMP(GetCurrentContext(), dither_enabled, GL_FALSE,
				  GetCurrentContext()->version_dither);
		break;
	case GL_FOG:
		SET_BOOL_AND_BUMP(GetCurrentContext(), fog.enabled, GL_FALSE,
				  GetCurrentContext()->fog.version);
		break;
	case GL_LIGHTING:
		SET_BOOL_AND_BUMP(GetCurrentContext(), lighting_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_lighting);
		break;
	case GL_LINE_SMOOTH:

		SET_BOOL_AND_BUMP(GetCurrentContext(), line_smooth_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_line_smooth);
		break;
	case GL_MULTISAMPLE:

		SET_BOOL_AND_BUMP(GetCurrentContext(), multisample_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_multisample);
		break;
	case GL_NORMALIZE:

		SET_BOOL_AND_BUMP(GetCurrentContext(), normalize_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_normalize);
		break;
	case GL_POINT_SMOOTH:

		SET_BOOL_AND_BUMP(GetCurrentContext(), point_smooth_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_point_smooth);
		break;
	case GL_POINT_SPRITE_OES:

		SET_BOOL_AND_BUMP(GetCurrentContext(), point_sprite_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_point_sprite);
		break;
	case GL_POLYGON_OFFSET_FILL:

		SET_BOOL_AND_BUMP(
			GetCurrentContext(), polygon_offset_fill_enabled,
			GL_FALSE,
			GetCurrentContext()->version_polygon_offset_fill);
		break;
	case GL_RESCALE_NORMAL:

		SET_BOOL_AND_BUMP(GetCurrentContext(), rescale_normal_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_rescale_normal);
		break;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:

		SET_BOOL_AND_BUMP(
			GetCurrentContext(), sample_alpha_to_coverage_enabled,
			GL_FALSE,
			GetCurrentContext()->version_sample_alpha_to_coverage);
		break;
	case GL_SAMPLE_ALPHA_TO_ONE:

		SET_BOOL_AND_BUMP(
			GetCurrentContext(), sample_alpha_to_one_enabled,
			GL_FALSE,
			GetCurrentContext()->version_sample_alpha_to_one);
		break;
	case GL_SAMPLE_COVERAGE:

		SET_BOOL_AND_BUMP(GetCurrentContext(), sample_coverage_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_sample_coverage);
		break;
	case GL_SCISSOR_TEST:
		SET_BOOL_AND_BUMP(GetCurrentContext(), scissor_test_enabled,
				  GL_FALSE,
				  GetCurrentContext()->version_scissor);
		break;
	case GL_STENCIL_TEST:
		GetCurrentContext()->stencil_test_enabled = GL_FALSE;
		break;
	case GL_TEXTURE_2D:
		SET_BOOL(GetCurrentContext()->texture_2d_enabled, GL_FALSE,
			 GetCurrentContext()->version_tex_enable);
		break;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:

		SET_BOOL_AND_BUMP(GetCurrentContext(),
				  clip_plane_enabled[cap - GL_CLIP_PLANE0],
				  GL_FALSE,
				  GetCurrentContext()->version_clip_plane);
		break;
	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		SET_BOOL(GetCurrentContext()->lights[cap - GL_LIGHT0].enabled,
			 GL_FALSE,
			 GetCurrentContext()->lights[cap - GL_LIGHT0].version);
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
	PROFILE_END("glDisable");
}

GL_API GLboolean GL_APIENTRY glIsEnabled(GLenum cap)
{
	switch (cap) {
	case GL_ALPHA_TEST:
		return GetCurrentContext()->alpha_test.enabled;
	case GL_BLEND:
		return GetCurrentContext()->blend_enabled;
	case GL_COLOR_LOGIC_OP:
		return GetCurrentContext()->color_logic_op_enabled;
	case GL_COLOR_MATERIAL:
		return GetCurrentContext()->color_material_enabled;
	case GL_CULL_FACE:
		return GetCurrentContext()->cull_face_enabled;
	case GL_DEPTH_TEST:
		return GetCurrentContext()->depth_test_enabled;
	case GL_DITHER:
		return GetCurrentContext()->dither_enabled;
	case GL_FOG:
		return GetCurrentContext()->fog.enabled;
	case GL_LIGHTING:
		return GetCurrentContext()->lighting_enabled;
	case GL_LINE_SMOOTH:
		return GetCurrentContext()->line_smooth_enabled;
	case GL_MULTISAMPLE:
		return GetCurrentContext()->multisample_enabled;
	case GL_NORMALIZE:
		return GetCurrentContext()->normalize_enabled;
	case GL_POINT_SMOOTH:
		return GetCurrentContext()->point_smooth_enabled;
	case GL_POINT_SPRITE_OES:
		return GetCurrentContext()->point_sprite_enabled;
	case GL_POLYGON_OFFSET_FILL:
		return GetCurrentContext()->polygon_offset_fill_enabled;
	case GL_RESCALE_NORMAL:
		return GetCurrentContext()->rescale_normal_enabled;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		return GetCurrentContext()->sample_alpha_to_coverage_enabled;
	case GL_SAMPLE_ALPHA_TO_ONE:
		return GetCurrentContext()->sample_alpha_to_one_enabled;
	case GL_SAMPLE_COVERAGE:
		return GetCurrentContext()->sample_coverage_enabled;
	case GL_SCISSOR_TEST:
		return GetCurrentContext()->scissor_test_enabled;
	case GL_STENCIL_TEST:
		return GetCurrentContext()->stencil_test_enabled;
	case GL_TEXTURE_2D:
		return GetCurrentContext()->texture_2d_enabled;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:
		return GetCurrentContext()
			->clip_plane_enabled[cap - GL_CLIP_PLANE0];
	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		return GetCurrentContext()->lights[cap - GL_LIGHT0].enabled;
	default:
		glSetError(GL_INVALID_ENUM);
		return GL_FALSE;
	}
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
	RenderContext *ctx = GetCurrentContext();
	if (ctx->cull_face_mode != mode) {
		ctx->cull_face_mode = mode;
		atomic_fetch_add_explicit(&ctx->version_cull, 1,
					  memory_order_relaxed);
	}
}

GL_API void GL_APIENTRY glFrontFace(GLenum mode)
{
	gl_state.front_face = mode;
	RenderContext *ctx = GetCurrentContext();
	if (ctx->front_face != mode) {
		ctx->front_face = mode;
		atomic_fetch_add_explicit(&ctx->version_cull, 1,
					  memory_order_relaxed);
	}
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
#if defined(__GNUC__)
		if (((uintptr_t)data % (sizeof(GLboolean) * 4)) == 0) {
			data[0] = gl_state.color_mask[0];
			data[1] = gl_state.color_mask[1];
			data[2] = gl_state.color_mask[2];
			data[3] = gl_state.color_mask[3];
		} else
			return;
#else
		data[0] = gl_state.color_mask[0];
		data[1] = gl_state.color_mask[1];
		data[2] = gl_state.color_mask[2];
		data[3] = gl_state.color_mask[3];
#endif
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
	case GL_ALPHA_TEST_REF:
		data[0] = gl_state.alpha_ref;
		break;
	case GL_LINE_WIDTH:
		data[0] = gl_state.line_width;
		break;
	default:
		break;
	}
}

GL_API void GL_APIENTRY glGetFixedv(GLenum pname, GLfixed *params)
{
	if (!params)
		return;
	switch (pname) {
	case GL_COLOR_CLEAR_VALUE:
		params[0] = float_to_fixed(gl_state.clear_color[0]);
		params[1] = float_to_fixed(gl_state.clear_color[1]);
		params[2] = float_to_fixed(gl_state.clear_color[2]);
		params[3] = float_to_fixed(gl_state.clear_color[3]);
		break;
	case GL_DEPTH_CLEAR_VALUE:
		params[0] = float_to_fixed(gl_state.clear_depth);
		break;
	case GL_MODELVIEW_MATRIX:
		for (int i = 0; i < 4; ++i)
			params[i] = float_to_fixed(
				gl_state.modelview_matrix.data[i]);
		break;
	case GL_PROJECTION_MATRIX:
		for (int i = 0; i < 4; ++i)
			params[i] = float_to_fixed(
				gl_state.projection_matrix.data[i]);
		break;
	case GL_TEXTURE_MATRIX:
		for (int i = 0; i < 4; ++i)
			params[i] =
				float_to_fixed(gl_state.texture_matrix.data[i]);
		break;
	case GL_LIGHT_MODEL_AMBIENT:
		for (int i = 0; i < 4; ++i)
			params[i] =
				float_to_fixed(gl_state.light_model_ambient[i]);
		break;
	case GL_DEPTH_RANGE:
		params[0] = float_to_fixed(gl_state.depth_range_near);
		params[1] = float_to_fixed(gl_state.depth_range_far);
		break;
	case GL_SAMPLE_COVERAGE_VALUE:
		params[0] = float_to_fixed(gl_state.sample_coverage_value);
		break;
	case GL_ALPHA_TEST_REF:
		params[0] = float_to_fixed(gl_state.alpha_ref);
		break;
	case GL_LINE_WIDTH:
		params[0] = float_to_fixed(gl_state.line_width);
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
#if defined(__GNUC__)
		if (((uintptr_t)data % (sizeof(GLint) * 4)) == 0) {
			data[0] = gl_state.viewport[0];
			data[1] = gl_state.viewport[1];
			data[2] = gl_state.viewport[2];
			data[3] = gl_state.viewport[3];
		} else
			return;
#else
		data[0] = gl_state.viewport[0];
		data[1] = gl_state.viewport[1];
		data[2] = gl_state.viewport[2];
		data[3] = gl_state.viewport[3];
#endif
		break;
	case GL_SCISSOR_BOX:
		data[0] = gl_state.scissor_box[0];
		data[1] = gl_state.scissor_box[1];
		data[2] = gl_state.scissor_box[2];
		data[3] = gl_state.scissor_box[3];
		break;
	case GL_ACTIVE_TEXTURE:
		*data = GetCurrentContext()->active_texture;
		break;
	case GL_CLIENT_ACTIVE_TEXTURE:
		*data = GetCurrentContext()->client_active_texture;
		break;
	case GL_ARRAY_BUFFER_BINDING:
		*data = gl_state.array_buffer_binding;
		break;
	case GL_ELEMENT_ARRAY_BUFFER_BINDING:
		*data = gl_state.element_array_buffer_binding;
		break;
	case GL_TEXTURE_BINDING_2D: {
		RenderContext *ctx = GetCurrentContext();
		*data = ctx->texture_env[ctx->active_texture - GL_TEXTURE0]
				.bound_texture;
		break;
	}
	case GL_TEXTURE_BINDING_EXTERNAL_OES:
		*data = GetCurrentContext()->bound_texture_external;
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
	case GL_ALPHA_TEST_FUNC:
		*data = gl_state.alpha_func;
		break;
	case GL_ALPHA_TEST:
		*data = GetCurrentContext()->alpha_test.enabled ? 1 : 0;
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
	RenderContext *ctx = GetCurrentContext();
	switch (pname) {
	case GL_VERTEX_ARRAY_POINTER:
		*params = (void *)ctx->vertex_array.pointer;
		break;
	case GL_COLOR_ARRAY_POINTER:
		*params = (void *)ctx->color_array.pointer;
		break;
	case GL_NORMAL_ARRAY_POINTER:
		*params = (void *)ctx->normal_array.pointer;
		break;
	case GL_TEXTURE_COORD_ARRAY_POINTER:
		*params = (void *)ctx->texcoord_array.pointer;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API GLboolean GL_APIENTRY glIsBuffer(GLuint buffer)
{
	for (GLint i = 0; i < gl_state.buffer_count; ++i) {
		if (gl_state.buffers[i]->id == buffer)
			return GL_TRUE;
	}
	return GL_FALSE;
}

GL_API GLboolean GL_APIENTRY glIsTexture(GLuint texture)
{
	return context_find_texture(texture) != NULL;
}
