#ifndef GL_API_STATE_H
#define GL_API_STATE_H
/**
 * @file gl_api_state.h
 * @brief State manipulation functions for OpenGL ES.
 */
#include <GLES/gl.h>

GL_API void GL_APIENTRY glEnable(GLenum cap);
GL_API void GL_APIENTRY glDisable(GLenum cap);
GL_API GLboolean GL_APIENTRY glIsEnabled(GLenum cap);
GL_API void GL_APIENTRY glEnableClientState(GLenum array);
GL_API void GL_APIENTRY glDisableClientState(GLenum array);
GL_API void GL_APIENTRY glClientActiveTexture(GLenum texture);
GL_API void GL_APIENTRY glHint(GLenum target, GLenum mode);
GL_API void GL_APIENTRY glCullFace(GLenum mode);
GL_API void GL_APIENTRY glFrontFace(GLenum mode);
GL_API void GL_APIENTRY glShadeModel(GLenum mode);
GL_API void GL_APIENTRY glViewport(GLint x, GLint y, GLsizei width,
				   GLsizei height);
GL_API void GL_APIENTRY glScissor(GLint x, GLint y, GLsizei width,
				  GLsizei height);
GL_API void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean *data);
GL_API void GL_APIENTRY glGetFloatv(GLenum pname, GLfloat *data);
GL_API void GL_APIENTRY glGetFixedv(GLenum pname, GLfixed *data);
GL_API void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *data);
GL_API void GL_APIENTRY glGetPointerv(GLenum pname, void **params);
GL_API GLboolean GL_APIENTRY glIsBuffer(GLuint buffer);
GL_API GLboolean GL_APIENTRY glIsTexture(GLuint texture);

#endif /* GL_API_STATE_H */
