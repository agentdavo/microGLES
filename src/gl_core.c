#include "gl_core.h"
#include "gl_state.h"
#include "logger.h"

// Assuming GLState is a global structure
extern GLState gl_state;

void glAlphaFunc(GLenum func, GLfloat ref) {
    gl_state.alpha_func = func;
    gl_state.alpha_ref = ref;
    LOG_DEBUG("Set AlphaFunc: func=0x%X, ref=%f.", func, ref);
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    gl_state.clear_color[0] = red;
    gl_state.clear_color[1] = green;
    gl_state.clear_color[2] = blue;
    gl_state.clear_color[3] = alpha;
    LOG_DEBUG("Set ClearColor: (%f, %f, %f, %f).", red, green, blue, alpha);
}

void glClearDepthf(GLfloat d) {
    gl_state.clear_depth = d;
    LOG_DEBUG("Set ClearDepthf: %f.", d);
}

// ... Rest of the core functions

void glFinish(void) {
    LOG_DEBUG("Called Finish.");
}

void glFlush(void) {
    LOG_DEBUG("Called Flush.");
}

void glEnable(GLenum cap) {
    gl_state.enabled_caps |= cap;
    LOG_DEBUG("Enabled Capability: 0x%X.", cap);
}

void glDisable(GLenum cap) {
    gl_state.enabled_caps &= ~cap;
    LOG_DEBUG("Disabled Capability: 0x%X.", cap);
}

void glGetFloatv(GLenum pname, GLfloat *data) {
    switch(pname) {
        case GL_MODELVIEW_MATRIX:
            memcpy(data, gl_state.modelview_matrix.data, sizeof(mat4));
            break;
        case GL_PROJECTION_MATRIX:
            memcpy(data, gl_state.projection_matrix.data, sizeof(mat4));
            break;
        case GL_CURRENT_COLOR:
            memcpy(data, gl_state.current_color, sizeof(GLfloat) * 4);
            break;
        default:
            break;
    }
}

void glGetIntegerv(GLenum pname, GLint *data) {
    switch(pname) {
        case GL_MAX_TEXTURE_SIZE:
            data[0] = gl_state.max_texture_size;
            break;
        case GL_MAX_LIGHTS:
            data[0] = gl_state.max_lights;
            break;
        default:
            break;
    }
}
