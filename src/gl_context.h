#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
/**
 * @file gl_context.h
 * @brief Global rendering context definition.
 */

#include "matrix_utils.h"
#include <GLES/gl.h>
#include <stdatomic.h>
#include "portable/c11threads.h"
#include "gl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLenum env_mode;
	GLfloat env_color[4];
	atomic_uint version;
	GLuint bound_texture;
	GLint wrap_s;
	GLint wrap_t;
	GLint min_filter;
	GLint mag_filter;
} TextureState;

typedef struct {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat position[4];
	GLfloat spot_direction[3];
	GLfloat spot_exponent;
	GLfloat spot_cutoff;
	GLfloat constant_attenuation;
	GLfloat linear_attenuation;
	GLfloat quadratic_attenuation;
	GLboolean enabled;
	atomic_uint version;
} LightState;

typedef struct {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat emission[4];
	GLfloat shininess;
	atomic_uint version;
} MaterialState;

typedef struct {
	GLenum src_factor;
	GLenum dst_factor;
	atomic_uint version;
} BlendState;

typedef struct {
	GLboolean enabled;
	GLint size;
	GLenum type;
	GLsizei stride;
	const void *pointer;
	atomic_uint version;
} ArrayState;

typedef struct {
	GLboolean enabled;
	GLenum func;
	GLfloat ref;
	atomic_uint version;
} AlphaTestState;

#define MAX_TEXTURES 1024

typedef struct TextureOES TextureOES;

typedef struct {
	mat4 modelview_matrix;
	mat4 projection_matrix;
	mat4 texture_matrix;
	GLint modelview_stack_depth;
	GLint projection_stack_depth;
	GLint texture_stack_depth;
	GLfloat current_color[4];
	GLfloat clear_color[4];
	GLboolean depth_test_enabled;
	GLenum depth_func;
	atomic_uint version_depth;
	GLfloat current_normal[3];
	GLfloat current_texcoord[8][4];
	GLboolean stencil_test_enabled;
	GLint clear_stencil;
	StencilState stencil;
	atomic_uint version_modelview;
	atomic_uint version_projection;
	atomic_uint version_texture;
	TextureState texture_env[2];
	TextureOES *textures[MAX_TEXTURES];
	GLuint texture_count;
	GLuint next_texture_id;
	GLenum active_texture;
	GLuint bound_texture_external;
	GLenum client_active_texture;
	BlendState blend;
	GLboolean blend_enabled;
	AlphaTestState alpha_test;
	LightState lights[8];
	GLboolean texture_2d_enabled;
	atomic_uint version_tex_enable;
	GLboolean cull_face_enabled;
	GLenum cull_face_mode;
	GLenum front_face;
	atomic_uint version_cull;
	GLboolean scissor_test_enabled;
	GLint scissor_box[4];
	atomic_uint version_scissor;
	GLboolean color_mask[4];
	GLboolean depth_mask;
	atomic_uint version_mask;
	ArrayState vertex_array;
	ArrayState color_array;
	ArrayState normal_array;
	ArrayState texcoord_array;
	GLboolean point_size_array_enabled;
	GLenum point_size_array_type;
	GLsizei point_size_array_stride;
	const void *point_size_array_pointer;
	MaterialState material;
	FogState fog;
	GLboolean color_logic_op_enabled;
	atomic_uint version_color_logic_op;
	GLboolean color_material_enabled;
	atomic_uint version_color_material;
	GLboolean dither_enabled;
	atomic_uint version_dither;
	GLboolean lighting_enabled;
	atomic_uint version_lighting;
	GLboolean line_smooth_enabled;
	atomic_uint version_line_smooth;
	GLboolean multisample_enabled;
	atomic_uint version_multisample;
	GLboolean normalize_enabled;
	atomic_uint version_normalize;
	GLboolean point_smooth_enabled;
	atomic_uint version_point_smooth;
	GLboolean point_sprite_enabled;
	atomic_uint version_point_sprite;
	GLboolean polygon_offset_fill_enabled;
	atomic_uint version_polygon_offset_fill;
	GLboolean rescale_normal_enabled;
	atomic_uint version_rescale_normal;
	GLboolean sample_alpha_to_coverage_enabled;
	atomic_uint version_sample_alpha_to_coverage;
	GLboolean sample_alpha_to_one_enabled;
	atomic_uint version_sample_alpha_to_one;
	GLboolean sample_coverage_enabled;
	atomic_uint version_sample_coverage;
	GLboolean clip_plane_enabled[6];
	atomic_uint version_clip_plane;

	unsigned validated_blend_version;
	unsigned validated_depth_version;
	unsigned validated_fog_version;
	unsigned validated_cull_version;
} RenderContext;

void context_init(void);
RenderContext *context_get(void);
RenderContext *GetCurrentContext(void);
void context_update_modelview_matrix(const mat4 *mat);
void context_update_projection_matrix(const mat4 *mat);
void context_update_texture_matrix(const mat4 *mat);
void context_set_clear_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void context_set_texture_env(GLenum unit, GLenum pname, const GLfloat *params);
void context_bind_texture(GLenum unit, GLenum target, GLuint texture);
void context_active_texture(GLenum unit);
void context_gen_textures(GLsizei n, GLuint *textures);
void context_delete_textures(GLsizei n, const GLuint *textures);
void context_tex_image_2d(GLenum target, GLint level, GLint internalformat,
			  GLsizei width, GLsizei height, GLenum format,
			  GLenum type, const void *pixels);
void context_tex_sub_image_2d(GLenum target, GLint level, GLint xoffset,
			      GLint yoffset, GLsizei width, GLsizei height,
			      GLenum format, GLenum type, const void *pixels);
void context_tex_parameterf(GLenum target, GLenum pname, GLfloat param);
TextureOES *context_find_texture(GLuint id);
void context_set_blend_func(GLenum sfactor, GLenum dfactor);
void context_set_alpha_func(GLenum func, GLfloat ref);
void context_set_depth_func(GLenum func);
void context_set_light(GLenum light, GLenum pname, const GLfloat *params);
void context_set_material(GLenum pname, const GLfloat *params);
void context_set_fog(GLenum pname, const GLfloat *params);
void context_set_stencil_func(GLenum func, GLint ref, GLuint mask);
void context_set_stencil_op(GLenum sfail, GLenum zfail, GLenum zpass);
void context_set_stencil_mask(GLuint mask);
void context_set_clear_stencil(GLint s);
void context_set_error(GLenum error);
GLenum context_get_error(void);

void log_state_change(const char *msg);

#ifdef __cplusplus
}
#endif

#endif // GL_CONTEXT_H
