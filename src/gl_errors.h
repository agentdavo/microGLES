#ifndef GL_ERRORS_H
#define GL_ERRORS_H

#include <GLES/gl.h>

// Enum representing all the possible OpenGL ES errors
typedef enum {
  GL_NO_ERROR = 0,
  GL_INVALID_ENUM,
  GL_INVALID_VALUE,
  GL_INVALID_OPERATION,
  GL_STACK_OVERFLOW,
  GL_STACK_UNDERFLOW,
  GL_OUT_OF_MEMORY,
  GL_INVALID_FRAMEBUFFER_OPERATION
} GLErrorCode;

// Sets the current OpenGL error
void glSetError(GLErrorCode error);

// Retrieves the current OpenGL error and clears it after returning
GLErrorCode glGetError(void);

// Checks if any OpenGL error is present
GLboolean hasGLError(void);

#endif // GL_ERRORS_H
