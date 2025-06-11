#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

#include "matrix_utils.h"
#include <GLES/gl.h>
#include <stdatomic.h>
#include <threads.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	mat4 modelview_matrix;
	mat4 projection_matrix;
	mat4 texture_matrix;
	GLfloat current_color[4];
	GLfloat clear_color[4];
	GLboolean depth_test_enabled;
	GLenum depth_func;
	GLfloat current_normal[3];
	atomic_uint version_modelview;
	atomic_uint version_projection;
	atomic_uint version_texture;
} RenderContext;

void context_init(void);
RenderContext *context_get(void);
void context_update_modelview_matrix(const mat4 *mat);
void context_set_clear_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void context_set_error(GLenum error);
GLenum context_get_error(void);

void log_state_change(const char *msg);

#ifdef __cplusplus
}
#endif

#endif // GL_CONTEXT_H
