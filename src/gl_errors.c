#include "gl_errors.h"
#include "logger.h"  // Assuming logger is responsible for debug logging

// This variable stores the current error state
static GLErrorCode current_gl_error = GL_NO_ERROR;

// Sets the current OpenGL error
void glSetError(GLErrorCode error) {
    if (current_gl_error == GL_NO_ERROR) {
        current_gl_error = error;
        LOG_DEBUG("OpenGL Error Set: 0x%X.", error);
    }
}

// Retrieves the current OpenGL error and clears it after returning
GLErrorCode glGetError(void) {
    GLErrorCode error = current_gl_error;
    if (current_gl_error != GL_NO_ERROR) {
        LOG_DEBUG("OpenGL Error Retrieved: 0x%X.", error);
        current_gl_error = GL_NO_ERROR;  // Clear the error after retrieval
    }
    return error;
}

// Checks if any OpenGL error is present
GLboolean hasGLError(void) {
    return current_gl_error != GL_NO_ERROR;
}

