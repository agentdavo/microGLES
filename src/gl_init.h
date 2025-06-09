#ifndef GL_INIT_H
#define GL_INIT_H

#include "gl_types.h"
#include "gl_state.h"
#include "gl_errors.h"

// Initializes the OpenGL ES context and sets up the rendering environment
void GL_init(void);

// Cleans up the OpenGL ES context (if needed)
void GL_cleanup(void);

// Sets the viewport and default projection matrix
void GL_setupViewport(GLint x, GLint y, GLsizei width, GLsizei height);

// Resets all OpenGL states to their default values
void GL_resetState(void);

// Default configuration for matrix stack and depth settings
void GL_defaultMatrixSetup(void);

#endif // GL_INIT_H
