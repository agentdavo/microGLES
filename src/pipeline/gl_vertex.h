#ifndef GL_VERTEX_H
#define GL_VERTEX_H

#include "../gl_types.h"
#include "../matrix_utils.h"
#include "gl_framebuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	Vertex in[3];
	Framebuffer *fb;
} VertexJob;

void pipeline_transform_vertex(Vertex *dst, const Vertex *src, const mat4 *mvp,
			       const mat4 *normal_mat);
void process_vertex_job(void *task_data);

#ifdef __cplusplus
}
#endif

#endif /* GL_VERTEX_H */
