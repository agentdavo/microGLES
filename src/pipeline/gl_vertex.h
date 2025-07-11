#ifndef GL_VERTEX_H
#define GL_VERTEX_H

#include "../gl_types.h"
#include "../matrix_utils.h"
#include "gl_framebuffer.h"
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	alignas(64) Vertex in[3];
	Framebuffer *fb;
	GLint viewport[4];
} VertexJob;
_Static_assert(sizeof(VertexJob) == 256, "VertexJob size must be 256 bytes");
_Static_assert(alignof(VertexJob) >= 64, "VertexJob must be 64-byte aligned");

void pipeline_transform_vertex(Vertex *restrict dst, const Vertex *restrict src,
			       const mat4 *restrict mvp,
			       const mat4 *restrict normal_mat,
			       const GLint *restrict viewport);
void process_vertex_job(void *task_data);

#ifdef __cplusplus
}
#endif

#endif /* GL_VERTEX_H */
