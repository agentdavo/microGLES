#ifndef GL_ERRORS_H
#define GL_ERRORS_H
/**
 * @file gl_errors.h
 * @brief Helpers for OpenGL ES error management.
 */

#include <GLES/gl.h>

/* Set the current OpenGL error. */
void glSetError(GLenum error);

/* Retrieve and clear the current OpenGL error. */
GLenum glGetError(void);

/* Query whether an error has been recorded. */
GLboolean hasGLError(void);

#endif // GL_ERRORS_H
