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

void context_set_clear_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	g_render_context.clear_color[0] = r;
	g_render_context.clear_color[1] = g;
	g_render_context.clear_color[2] = b;
	g_render_context.clear_color[3] = a;
	log_state_change("clear color");
}

void context_set_error(GLenum error)
{
	if (thread_error == GL_NO_ERROR)
		thread_error = error;
}

GLenum context_get_error(void)
{
	GLenum e = thread_error;
	thread_error = GL_NO_ERROR;
	return e;
}

void log_state_change(const char *msg)
{
	LOG_DEBUG("State change: %s", msg);
}
