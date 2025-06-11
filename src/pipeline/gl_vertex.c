#include "gl_vertex.h"

void pipeline_transform_vertex(Vertex *dst, const Vertex *src, const mat4 *mvp)
{
	GLfloat in[4] = { src->x, src->y, src->z, src->w };
	GLfloat out[4];
	mat4_transform_vec4(mvp, in, out);
	dst->x = out[0];
	dst->y = out[1];
	dst->z = out[2];
	dst->w = out[3];
	for (int i = 0; i < 4; ++i)
		dst->color[i] = src->color[i];
	for (int i = 0; i < 4; ++i)
		dst->texcoord[i] = src->texcoord[i];
}
