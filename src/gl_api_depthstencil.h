#ifndef GL_API_DEPTHSTENCIL_H
#define GL_API_DEPTHSTENCIL_H
#include <GLES/gl.h>

GL_API void GL_APIENTRY glClearDepthf(GLfloat d);
GL_API void GL_APIENTRY glDepthFunc(GLenum func);
GL_API void GL_APIENTRY glDepthMask(GLboolean flag);
GL_API void GL_APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask);
GL_API void GL_APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
GL_API void GL_APIENTRY glStencilMask(GLuint mask);
GL_API void GL_APIENTRY glClearStencil(GLint s);

#endif /* GL_API_DEPTHSTENCIL_H */
