#include "gl_vertex.h"
#include "gl_primitive.h"
#include "../gl_context.h"
#include "../gl_memory_tracker.h"
#include "../gl_thread.h"
#include <string.h>
#include <math.h>

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

static _Thread_local mat4 tl_mvp;
static _Thread_local unsigned seen_mv, seen_proj;
static _Thread_local LightState tl_light;
static _Thread_local unsigned seen_light;
static _Thread_local MaterialState tl_mat;
static _Thread_local unsigned seen_mat;

static void apply_lighting(Vertex *v)
{
	RenderContext *ctx = context_get();
	unsigned lv = atomic_load(&ctx->lights[0].version);
	if (lv != seen_light) {
		memcpy(&tl_light, &ctx->lights[0], sizeof(LightState));
		seen_light = lv;
	}
	unsigned mv = atomic_load(&ctx->material.version);
	if (mv != seen_mat) {
		memcpy(&tl_mat, &ctx->material, sizeof(MaterialState));
		seen_mat = mv;
	}
	if (!tl_light.enabled)
		return;
	float nx = ctx->current_normal[0];
	float ny = ctx->current_normal[1];
	float nz = ctx->current_normal[2];
	float lx = -tl_light.position[0];
	float ly = -tl_light.position[1];
	float lz = -tl_light.position[2];
	float len = sqrtf(lx * lx + ly * ly + lz * lz);
	if (len > 0.0f) {
		lx /= len;
		ly /= len;
		lz /= len;
	}
	float dot = nx * lx + ny * ly + nz * lz;
	if (dot < 0.0f)
		dot = 0.0f;
	for (int i = 0; i < 3; ++i) {
		float ambient = tl_mat.ambient[i] * tl_light.ambient[i];
		float diffuse = tl_mat.diffuse[i] * tl_light.diffuse[i] * dot;
		v->color[i] = ambient + diffuse;
	}
	v->color[3] = tl_mat.diffuse[3];
}

void process_vertex_job(void *task_data)
{
	VertexJob *job = (VertexJob *)task_data;
	RenderContext *ctx = context_get();
	unsigned mv = atomic_load(&ctx->version_modelview);
	unsigned pr = atomic_load(&ctx->version_projection);
	if (mv != seen_mv || pr != seen_proj) {
		mat4 mul;
		mat4_multiply(&mul, &ctx->projection_matrix,
			      &ctx->modelview_matrix);
		tl_mvp = mul;
		seen_mv = mv;
		seen_proj = pr;
	}
	Vertex v0, v1, v2;
	pipeline_transform_vertex(&v0, &job->in[0], &tl_mvp);
	pipeline_transform_vertex(&v1, &job->in[1], &tl_mvp);
	pipeline_transform_vertex(&v2, &job->in[2], &tl_mvp);
	apply_lighting(&v0);
	apply_lighting(&v1);
	apply_lighting(&v2);
	PrimitiveJob *pjob = MT_ALLOC(sizeof(PrimitiveJob), STAGE_PRIMITIVE);
	if (!pjob)
		return;
	pjob->verts[0] = v0;
	pjob->verts[1] = v1;
	pjob->verts[2] = v2;
	pjob->fb = job->fb;
	MT_FREE(job, STAGE_VERTEX);
	thread_pool_submit(process_primitive_job, pjob, STAGE_PRIMITIVE);
}
