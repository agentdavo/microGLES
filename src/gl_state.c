/* gl_state.c */

#include <stddef.h>           // For size_t
#include <stdlib.h>           // For malloc and free
#include <GLES/gl.h>          // For OpenGL ES types
#include <GLES/glext.h>       // For extension types
#include "gl_state.h"
#include "gl_framebuffer_object.h" // For RenderbufferOES, FramebufferOES
#include "gl_texture.h"           // For TextureOES
#include "gl_utils.h"             // For tracked_free
#include "memory_tracker.h"
#include "logger.h"

GLState gl_state; // Global GLState instance

void InitGLState(GLState* state) {
    state->next_renderbuffer_id = 1;
    state->renderbuffer_count = 0;
    state->bound_renderbuffer = NULL;

    state->next_framebuffer_id = 1;
    state->framebuffer_count = 0;
    state->bound_framebuffer = &state->default_framebuffer;
    state->default_framebuffer.id = 0;

    state->texture_count = 0;

    LOG_DEBUG("GLState initialized.");
}

void CleanupGLState(GLState* state) {
    for (GLint i = 0; i < state->renderbuffer_count; ++i) {
        tracked_free(state->renderbuffers[i], sizeof(RenderbufferOES));
    }
    state->renderbuffer_count = 0;

    for (GLint i = 0; i < state->framebuffer_count; ++i) {
        tracked_free(state->framebuffers[i], sizeof(FramebufferOES));
    }
    state->framebuffer_count = 0;

    for (GLuint i = 0; i < state->texture_count; ++i) {
        tracked_free(state->textures[i], sizeof(TextureOES));
    }
    state->texture_count = 0;

    LOG_DEBUG("GLState cleaned up.");
}