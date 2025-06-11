#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

#include "matrix_utils.h"
#include <GLES/gl.h>
#include <stdatomic.h>
#include <threads.h>
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
	GLfloat position[4];
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

#define MAX_TEXTURES 1024

typedef struct TextureOES TextureOES;

typedef struct {
	mat4 modelview_matrix;
	mat4 projection_matrix;
	mat4 texture_matrix;
	GLfloat current_color[4];
	GLfloat clear_color[4];
	GLboolean depth_test_enabled;
	GLenum depth_func;
	GLfloat current_normal[3];
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
	BlendState blend;
	LightState lights[1];
	MaterialState material;
	FogState fog;
} RenderContext;

void context_init(void);
RenderContext *context_get(void);
void context_update_modelview_matrix(const mat4 *mat);
void context_update_projection_matrix(const mat4 *mat);
void context_update_texture_matrix(const mat4 *mat);
void context_set_clear_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void context_set_texture_env(GLenum unit, GLenum pname, const GLfloat *params);
void context_bind_texture(GLenum unit, GLuint texture);
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
