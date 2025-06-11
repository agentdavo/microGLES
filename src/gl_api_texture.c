#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include <GLES/gl.h>
#include <string.h>

GL_API void GL_APIENTRY glActiveTexture(GLenum texture)
{
	gl_state.active_texture = texture;
	context_active_texture(texture);
}

GL_API void GL_APIENTRY glBindTexture(GLenum target, GLuint texture)
{
	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_EXTERNAL_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (target == GL_TEXTURE_EXTERNAL_OES)
		gl_state.bound_texture_external = texture;
	else
		gl_state.bound_texture = texture;
	context_bind_texture(gl_state.active_texture - GL_TEXTURE0, texture);
}

GL_API void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
	if (n < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (!textures)
		return;
	context_gen_textures(n, textures);
}

GL_API void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
	if (n < 0 || !textures)
		return;
	context_delete_textures(n, textures);
}

GL_API void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname,
					GLfloat param)
{
	glTexParameteri(target, pname, (GLint)param);
}

GL_API void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname,
					 const GLfloat *params)
{
	glTexParameteri(target, pname, (GLint)params[0]);
}

GL_API void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname,
					GLint param)
{
	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_EXTERNAL_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	context_tex_parameter(target, pname, param);
}

GL_API void GL_APIENTRY glTexImage2D(GLenum target, GLint level,
				     GLint internalformat, GLsizei width,
				     GLsizei height, GLint border,
				     GLenum format, GLenum type,
				     const void *pixels)
{
	context_tex_image_2d(target, level, internalformat, width, height,
			     border, format, type, pixels);
}

GL_API void GL_APIENTRY glTexSubImage2D(GLenum target, GLint level,
					GLint xoffset, GLint yoffset,
					GLsizei width, GLsizei height,
					GLenum format, GLenum type,
					const void *pixels)
{
	context_tex_sub_image_2d(target, level, xoffset, yoffset, width, height,
				 format, type, pixels);
}
