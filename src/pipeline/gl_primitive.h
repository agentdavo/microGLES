#ifndef GL_PRIMITIVE_H
#define GL_PRIMITIVE_H

#include "../gl_types.h"
#include "gl_framebuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

void pipeline_assemble_triangle(Triangle *dst, const Vertex *v0,
				const Vertex *v1, const Vertex *v2);

typedef struct {
	Vertex verts[3];
	Framebuffer *fb;
	GLint viewport[4];
} PrimitiveJob;

void process_primitive_job(void *task_data);

#ifdef __cplusplus
}
#endif

#endif /* GL_PRIMITIVE_H */
