/* gl_state.c */

#include "gl_state.h"
#include "gl_framebuffer_object.h" // For RenderbufferOES, FramebufferOES
#include "gl_types.h" // For TextureOES
#include "gl_utils.h" // For tracked_free
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include <GLES/gl.h> // For OpenGL ES types
#include <GLES/glext.h> // For extension types
#include <stddef.h> // For size_t
#include <stdlib.h> // For malloc and free

GLState gl_state; // Global GLState instance

void InitGLState(GLState *state)
{
	state->next_renderbuffer_id = 1;
	state->renderbuffer_count = 0;
	state->bound_renderbuffer = NULL;

	state->next_framebuffer_id = 1;
	state->framebuffer_count = 0;
	state->bound_framebuffer = &state->default_framebuffer;
	state->default_framebuffer.id = 0;

	state->texture_count = 0;
	state->next_texture_id = 1;

	state->next_buffer_id = 1;
	state->array_buffer_binding = 0;
	state->element_array_buffer_binding = 0;
	state->buffer_count = 0;

	state->next_vertex_array_id = 1;
	state->vao_count = 0;
	state->bound_vao = NULL;

	mat4_identity(&state->modelview_matrix);
	mat4_identity(&state->projection_matrix);
	mat4_identity(&state->texture_matrix);
	mat4_identity(&state->modelview_stack[0]);
	mat4_identity(&state->projection_stack[0]);
	mat4_identity(&state->texture_stack[0]);
	state->modelview_stack_depth = 1;
	state->projection_stack_depth = 1;
	state->texture_stack_depth = 1;
	state->matrix_mode = GL_MODELVIEW;

	/* Initialize tracked state */
	state->clear_color[0] = 0.0f;
	state->clear_color[1] = 0.0f;
	state->clear_color[2] = 0.0f;
	state->clear_color[3] = 1.0f;
	state->clear_depth = 1.0f;
	state->depth_range_near = 0.0f;
	state->depth_range_far = 1.0f;
	state->clear_stencil = 0;
	state->color_mask[0] = GL_TRUE;
	state->color_mask[1] = GL_TRUE;
	state->color_mask[2] = GL_TRUE;
	state->color_mask[3] = GL_TRUE;
	state->depth_mask = GL_TRUE;
	state->alpha_func = GL_ALWAYS;
	state->alpha_ref = 0.0f;
	state->blend_sfactor = GL_ONE;
	state->blend_dfactor = GL_ZERO;
	state->blend_sfactor_alpha = GL_ONE;
	state->blend_dfactor_alpha = GL_ZERO;
	state->blend_equation_rgb = GL_FUNC_ADD_OES;
	state->blend_equation_alpha = GL_FUNC_ADD_OES;
	state->depth_func = GL_LESS;
	state->cull_face_mode = GL_BACK;
	state->front_face = GL_CCW;
	state->current_color[0] = 1.0f;
	state->current_color[1] = 1.0f;
	state->current_color[2] = 1.0f;
	state->current_color[3] = 1.0f;
	state->current_normal[0] = 0.0f;
	state->current_normal[1] = 0.0f;
	state->current_normal[2] = 1.0f;
	state->active_texture = GL_TEXTURE0;
	state->client_active_texture = GL_TEXTURE0;
	state->bound_texture = 0;
	state->bound_texture_external = 0;
	state->viewport[0] = 0;
	state->viewport[1] = 0;
	state->viewport[2] = 0;
	state->viewport[3] = 0;
	state->point_size = 1.0f;
	state->line_width = 1.0f;
	state->polygon_offset_factor = 0.0f;
	state->polygon_offset_units = 0.0f;
	state->shade_model = GL_SMOOTH;
	state->scissor_box[0] = 0;
	state->scissor_box[1] = 0;
	state->scissor_box[2] = 0;
	state->scissor_box[3] = 0;
	state->pack_alignment = 4;
	state->unpack_alignment = 4;

	for (int i = 0; i < 6; ++i) {
		for (int j = 0; j < 4; ++j)
			state->clip_planes[i][j] = 0.0f;
	}

	state->light_model_ambient[0] = 0.0f;
	state->light_model_ambient[1] = 0.0f;
	state->light_model_ambient[2] = 0.0f;
	state->light_model_ambient[3] = 1.0f;
	state->light_model_two_side = GL_FALSE;

	state->logic_op_mode = GL_COPY;

	state->sample_coverage_value = 1.0f;
	state->sample_coverage_invert = GL_FALSE;

	state->stencil_func = GL_ALWAYS;
	state->stencil_ref = 0;
	state->stencil_value_mask = 0xFFFFFFFFu;
	state->stencil_fail = GL_KEEP;
	state->stencil_zfail = GL_KEEP;
	state->stencil_zpass = GL_KEEP;
	state->stencil_writemask = 0xFFFFFFFFu;

	state->point_fade_threshold_size = 1.0f;
	state->point_size_min = 0.0f;
	state->point_size_max = 1.0f;

	for (int i = 0; i < 8; ++i) {
		state->tex_env_mode[i] = GL_MODULATE;
		state->tex_env_color[i][0] = 0.0f;
		state->tex_env_color[i][1] = 0.0f;
		state->tex_env_color[i][2] = 0.0f;
		state->tex_env_color[i][3] = 0.0f;
		state->tex_env_combine_rgb[i] = GL_MODULATE;
		state->tex_env_combine_alpha[i] = GL_MODULATE;
		state->tex_env_src_rgb[i][0] = GL_TEXTURE;
		state->tex_env_src_rgb[i][1] = GL_PREVIOUS;
		state->tex_env_src_rgb[i][2] = GL_CONSTANT;
		state->tex_env_src_alpha[i][0] = GL_TEXTURE;
		state->tex_env_src_alpha[i][1] = GL_PREVIOUS;
		state->tex_env_src_alpha[i][2] = GL_CONSTANT;
		state->tex_env_operand_rgb[i][0] = GL_SRC_COLOR;
		state->tex_env_operand_rgb[i][1] = GL_SRC_COLOR;
		state->tex_env_operand_rgb[i][2] = GL_SRC_ALPHA;
		state->tex_env_operand_alpha[i][0] = GL_SRC_ALPHA;
		state->tex_env_operand_alpha[i][1] = GL_SRC_ALPHA;
		state->tex_env_operand_alpha[i][2] = GL_SRC_ALPHA;
		state->tex_env_rgb_scale[i] = 1.0f;
		state->tex_env_alpha_scale[i] = 1.0f;
		state->tex_env_coord_replace[i] = GL_FALSE;
		state->current_texcoord[i][0] = 0.0f;
		state->current_texcoord[i][1] = 0.0f;
		state->current_texcoord[i][2] = 0.0f;
		state->current_texcoord[i][3] = 1.0f;
	}
	for (int i = 0; i < 4; ++i) {
		state->tex_gen_mode[i] = 0;
		for (int j = 0; j < 4; ++j)
			state->tex_gen_plane[i][j] = (j == i) ? 1.0f : 0.0f;
	}
	state->fog_color[0] = 0.0f;
	state->fog_color[1] = 0.0f;
	state->fog_color[2] = 0.0f;
	state->fog_color[3] = 0.0f;
	state->fog_density = 1.0f;
	state->fog_start = 0.0f;
	state->fog_end = 1.0f;
	state->fog_mode = GL_EXP;
	state->fog_hint = GL_DONT_CARE;
	state->generate_mipmap_hint = GL_DONT_CARE;
	state->line_smooth_hint = GL_DONT_CARE;
	state->perspective_correction_hint = GL_DONT_CARE;
	state->point_smooth_hint = GL_DONT_CARE;

	/* Capability defaults */
	state->alpha_test_enabled = GL_FALSE;
	state->blend_enabled = GL_FALSE;
	state->color_logic_op_enabled = GL_FALSE;
	state->color_material_enabled = GL_FALSE;
	state->cull_face_enabled = GL_FALSE;
	state->depth_test_enabled = GL_FALSE;
	state->dither_enabled = GL_TRUE;
	state->fog_enabled = GL_FALSE;
	state->lighting_enabled = GL_FALSE;
	state->line_smooth_enabled = GL_FALSE;
	state->multisample_enabled = GL_TRUE;
	state->normalize_enabled = GL_FALSE;
	state->point_smooth_enabled = GL_FALSE;
	state->point_sprite_enabled = GL_FALSE;
	state->polygon_offset_fill_enabled = GL_FALSE;
	state->rescale_normal_enabled = GL_FALSE;
	state->sample_alpha_to_coverage_enabled = GL_FALSE;
	state->sample_alpha_to_one_enabled = GL_FALSE;
	state->sample_coverage_enabled = GL_FALSE;
	state->scissor_test_enabled = GL_FALSE;
	state->stencil_test_enabled = GL_FALSE;
	state->texture_2d_enabled = GL_FALSE;
	for (int i = 0; i < 6; ++i)
		state->clip_plane_enabled[i] = GL_FALSE;
	for (int i = 0; i < 8; ++i)
		state->light_enabled[i] = GL_FALSE;

	state->vertex_array_enabled = GL_FALSE;
	state->vertex_array_size = 4;
	state->vertex_array_type = GL_FLOAT;
	state->vertex_array_stride = 0;
	state->vertex_array_pointer = NULL;

	state->color_array_enabled = GL_FALSE;
	state->color_array_size = 4;
	state->color_array_type = GL_FLOAT;
	state->color_array_stride = 0;
	state->color_array_pointer = NULL;

	state->normal_array_enabled = GL_FALSE;
	state->normal_array_type = GL_FLOAT;
	state->normal_array_stride = 0;
	state->normal_array_pointer = NULL;

	state->texcoord_array_enabled = GL_FALSE;
	state->texcoord_array_size = 4;
	state->texcoord_array_type = GL_FLOAT;
	state->texcoord_array_stride = 0;
	state->texcoord_array_pointer = NULL;

	/* OES_point_size_array */
	state->point_size_array_type = GL_FLOAT;
	state->point_size_array_stride = 0;
	state->point_size_array_pointer = NULL;

	/* OES_matrix_palette */
	state->matrix_index_array_size = 0;
	state->matrix_index_array_type = GL_UNSIGNED_BYTE;
	state->matrix_index_array_stride = 0;
	state->matrix_index_array_pointer = NULL;
	state->weight_array_size = 0;
	state->weight_array_type = GL_FLOAT;
	state->weight_array_stride = 0;
	state->weight_array_pointer = NULL;
	state->current_palette_matrix = 0;
	for (int i = 0; i < 32; ++i)
		mat4_identity(&state->palette_matrices[i]);

	for (int i = 0; i < 8; ++i) {
		Light *lt = &state->lights[i];
		lt->ambient[0] = 0.0f;
		lt->ambient[1] = 0.0f;
		lt->ambient[2] = 0.0f;
		lt->ambient[3] = 1.0f;
		if (i == 0) {
			lt->diffuse[0] = 1.0f;
			lt->diffuse[1] = 1.0f;
			lt->diffuse[2] = 1.0f;
			lt->diffuse[3] = 1.0f;
			lt->specular[0] = 1.0f;
			lt->specular[1] = 1.0f;
			lt->specular[2] = 1.0f;
			lt->specular[3] = 1.0f;
		} else {
			lt->diffuse[0] = lt->diffuse[1] = lt->diffuse[2] =
				lt->diffuse[3] = 0.0f;
			lt->specular[0] = lt->specular[1] = lt->specular[2] =
				lt->specular[3] = 0.0f;
		}
		lt->position[0] = 0.0f;
		lt->position[1] = 0.0f;
		lt->position[2] = 1.0f;
		lt->position[3] = 0.0f;
		lt->spot_direction[0] = 0.0f;
		lt->spot_direction[1] = 0.0f;
		lt->spot_direction[2] = -1.0f;
		lt->spot_exponent = 0.0f;
		lt->spot_cutoff = 180.0f;
		lt->constant_attenuation = 1.0f;
		lt->linear_attenuation = 0.0f;
		lt->quadratic_attenuation = 0.0f;
	}

	for (int i = 0; i < 2; ++i) {
		Material *mat = &state->material[i];
		mat->ambient[0] = 0.2f;
		mat->ambient[1] = 0.2f;
		mat->ambient[2] = 0.2f;
		mat->ambient[3] = 1.0f;
		mat->diffuse[0] = 0.8f;
		mat->diffuse[1] = 0.8f;
		mat->diffuse[2] = 0.8f;
		mat->diffuse[3] = 1.0f;
		mat->specular[0] = 0.0f;
		mat->specular[1] = 0.0f;
		mat->specular[2] = 0.0f;
		mat->specular[3] = 1.0f;
		mat->emission[0] = 0.0f;
		mat->emission[1] = 0.0f;
		mat->emission[2] = 0.0f;
		mat->emission[3] = 1.0f;
		mat->shininess = 0.0f;
	}

	LOG_DEBUG("GLState initialized.");
}

void CleanupGLState(GLState *state)
{
	for (GLint i = 0; i < state->renderbuffer_count; ++i) {
		tracked_free(state->renderbuffers[i], sizeof(RenderbufferOES));
	}
	state->renderbuffer_count = 0;

	for (GLint i = 0; i < state->framebuffer_count; ++i) {
		tracked_free(state->framebuffers[i], sizeof(FramebufferOES));
	}
	state->framebuffer_count = 0;

	for (GLuint i = 0; i < state->texture_count; ++i) {
		tracked_free(state->textures[i], sizeof(TextureOES));
	}
	state->texture_count = 0;

	for (GLint i = 0; i < state->buffer_count; ++i) {
		tracked_free(state->buffers[i]->data, state->buffers[i]->size);
		tracked_free(state->buffers[i], sizeof(BufferObject));
	}
	state->buffer_count = 0;

	for (GLint i = 0; i < state->vao_count; ++i) {
		tracked_free(state->vaos[i], sizeof(VertexArrayObject));
	}
	state->vao_count = 0;

	LOG_DEBUG("GLState cleaned up.");
}