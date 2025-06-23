#include "plugin.h"
#include "pipeline/gl_vertex.h"
#include "gl_context.h"
#include "matrix_utils.h"

static void vertex_shader_1_1(void *job)
{
	VertexJob *vj = (VertexJob *)job;
	if (!vj)
		return;

	RenderContext *ctx = GetCurrentContext();
	if (!ctx)
		return;

	mat4 mvp;
	mat4_multiply(&mvp, &ctx->projection_matrix, &ctx->modelview_matrix);

	for (int i = 0; i < 3; ++i) {
		Vertex *v = &vj->in[i];
		GLfloat in[4] = { v->x, v->y, v->z, v->w };
		GLfloat out[4];
		mat4_transform_vec4(&mvp, in, out);
		GLfloat inv_w = out[3] != 0.0f ? 1.0f / out[3] : 1.0f;
		GLfloat ndc_x = out[0] * inv_w;
		GLfloat ndc_y = out[1] * inv_w;
		GLfloat ndc_z = out[2] * inv_w;
		v->color[0] = ndc_x * 0.5f + 0.5f;
		v->color[1] = ndc_y * 0.5f + 0.5f;
		v->color[2] = ndc_z * 0.5f + 0.5f;
	}
}

PLUGIN_REGISTER(STAGE_VERTEX, vertex_shader_1_1)

int vertex_shader_1_1_link;
