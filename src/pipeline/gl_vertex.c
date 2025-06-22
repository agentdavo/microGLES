#include "gl_vertex.h"
#include "gl_primitive.h"
#include "../gl_context.h"
#define PIPELINE_USE_GLSTATE 0
_Static_assert(PIPELINE_USE_GLSTATE == 0, "pipeline must not touch gl_state");
#include "../gl_memory_tracker.h"
#include "../gl_thread.h"
#include "../pool.h"
#include <string.h>
#include "../c11_opt.h"
#include "../matrix_utils.h"

/*
 * Vertices flow through the pipeline in several coordinate spaces:
 * - object space: incoming attribute values
 * - eye space:   after modelview transform
 * - clip space:  after projection
 * - window space: after viewport scaling in the raster stage
 * This file performs object->clip transformations and lighting.
 */

void pipeline_transform_vertex(Vertex *restrict dst, const Vertex *restrict src,
			       const mat4 *restrict mvp,
			       const mat4 *restrict normal_mat,
			       const GLint *restrict viewport)
{
	GLfloat in[4] = { src->x, src->y, src->z, src->w };
	GLfloat out[4];
	mat4_transform_vec4(mvp, in, out);
	GLfloat inv_w = out[3] != 0.0f ? 1.0f / out[3] : 1.0f;
	GLfloat ndc_x = out[0] * inv_w;
	GLfloat ndc_y = out[1] * inv_w;
	GLfloat ndc_z = out[2] * inv_w;
	dst->x = viewport[0] + (ndc_x * 0.5f + 0.5f) * viewport[2];
	dst->y = viewport[1] + (1.0f - (ndc_y * 0.5f + 0.5f)) * viewport[3];
	dst->z = ndc_z;
	dst->w = out[3];
	for (int i = 0; i < 3; ++i)
		dst->normal[i] = src->normal[i];
	if (normal_mat) {
		GLfloat nin[4] = { src->normal[0], src->normal[1],
				   src->normal[2], 0.0f };
		GLfloat nout[4];
		mat4_transform_vec4(normal_mat, nin, nout);
		dst->normal[0] = nout[0];
		dst->normal[1] = nout[1];
		dst->normal[2] = nout[2];
	}
	for (int i = 0; i < 4; ++i)
		dst->color[i] = src->color[i];
	for (int i = 0; i < 4; ++i)
		dst->texcoord[i] = src->texcoord[i];
	dst->point_size = src->point_size;
}

static _Thread_local mat4 tl_mvp;
static _Thread_local mat4 tl_normal;
static _Thread_local unsigned seen_mv, seen_proj;
static _Thread_local unsigned seen_normal;
static _Thread_local LightState tl_lights[8];
static _Thread_local unsigned seen_light[8];
static _Thread_local MaterialState tl_mat;
static _Thread_local unsigned seen_mat;

static void apply_lighting(Vertex *v)
{
	RenderContext *ctx = GetCurrentContext();
	for (int i = 0; i < 8; ++i) {
		unsigned lv = atomic_load(&ctx->lights[i].version);
		if (lv != seen_light[i]) {
			memcpy(&tl_lights[i], &ctx->lights[i],
			       sizeof(LightState));
			seen_light[i] = lv;
		}
	}
	unsigned mv = atomic_load(&ctx->material.version);
	if (mv != seen_mat) {
		memcpy(&tl_mat, &ctx->material, sizeof(MaterialState));
		seen_mat = mv;
	}
	float r = tl_mat.emission[0];
	float g = tl_mat.emission[1];
	float b = tl_mat.emission[2];
	float nx = v->normal[0];
	float ny = v->normal[1];
	float nz = v->normal[2];
	vec3_normalize(&nx, &ny, &nz);
	for (int li = 0; li < 8; ++li) {
		LightState *lt = &tl_lights[li];
		if (!lt->enabled)
			continue;
		float lx = -lt->position[0];
		float ly = -lt->position[1];
		float lz = -lt->position[2];
		float dist = GL_SQRT(lx * lx + ly * ly + lz * lz);
		if (dist > 0.0f) {
			lx /= dist;
			ly /= dist;
			lz /= dist;
		}
		float att = 1.0f / (lt->constant_attenuation +
				    lt->linear_attenuation * dist +
				    lt->quadratic_attenuation * dist * dist);
		float dot = nx * lx + ny * ly + nz * lz;
		if (dot < 0.0f)
			dot = 0.0f;
		float hx = lx;
		float hy = ly;
		float hz = lz + 1.0f;
		float spec = 0.0f;
		if (hx != 0.0f || hy != 0.0f || hz != 0.0f) {
			vec3_normalize(&hx, &hy, &hz);
			float spec_dot = nx * hx + ny * hy + nz * hz;
			if (spec_dot < 0.0f)
				spec_dot = 0.0f;
			spec = GL_POW(spec_dot, tl_mat.shininess);
		}
		r += tl_mat.ambient[0] * lt->ambient[0] * att +
		     tl_mat.diffuse[0] * lt->diffuse[0] * dot * att +
		     tl_mat.specular[0] * lt->specular[0] * spec * att;
		g += tl_mat.ambient[1] * lt->ambient[1] * att +
		     tl_mat.diffuse[1] * lt->diffuse[1] * dot * att +
		     tl_mat.specular[1] * lt->specular[1] * spec * att;
		b += tl_mat.ambient[2] * lt->ambient[2] * att +
		     tl_mat.diffuse[2] * lt->diffuse[2] * dot * att +
		     tl_mat.specular[2] * lt->specular[2] * spec * att;
	}
	v->color[0] = r;
	v->color[1] = g;
	v->color[2] = b;
	v->color[3] = tl_mat.diffuse[3];
}

void process_vertex_job(void *task_data)
{
	VertexJob *job = (VertexJob *)task_data;
	RenderContext *ctx = GetCurrentContext();
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
	if (mv != seen_normal) {
		tl_normal = ctx->modelview_matrix;
		if (!mat4_inverse(&tl_normal))
			mat4_identity(&tl_normal);
		mat4_transpose(&tl_normal);
		seen_normal = mv;
	}
	Vertex v0, v1, v2;
	pipeline_transform_vertex(&v0, &job->in[0], &tl_mvp, &tl_normal,
				  job->viewport);
	pipeline_transform_vertex(&v1, &job->in[1], &tl_mvp, &tl_normal,
				  job->viewport);
	pipeline_transform_vertex(&v2, &job->in[2], &tl_mvp, &tl_normal,
				  job->viewport);
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
	memcpy(pjob->viewport, job->viewport, sizeof(job->viewport));
	vertex_job_release(job);
	thread_pool_submit(process_primitive_job, pjob, STAGE_PRIMITIVE);
}
