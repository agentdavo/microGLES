#include "gl_init.h"
#include "gl_logger.h"
#include "gl_context.h"
#include "gl_errors.h"
#include "matrix_utils.h"
#include "pipeline/gl_framebuffer.h"
#include <GLES/gl.h>
#include <stdio.h>

static Framebuffer *g_default_fb = NULL;

// This initializes all the states for OpenGL ES and sets default values
void GL_init(void)
{
	// Set up default error state
	glSetError(GL_NO_ERROR);

	// Reset all states
	GL_resetState();

	// Set up default viewport
	GL_setupViewport(0, 0, 800, 600); // Default dimensions

	// Setup default matrices for projection and modelview
	GL_defaultMatrixSetup();

	LOG_DEBUG("OpenGL ES initialized with default state.");
}

// Cleans up the OpenGL ES context if needed (empty here, could free resources
// later)
void GL_cleanup(void)
{
	LOG_DEBUG("OpenGL ES cleanup completed.");
}

// This function sets up the viewport and adjusts the projection matrix
// accordingly
void GL_setupViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	if (width <= 0 || height <= 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}

	// Set the viewport in the global state
	glViewport(x, y, width, height);

	// Adjust the projection matrix for a basic orthogonal projection (2D setup as
	// default)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0.0f, (GLfloat)width, 0.0f, (GLfloat)height, -1.0f, 1.0f);

	// Switch back to modelview matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	LOG_DEBUG("Viewport set to: x=%d, y=%d, width=%d, height=%d", x, y,
		  width, height);
}

// Resets all OpenGL states to their default values
void GL_resetState(void)
{
	// Reset blending state
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);

	// Reset depth state
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	// Set the clear color to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepthf(1.0f);

	// Reset texture state
	glDisable(GL_TEXTURE_2D);

	// Reset culling state
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Set up stencil defaults
	glDisable(GL_STENCIL_TEST);
	glStencilMask(0xFFFFFFFF);

	// Default point size
	glPointSize(1.0f);

	LOG_DEBUG("OpenGL ES state reset to defaults.");
}

// This function sets up the default matrices for projection and modelview
void GL_defaultMatrixSetup(void)
{
	// Load identity into both modelview and projection matrix stacks
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set initial depth range (near and far clipping)
	glDepthRangef(0.0f, 1.0f);

	LOG_DEBUG("Default matrix setup completed.");
}

Framebuffer *GL_init_with_framebuffer(uint32_t width, uint32_t height)
{
	glSetError(GL_NO_ERROR);
	context_init();
	GL_resetState();
	GL_setupViewport(0, 0, (GLsizei)width, (GLsizei)height);
	GL_defaultMatrixSetup();
	Framebuffer *fb = framebuffer_create(width, height);
	if (!fb) {
		LOG_FATAL("Failed to create framebuffer %ux%u", width, height);
		return NULL;
	}
	framebuffer_clear(fb, 0x00000000u, 1.0f, 0);
	LOG_INFO("Initialized renderer with framebuffer %ux%u", width, height);
	g_default_fb = fb;
	gl_state.default_framebuffer.fb = fb;
	gl_state.bound_framebuffer = &gl_state.default_framebuffer;
	return fb;
}

void GL_cleanup_with_framebuffer(Framebuffer *fb)
{
	if (fb) {
		framebuffer_destroy(fb);
		LOG_INFO("Destroyed framebuffer");
	}
	g_default_fb = NULL;
	gl_state.default_framebuffer.fb = NULL;
	gl_state.bound_framebuffer = NULL;
	GL_cleanup();
}

Framebuffer *GL_get_default_framebuffer(void)
{
	return g_default_fb;
}
