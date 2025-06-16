#ifndef GL_INIT_H
#define GL_INIT_H
/**
 * @file gl_init.h
 * @brief Context initialization helpers.
 */

#include "gl_errors.h"
#include "gl_state.h"
#include "pipeline/gl_framebuffer.h"
#include <GLES/gl.h>
#include <stdint.h>

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

// Initialize GL and create a software framebuffer
Framebuffer *GL_init_with_framebuffer(uint32_t width, uint32_t height);

// Destroy the framebuffer and clean up GL state
void GL_cleanup_with_framebuffer(Framebuffer *fb);

// Retrieve the framebuffer created by GL_init_with_framebuffer
Framebuffer *GL_get_default_framebuffer(void);

#endif // GL_INIT_H
