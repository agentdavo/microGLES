#ifndef GL_TYPES_H
#define GL_TYPES_H

#include <GLES/gl.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLfloat x, y, z, w;
	GLfloat color[4];
	GLfloat texcoord[4];
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

#ifdef __cplusplus
}
#endif

#endif /* GL_TYPES_H */
