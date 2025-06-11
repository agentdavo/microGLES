#include "gl_context.h"
#include "gl_logger.h"
#include <string.h>

static RenderContext g_render_context;
static _Thread_local GLenum thread_error = GL_NO_ERROR;

static void init_defaults(RenderContext *ctx)
{
	mat4_identity(&ctx->modelview_matrix);
	mat4_identity(&ctx->projection_matrix);
	mat4_identity(&ctx->texture_matrix);
	ctx->current_color[0] = 1.0f;
	ctx->current_color[1] = 1.0f;
	ctx->current_color[2] = 1.0f;
	ctx->current_color[3] = 1.0f;
	ctx->clear_color[0] = 0.0f;
	ctx->clear_color[1] = 0.0f;
	ctx->clear_color[2] = 0.0f;
	ctx->clear_color[3] = 1.0f;
	ctx->depth_test_enabled = GL_TRUE;
	ctx->depth_func = GL_LESS;
	ctx->current_normal[0] = 0.0f;
	ctx->current_normal[1] = 0.0f;
	ctx->current_normal[2] = 1.0f;
	atomic_init(&ctx->version_modelview, 0);
	atomic_init(&ctx->version_projection, 0);
	atomic_init(&ctx->version_texture, 0);
	for (int i = 0; i < 2; ++i) {
		ctx->texture_env[i].env_mode = GL_MODULATE;
		ctx->texture_env[i].env_color[0] = 0.f;
		ctx->texture_env[i].env_color[1] = 0.f;
		ctx->texture_env[i].env_color[2] = 0.f;
		ctx->texture_env[i].env_color[3] = 0.f;
		atomic_init(&ctx->texture_env[i].version, 0);
	}
	ctx->blend.src_factor = GL_ONE;
	ctx->blend.dst_factor = GL_ZERO;
	atomic_init(&ctx->blend.version, 0);

	for (int i = 0; i < 1; ++i) {
		ctx->lights[i].ambient[0] = 0.2f;
		ctx->lights[i].ambient[1] = 0.2f;
		ctx->lights[i].ambient[2] = 0.2f;
		ctx->lights[i].ambient[3] = 1.0f;
		ctx->lights[i].diffuse[0] = 1.0f;
		ctx->lights[i].diffuse[1] = 1.0f;
		ctx->lights[i].diffuse[2] = 1.0f;
		ctx->lights[i].diffuse[3] = 1.0f;
		ctx->lights[i].position[0] = 0.0f;
		ctx->lights[i].position[1] = 0.0f;
		ctx->lights[i].position[2] = 1.0f;
		ctx->lights[i].position[3] = 0.0f; /* directional */
		ctx->lights[i].enabled = GL_FALSE;
		atomic_init(&ctx->lights[i].version, 0);
	}
	memcpy(ctx->material.ambient, ctx->current_color, sizeof(GLfloat) * 4);
	ctx->material.diffuse[0] = 1.0f;
	ctx->material.diffuse[1] = 1.0f;
	ctx->material.diffuse[2] = 1.0f;
	ctx->material.diffuse[3] = 1.0f;
	ctx->material.specular[0] = 0.0f;
	ctx->material.specular[1] = 0.0f;
	ctx->material.specular[2] = 0.0f;
	ctx->material.specular[3] = 1.0f;
	ctx->material.emission[0] = 0.0f;
	ctx->material.emission[1] = 0.0f;
	ctx->material.emission[2] = 0.0f;
	ctx->material.emission[3] = 1.0f;
	ctx->material.shininess = 0.0f;
	atomic_init(&ctx->material.version, 0);
}

void context_init(void)
{
	init_defaults(&g_render_context);
	thread_error = GL_NO_ERROR;
	LOG_INFO("Render context initialized");
}

RenderContext *context_get(void)
{
	return &g_render_context;
}

void context_update_modelview_matrix(const mat4 *mat)
{
	mat4_copy(&g_render_context.modelview_matrix, mat);
	atomic_fetch_add_explicit(&g_render_context.version_modelview, 1,
				  memory_order_relaxed);
	log_state_change("modelview updated");
}

void context_update_projection_matrix(const mat4 *mat)
{
	mat4_copy(&g_render_context.projection_matrix, mat);
	atomic_fetch_add_explicit(&g_render_context.version_projection, 1,
				  memory_order_relaxed);
	log_state_change("projection updated");
}

void context_update_texture_matrix(const mat4 *mat)
{
	mat4_copy(&g_render_context.texture_matrix, mat);
	atomic_fetch_add_explicit(&g_render_context.version_texture, 1,
				  memory_order_relaxed);
	log_state_change("texture matrix updated");
}

void context_set_clear_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	g_render_context.clear_color[0] = r;
	g_render_context.clear_color[1] = g;
	g_render_context.clear_color[2] = b;
	g_render_context.clear_color[3] = a;
	log_state_change("clear color");
}

void context_set_texture_env(GLenum unit, GLenum pname, const GLfloat *params)
{
	if (unit >= 2)
		return;
	TextureState *ts = &g_render_context.texture_env[unit];
	if (pname == GL_TEXTURE_ENV_MODE) {
		ts->env_mode = (GLenum)params[0];
	} else if (pname == GL_TEXTURE_ENV_COLOR) {
		memcpy(ts->env_color, params, sizeof(GLfloat) * 4);
	} else {
		return;
	}
	atomic_fetch_add_explicit(&ts->version, 1, memory_order_relaxed);
	log_state_change("texture env updated");
}

void context_set_blend_func(GLenum sfactor, GLenum dfactor)
{
	g_render_context.blend.src_factor = sfactor;
	g_render_context.blend.dst_factor = dfactor;
	atomic_fetch_add_explicit(&g_render_context.blend.version, 1,
				  memory_order_relaxed);
	log_state_change("blend func");
}

void context_set_light(GLenum light, GLenum pname, const GLfloat *params)
{
	if (light != GL_LIGHT0)
		return;
	LightState *ls = &g_render_context.lights[0];
	switch (pname) {
	case GL_AMBIENT:
		memcpy(ls->ambient, params, sizeof(GLfloat) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(ls->diffuse, params, sizeof(GLfloat) * 4);
		break;
	case GL_POSITION:
		memcpy(ls->position, params, sizeof(GLfloat) * 4);
		break;
	case GL_LIGHT_MODEL_TWO_SIDE:
		/* ignore for now */
		break;
	default:
		return;
	}
	ls->enabled = GL_TRUE;
	atomic_fetch_add_explicit(&ls->version, 1, memory_order_relaxed);
}

void context_set_material(GLenum pname, const GLfloat *params)
{
	MaterialState *ms = &g_render_context.material;
	switch (pname) {
	case GL_AMBIENT:
		memcpy(ms->ambient, params, sizeof(GLfloat) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(ms->diffuse, params, sizeof(GLfloat) * 4);
		break;
	case GL_SPECULAR:
		memcpy(ms->specular, params, sizeof(GLfloat) * 4);
		break;
	case GL_EMISSION:
		memcpy(ms->emission, params, sizeof(GLfloat) * 4);
		break;
	case GL_SHININESS:
		ms->shininess = params[0];
		break;
	default:
		return;
	}
	atomic_fetch_add_explicit(&ms->version, 1, memory_order_relaxed);
}

void context_set_error(GLenum error)
{
	if (thread_error == GL_NO_ERROR) {
		thread_error = error;
		LOG_DEBUG("context_set_error: Set error to 0x%X", error);
	} else {
		LOG_DEBUG(
			"context_set_error: Error already set to 0x%X, ignoring 0x%X",
			thread_error, error);
	}
}

GLenum context_get_error(void)
{
	GLenum e = thread_error;
	thread_error = GL_NO_ERROR;
	if (e != GL_NO_ERROR)
		LOG_DEBUG("context_get_error: returning 0x%X", e);
	return e;
}

void log_state_change(const char *msg)
{
	LOG_DEBUG("State change: %s", msg);
}
