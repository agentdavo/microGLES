#ifndef GL_API_VERTEX_ARRAY_H
#define GL_API_VERTEX_ARRAY_H
/**
 * @file gl_api_vertex_array.h
 * @brief Vertex array management APIs.
 */
#include <GLES/gl.h>

GL_API void GL_APIENTRY glEnableClientState(GLenum array);
GL_API void GL_APIENTRY glDisableClientState(GLenum array);
GL_API void GL_APIENTRY glClientActiveTexture(GLenum texture);
GL_API void GL_APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride,
					const void *ptr);
GL_API void GL_APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride,
				       const void *ptr);
GL_API void GL_APIENTRY glNormalPointer(GLenum type, GLsizei stride,
					const void *ptr);
GL_API void GL_APIENTRY glTexCoordPointer(GLint size, GLenum type,
					  GLsizei stride, const void *ptr);

#endif /* GL_API_VERTEX_ARRAY_H */
