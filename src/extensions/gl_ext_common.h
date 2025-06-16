#pragma once
#include <GLES/gl.h>
#include <string.h>
#include "../gl_state.h"
#include "../gl_context.h"
#include "../gl_types.h"
static inline BufferObject *find_buffer(GLuint id)
{
	for (GLint i = 0; i < gl_state.buffer_count; ++i) {
		if (gl_state.buffers[i]->id == id)
			return gl_state.buffers[i];
	}
	return NULL;
}
static inline TextureOES *find_texture(GLuint id)
{
	RenderContext *ctx = GetCurrentContext();
	for (GLuint i = 0; i < ctx->texture_count; ++i) {
		if (ctx->textures[i] && ctx->textures[i]->id == id)
			return ctx->textures[i];
	}
	return NULL;
}
static inline VertexArrayObject *find_vao(GLuint id)
{
	for (GLint i = 0; i < gl_state.vao_count; ++i) {
		if (gl_state.vaos[i]->id == id)
			return gl_state.vaos[i];
	}
	return NULL;
}
void ext_register(const char *flag);
#define EXT_REGISTER(flag)                                  \
	static void __attribute__((constructor)) _reg(void) \
	{                                                   \
		ext_register(flag);                         \
	}
const GLubyte *renderer_get_extensions(void);
