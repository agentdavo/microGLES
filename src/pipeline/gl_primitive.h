#ifndef GL_PRIMITIVE_H
#define GL_PRIMITIVE_H

#include "../gl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void pipeline_assemble_triangle(Triangle *dst, const Vertex *v0,
				const Vertex *v1, const Vertex *v2);

#ifdef __cplusplus
}
#endif

#endif /* GL_PRIMITIVE_H */
