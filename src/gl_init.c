#include "gl_init.h"
#include "gl_logger.h"
#include "gl_context.h"
#include "gl_errors.h"
#include "matrix_utils.h"
#include "pipeline/gl_framebuffer.h"
#include <GLES/gl.h>
#include <stdio.h>
#include <pthread.h>

static Framebuffer *g_default_fb = NULL;
static pthread_mutex_t g_fb_mutex = PTHREAD_MUTEX_INITIALIZER;

// Initializes OpenGL ES with default state.
void GL_init(void)
{
    glSetError(GL_NO_ERROR);
    context_init(); // Initialize context (assumed to set up gl_state)
    GL_resetState();
    GL_setupViewport(0, 0, 800, 600); // Default viewport
    GL_defaultMatrixSetup();
    LOG_INFO("OpenGL ES initialized with default state.");
}

// Cleans up OpenGL ES context and resources.
void GL_cleanup(void)
{
    context_cleanup(); // Assumed to clean up gl_state
    LOG_INFO("OpenGL ES cleanup completed.");
}

// Sets up the viewport and adjusts the projection matrix.
void GL_setupViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (width <= 0 || height <= 0) {
        glSetError(GL_INVALID_VALUE);
        LOG_ERROR("Invalid viewport dimensions: width=%d, height=%d", width, height);
        return;
    }

    glViewport(x, y, width, height);

    // Set up an orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0.0f, (GLfloat)width, 0.0f, (GLfloat)height, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    LOG_DEBUG("Viewport set: x=%d, y=%d, width=%d, height=%d", x, y, width, height);
}

// Resets all OpenGL ES states to their default values.
void GL_resetState(void)
{
    // Blending
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // More common default

    // Depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    // Clear values
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);

    // Textures
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);

    // Culling
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Stencil
    glDisable(GL_STENCIL_TEST);
    glStencilMask(~0U);

    // Point size
    glPointSize(1.0f);

    LOG_DEBUG("OpenGL ES state reset to defaults.");
}

// Sets up default matrices for projection and modelview.
void GL_defaultMatrixSetup(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthRangef(0.0f, 1.0f);

    LOG_DEBUG("Default matrix setup completed.");
}

// Initializes OpenGL ES with a framebuffer.
Framebuffer *GL_init_with_framebuffer(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0 || width > 16384 || height > 16384) {
        glSetError(GL_INVALID_VALUE);
        LOG_ERROR("Invalid framebuffer dimensions: %ux%u", width, height);
        return NULL;
    }

    glSetError(GL_NO_ERROR);
    context_init();
    GL_resetState();
    GL_setupViewport(0, 0, (GLsizei)width, (GLsizei)height);
    GL_defaultMatrixSetup();

    Framebuffer *fb = framebuffer_create(width, height);
    if (!fb) {
        glSetError(GL_OUT_OF_MEMORY);
        LOG_FATAL("Failed to create framebuffer %ux%u", width, height);
        context_cleanup();
        return NULL;
    }

    framebuffer_clear(fb, 0x00000000u, 1.0f, 0);

    pthread_mutex_lock(&g_fb_mutex);
    if (g_default_fb) {
        LOG_WARN("Overwriting existing default framebuffer");
        framebuffer_destroy(g_default_fb);
    }
    g_default_fb = fb;
    gl_state.default_framebuffer.fb = fb;
    gl_state.bound_framebuffer = &gl_state.default_framebuffer;
    pthread_mutex_unlock(&g_fb_mutex);

    LOG_INFO("Initialized renderer with framebuffer %ux%u", width, height);
    return fb;
}

// Cleans up the framebuffer and OpenGL ES context.
void GL_cleanup_with_framebuffer(Framebuffer *fb)
{
    pthread_mutex_lock(&g_fb_mutex);
    if (fb && fb == g_default_fb) {
        framebuffer_destroy(fb);
        g_default_fb = NULL;
        gl_state.default_framebuffer.fb = NULL;
        gl_state.bound_framebuffer = NULL;
        LOG_INFO("Destroyed default framebuffer");
    } else if (fb) {
        framebuffer_destroy(fb);
        LOG_INFO("Destroyed non-default framebuffer");
    }
    pthread_mutex_unlock(&g_fb_mutex);
    GL_cleanup();
}

// Returns the default framebuffer.
Framebuffer *GL_get_default_framebuffer(void)
{
    pthread_mutex_lock(&g_fb_mutex);
    Framebuffer *fb = g_default_fb;
    pthread_mutex_unlock(&g_fb_mutex);
    return fb;
}
