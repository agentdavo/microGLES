#ifndef GL_TYPES_H
#define GL_TYPES_H
/**
 * @file gl_types.h
 * @brief Core data structures shared across modules.
 */

#include <GLES/gl.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLfloat x, y, z, w;
	GLfloat normal[3];
	GLfloat color[4];
	GLfloat texcoord[4];
	GLfloat point_size;
} Vertex;

typedef struct {
	Vertex v0, v1, v2;
} Triangle;

typedef struct {
	uint32_t x;
	uint32_t y;
	GLuint color;
	float depth;
} Fragment;

typedef struct {
	GLenum func;
	GLint ref;
	GLuint mask;
	GLenum sfail;
	GLenum zfail;
	GLenum zpass;
	GLuint writemask;
	atomic_uint version;
} StencilState;

typedef struct {
	GLboolean enabled;
	GLenum mode;
	GLfloat density;
	GLfloat start;
	GLfloat end;
	GLfloat color[4];
	atomic_uint version;
} FogState;

#define MAX_MIPMAP_LEVELS 12

typedef struct TextureOES {
	GLuint id;
	GLenum target;
	GLenum internalformat;
	GLenum format; /* original user format */
	GLsizei width;
	GLsizei height;
	GLsizei mip_width[MAX_MIPMAP_LEVELS];
	GLsizei mip_height[MAX_MIPMAP_LEVELS];
	uint32_t *levels[MAX_MIPMAP_LEVELS];
	GLboolean mipmap_supported;
	GLint current_level;
	GLint wrap_s;
	GLint wrap_t;
	GLint min_filter;
	GLint mag_filter;
	GLint crop_rect[4];
	GLint required_units;
	atomic_uint version;
	atomic_bool active;
} TextureOES;

#ifdef __cplusplus
}
#endif

#endif /* GL_TYPES_H */
