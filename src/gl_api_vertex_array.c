#include "gl_context.h"
#include "gl_state_helpers.h"
#include "gl_errors.h"
#include <GLES/gl.h>

GL_API void GL_APIENTRY glEnableClientState(GLenum array)
{
	switch (array) {
	case GL_VERTEX_ARRAY:
	case GL_COLOR_ARRAY:
	case GL_NORMAL_ARRAY:
	case GL_TEXTURE_COORD_ARRAY:
	case GL_POINT_SIZE_ARRAY_OES:
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}
	RenderContext *ctx = GetCurrentContext();
	switch (array) {
	case GL_VERTEX_ARRAY:
		SET_BOOL(ctx->vertex_array.enabled, GL_TRUE,
			 ctx->vertex_array.version);
		break;
	case GL_COLOR_ARRAY:
		SET_BOOL(ctx->color_array.enabled, GL_TRUE,
			 ctx->color_array.version);
		break;
	case GL_NORMAL_ARRAY:
		SET_BOOL(ctx->normal_array.enabled, GL_TRUE,
			 ctx->normal_array.version);
		break;
	case GL_TEXTURE_COORD_ARRAY:
		SET_BOOL(ctx->texcoord_array.enabled, GL_TRUE,
			 ctx->texcoord_array.version);
		break;
	case GL_POINT_SIZE_ARRAY_OES:
		ctx->point_size_array_enabled = GL_TRUE;
		break;
	default:
		break;
	}
}

GL_API void GL_APIENTRY glDisableClientState(GLenum array)
{
	switch (array) {
	case GL_VERTEX_ARRAY:
	case GL_COLOR_ARRAY:
	case GL_NORMAL_ARRAY:
	case GL_TEXTURE_COORD_ARRAY:
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}
	RenderContext *ctx = GetCurrentContext();
	switch (array) {
	case GL_VERTEX_ARRAY:
		SET_BOOL(ctx->vertex_array.enabled, GL_FALSE,
			 ctx->vertex_array.version);
		break;
	case GL_COLOR_ARRAY:
		SET_BOOL(ctx->color_array.enabled, GL_FALSE,
			 ctx->color_array.version);
		break;
	case GL_NORMAL_ARRAY:
		SET_BOOL(ctx->normal_array.enabled, GL_FALSE,
			 ctx->normal_array.version);
		break;
	case GL_TEXTURE_COORD_ARRAY:
		SET_BOOL(ctx->texcoord_array.enabled, GL_FALSE,
			 ctx->texcoord_array.version);
		break;
	case GL_POINT_SIZE_ARRAY_OES:
		ctx->point_size_array_enabled = GL_FALSE;
		break;
	default:
		break;
	}
}

GL_API void GL_APIENTRY glClientActiveTexture(GLenum texture)
{
	RenderContext *ctx = GetCurrentContext();
	ctx->client_active_texture = texture;
}

GL_API void GL_APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride,
					const void *ptr)
{
	RenderContext *ctx = GetCurrentContext();
	ctx->vertex_array.size = size;
	ctx->vertex_array.type = type;
	ctx->vertex_array.stride = stride;
	ctx->vertex_array.pointer = ptr;
	atomic_fetch_add_explicit(&ctx->vertex_array.version, 1,
				  memory_order_relaxed);
}

GL_API void GL_APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride,
				       const void *ptr)
{
	RenderContext *ctx = GetCurrentContext();
	ctx->color_array.size = size;
	ctx->color_array.type = type;
	ctx->color_array.stride = stride;
	ctx->color_array.pointer = ptr;
	atomic_fetch_add_explicit(&ctx->color_array.version, 1,
				  memory_order_relaxed);
}

GL_API void GL_APIENTRY glNormalPointer(GLenum type, GLsizei stride,
					const void *ptr)
{
	RenderContext *ctx = GetCurrentContext();
	ctx->normal_array.type = type;
	ctx->normal_array.stride = stride;
	ctx->normal_array.pointer = ptr;
	atomic_fetch_add_explicit(&ctx->normal_array.version, 1,
				  memory_order_relaxed);
}

GL_API void GL_APIENTRY glTexCoordPointer(GLint size, GLenum type,
					  GLsizei stride, const void *ptr)
{
	RenderContext *ctx = GetCurrentContext();
	ctx->texcoord_array.size = size;
	ctx->texcoord_array.type = type;
	ctx->texcoord_array.stride = stride;
	ctx->texcoord_array.pointer = ptr;
	atomic_fetch_add_explicit(&ctx->texcoord_array.version, 1,
				  memory_order_relaxed);
}
