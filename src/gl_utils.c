#include "gl_utils.h"
#include "gl_errors.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_context.h"          // For gl_state
#include "pipeline/gl_framebuffer.h" // For Framebuffer
#include <GLES/gl.h>
#include <GLES/glext.h>          // For GL_INVALID_FRAMEBUFFER_OPERATION_OES
#include <stdalign.h>
#include <stddef.h>
#include <pthread.h>

static pthread_mutex_t g_alloc_mutex = PTHREAD_MUTEX_INITIALIZER;

// Allocates memory with tracking.
void *tracked_malloc(size_t size)
{
    if (size == 0) {
        LOG_WARN("tracked_malloc: Requested zero bytes");
        return NULL;
    }

    pthread_mutex_lock(&g_alloc_mutex);
    void *ptr = MT_ALLOC(size, STAGE_FRAMEBUFFER);
    if (!ptr) {
        LOG_ERROR("tracked_malloc: Failed to allocate %zu bytes", size);
        glSetError(GL_OUT_OF_MEMORY);
    } else {
        LOG_DEBUG("tracked_malloc: Allocated %zu bytes at %p", size, ptr);
    }
    pthread_mutex_unlock(&g_alloc_mutex);
    return ptr;
}

// Allocates aligned memory with tracking.
void *tracked_aligned_alloc(size_t alignment, size_t size)
{
    if (size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0) {
        LOG_ERROR("tracked_aligned_alloc: Invalid alignment=%zu or size=%zu", alignment, size);
        glSetError(GL_INVALID_VALUE);
        return NULL;
    }

    pthread_mutex_lock(&g_alloc_mutex);
    void *ptr = MT_ALIGNED_ALLOC(alignment, size, STAGE_FRAMEBUFFER);
    if (!ptr) {
        LOG_WARN("tracked_aligned_alloc: Falling back to unaligned malloc");
        ptr = MT_ALLOC(size, STAGE_FRAMEBUFFER);
        if (!ptr) {
            LOG_ERROR("tracked_aligned_alloc: Failed to allocate %zu bytes", size);
            glSetError(GL_OUT_OF_MEMORY);
        } else {
            LOG_DEBUG("tracked_aligned_alloc: Allocated %zu bytes (unaligned) at %p", size, ptr);
        }
    } else {
        LOG_DEBUG("tracked_aligned_alloc: Allocated %zu bytes (aligned %zu) at %p", size, alignment, ptr);
    }
    pthread_mutex_unlock(&g_alloc_mutex);
    return ptr;
}

// Frees memory with tracking.
void tracked_free(void *ptr, size_t size)
{
    if (!ptr) {
        LOG_DEBUG("tracked_free: Ignoring NULL pointer");
        return;
    }

    pthread_mutex_lock(&g_alloc_mutex);
    MT_FREE(ptr, STAGE_FRAMEBUFFER);
    LOG_DEBUG("tracked_free: Freed %zu bytes at %p", size, ptr);
    pthread_mutex_unlock(&g_alloc_mutex);
}

// Validates framebuffer completeness.
GLboolean ValidateFramebufferCompleteness(void)
{
#ifndef GL_STATE_DEFINED
    LOG_ERROR("ValidateFramebufferCompleteness: gl_state not defined");
    glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_OES);
    return GL_FALSE;
#else
    Framebuffer *fb = gl_state.bound_framebuffer ? gl_state.bound_framebuffer->fb : NULL;
    if (!fb) {
        LOG_ERROR("ValidateFramebufferCompleteness: No framebuffer bound");
        glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_OES);
        return GL_FALSE;
    }

    // Check dimensions
    if (fb->width == 0 || fb->height == 0) {
        LOG_ERROR("ValidateFramebufferCompleteness: Invalid framebuffer dimensions");
        glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_OES);
        return GL_FALSE;
    }

    // Check buffer allocations
    if (!fb->color_buffer || !fb->depth_buffer || !fb->stencil_buffer) {
        LOG_ERROR("ValidateFramebufferCompleteness: Missing buffer allocations");
        glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_OES);
        return GL_FALSE;
    }

    LOG_DEBUG("ValidateFramebufferCompleteness: Framebuffer valid");
    return GL_TRUE;
#endif
}
