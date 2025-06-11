#ifndef GL_API_PIXELS_H
#define GL_API_PIXELS_H
#include <GLES/gl.h>

GL_API void GL_APIENTRY glClear(GLbitfield mask);
GL_API void GL_APIENTRY glReadPixels(GLint x, GLint y, GLsizei width,
				     GLsizei height, GLenum format, GLenum type,
				     void *pixels);
GL_API void GL_APIENTRY glColorMask(GLboolean r, GLboolean g, GLboolean b,
				    GLboolean a);
GL_API void GL_APIENTRY glDepthRangef(GLfloat n, GLfloat f);

#endif /* GL_API_PIXELS_H */
