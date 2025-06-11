#ifndef GL_API_TEXTURE_H
#define GL_API_TEXTURE_H
#include <GLES/gl.h>

GL_API void GL_APIENTRY glActiveTexture(GLenum texture);
GL_API void GL_APIENTRY glBindTexture(GLenum target, GLuint texture);
GL_API void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures);
GL_API void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures);
GL_API void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname,
					GLfloat param);
GL_API void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname,
					 const GLfloat *params);
GL_API void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname,
					GLint param);
GL_API void GL_APIENTRY glTexImage2D(GLenum target, GLint level,
				     GLint internalformat, GLsizei width,
				     GLsizei height, GLint border,
				     GLenum format, GLenum type,
				     const void *pixels);
GL_API void GL_APIENTRY glTexSubImage2D(GLenum target, GLint level,
					GLint xoffset, GLint yoffset,
					GLsizei width, GLsizei height,
					GLenum format, GLenum type,
					const void *pixels);

#endif /* GL_API_TEXTURE_H */
