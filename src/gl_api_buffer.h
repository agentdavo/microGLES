#ifndef GL_API_BUFFER_H
#define GL_API_BUFFER_H
#include <GLES/gl.h>

GL_API void GL_APIENTRY glBindBuffer(GLenum target, GLuint buffer);
GL_API void GL_APIENTRY glGenBuffers(GLsizei n, GLuint *buffers);
GL_API void GL_APIENTRY glDeleteBuffers(GLsizei n, const GLuint *buffers);
GL_API void GL_APIENTRY glBufferData(GLenum target, GLsizeiptr size,
				     const void *data, GLenum usage);
GL_API void GL_APIENTRY glBufferSubData(GLenum target, GLintptr offset,
					GLsizeiptr size, const void *data);

#endif /* GL_API_BUFFER_H */
