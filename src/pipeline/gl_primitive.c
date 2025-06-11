#include "gl_primitive.h"

void pipeline_assemble_triangle(Triangle *dst, const Vertex *v0,
				const Vertex *v1, const Vertex *v2)
{
	*dst = (Triangle){ *v0, *v1, *v2 };
}
