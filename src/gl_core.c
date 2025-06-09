#include "gl_core.h"
#include "gl_state.h"
#include "logger.h"

// Assuming GLState is a global structure
extern GLState gl_state;

static inline GLfloat clamp_float(GLfloat v) {
  return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

void glAlphaFunc(GLenum func, GLfloat ref) {
  gl_state.alpha_func = func;
  gl_state.alpha_ref = clamp_float(ref);
  LOG_DEBUG("Set AlphaFunc: func=0x%X, ref=%f.", func, gl_state.alpha_ref);
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
  gl_state.clear_color[0] = clamp_float(red);
  gl_state.clear_color[1] = clamp_float(green);
  gl_state.clear_color[2] = clamp_float(blue);
  gl_state.clear_color[3] = clamp_float(alpha);
  LOG_DEBUG("Set ClearColor: (%f, %f, %f, %f).", gl_state.clear_color[0],
            gl_state.clear_color[1], gl_state.clear_color[2],
            gl_state.clear_color[3]);
}

void glClearDepthf(GLfloat d) {
  gl_state.clear_depth = clamp_float(d);
  LOG_DEBUG("Set ClearDepthf: %f.", gl_state.clear_depth);
}

void glDepthRangef(GLfloat n, GLfloat f) {
  gl_state.depth_range_near = clamp_float(n);
  gl_state.depth_range_far = clamp_float(f);
  LOG_DEBUG("Set DepthRangef: near=%f, far=%f.", gl_state.depth_range_near,
            gl_state.depth_range_far);
}

// ... Rest of the core functions

void glFinish(void) { LOG_DEBUG("Called Finish."); }

void glFlush(void) { LOG_DEBUG("Called Flush."); }

void glEnable(GLenum cap) {
  gl_state.enabled_caps |= cap;
  LOG_DEBUG("Enabled Capability: 0x%X.", cap);
}

void glDisable(GLenum cap) {
  gl_state.enabled_caps &= ~cap;
  LOG_DEBUG("Disabled Capability: 0x%X.", cap);
}

void glGetFloatv(GLenum pname, GLfloat *data) {
  switch (pname) {
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
  switch (pname) {
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
