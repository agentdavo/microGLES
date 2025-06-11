#ifndef GL_TYPES_H
#define GL_TYPES_H

#include <GLES/gl.h>

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
	GLuint color;
	float depth;
} Fragment;

#ifdef __cplusplus
}
#endif

#endif /* GL_TYPES_H */
