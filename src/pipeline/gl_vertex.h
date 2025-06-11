#ifndef GL_VERTEX_H
#define GL_VERTEX_H

#include "../gl_types.h"
#include "../matrix_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

void pipeline_transform_vertex(Vertex *dst, const Vertex *src, const mat4 *mvp);

#ifdef __cplusplus
}
#endif

#endif /* GL_VERTEX_H */
