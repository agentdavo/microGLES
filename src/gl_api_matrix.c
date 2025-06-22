#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include <GLES/gl.h>
#include <string.h>
#include "matrix_utils.h"
#include "gl_utils.h"
#include "gl_thread.h"
#include "function_profile.h"

#define MODELVIEW_STACK_MAX 32
#define PROJECTION_STACK_MAX 2
#define TEXTURE_STACK_MAX 32

static mat4 *current_matrix_ptr(void)
{
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		return &gl_state.modelview_matrix;
	case GL_PROJECTION:
		return &gl_state.projection_matrix;
	case GL_TEXTURE:
		return &gl_state.texture_matrix;
	default:
		return NULL;
	}
}

static void sync_current_matrix(void)
{
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		context_update_modelview_matrix(&gl_state.modelview_matrix);
		break;
	case GL_PROJECTION:
		context_update_projection_matrix(&gl_state.projection_matrix);
		break;
	case GL_TEXTURE:
		context_update_texture_matrix(&gl_state.texture_matrix);
		break;
	default:
		break;
	}
}

static mat4 *stack_for_mode(GLenum mode, GLint **depth_out, GLint *max_depth)
{
	switch (mode) {
	case GL_MODELVIEW:
		*depth_out = &gl_state.modelview_stack_depth;
		*max_depth = MODELVIEW_STACK_MAX;
		return gl_state.modelview_stack;
	case GL_PROJECTION:
		*depth_out = &gl_state.projection_stack_depth;
		*max_depth = PROJECTION_STACK_MAX;
		return gl_state.projection_stack;
	case GL_TEXTURE:
		*depth_out = &gl_state.texture_stack_depth;
		*max_depth = TEXTURE_STACK_MAX;
		return gl_state.texture_stack;
	default:
		return NULL;
	}
}

GL_API void GL_APIENTRY glMatrixMode(GLenum mode)
{
	PROFILE_START("glMatrixMode");
	switch (mode) {
	case GL_MODELVIEW:
	case GL_PROJECTION:
	case GL_TEXTURE:
		gl_state.matrix_mode = mode;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		PROFILE_END("glMatrixMode");
		break;
	}
	PROFILE_END("glMatrixMode");
}

GL_API void GL_APIENTRY glPushMatrix(void)
{
	PROFILE_START("glPushMatrix");
	GLint *depth;
	GLint max_depth;
	mat4 *stack = stack_for_mode(gl_state.matrix_mode, &depth, &max_depth);
	if (!stack)
		return;
	if (*depth >= max_depth) {
		glSetError(GL_STACK_OVERFLOW);
		PROFILE_END("glPushMatrix");
		return;
	}
	mat4_copy(&stack[*depth], current_matrix_ptr());
	(*depth)++;
	RenderContext *ctx = context_get();
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		ctx->modelview_stack_depth = *depth;
		break;
	case GL_PROJECTION:
		ctx->projection_stack_depth = *depth;
		break;
	case GL_TEXTURE:
		ctx->texture_stack_depth = *depth;
		break;
	default:
		break;
	}
	PROFILE_END("glPushMatrix");
}

GL_API void GL_APIENTRY glPopMatrix(void)
{
	PROFILE_START("glPopMatrix");
	GLint *depth;
	GLint max_depth;
	mat4 *stack = stack_for_mode(gl_state.matrix_mode, &depth, &max_depth);
	if (!stack)
		return;
	if (*depth <= 1) {
		glSetError(GL_STACK_UNDERFLOW);
		PROFILE_END("glPopMatrix");
		return;
	}
	(*depth)--;
	RenderContext *ctx = context_get();
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		ctx->modelview_stack_depth = *depth;
		break;
	case GL_PROJECTION:
		ctx->projection_stack_depth = *depth;
		break;
	case GL_TEXTURE:
		ctx->texture_stack_depth = *depth;
		break;
	default:
		break;
	}
	mat4_copy(current_matrix_ptr(), &stack[*depth - 1]);
	sync_current_matrix();
	PROFILE_END("glPopMatrix");
}

GL_API void GL_APIENTRY glLoadIdentity(void)
{
	PROFILE_START("glLoadIdentity");
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		mat4_identity(&gl_state.modelview_matrix);
		break;
	case GL_PROJECTION:
		mat4_identity(&gl_state.projection_matrix);
		break;
	case GL_TEXTURE:
		mat4_identity(&gl_state.texture_matrix);
		break;
	default:
		break;
	}
	sync_current_matrix();
	PROFILE_END("glLoadIdentity");
}

GL_API void GL_APIENTRY glLoadMatrixf(const GLfloat *m)
{
	PROFILE_START("glLoadMatrixf");
	if (!m) {
		PROFILE_END("glLoadMatrixf");
		return;
	}
	mat4 mat;
	memcpy(mat.data, m, sizeof(GLfloat) * 16);
	mat4_copy(current_matrix_ptr(), &mat);
	sync_current_matrix();
	PROFILE_END("glLoadMatrixf");
}

GL_API void GL_APIENTRY glLoadMatrixx(const GLfixed *m)
{
	if (!m) {
		return;
	}
	GLfloat mf[16];
	for (int i = 0; i < 16; ++i)
		mf[i] = fixed_to_float(m[i]);
	glLoadMatrixf(mf);
}

GL_API void GL_APIENTRY glMultMatrixf(const GLfloat *m)
{
	PROFILE_START("glMultMatrixf");
	if (!m) {
		PROFILE_END("glMultMatrixf");
		return;
	}
	mat4 mat, result;
	memcpy(mat.data, m, sizeof(GLfloat) * 16);
	mat4_multiply(&result, current_matrix_ptr(), &mat);
	mat4_copy(current_matrix_ptr(), &result);
	sync_current_matrix();
	PROFILE_END("glMultMatrixf");
}

GL_API void GL_APIENTRY glMultMatrixx(const GLfixed *m)
{
	if (!m) {
		return;
	}
	GLfloat mf[16];
	for (int i = 0; i < 16; ++i)
		mf[i] = fixed_to_float(m[i]);
	glMultMatrixf(mf);
}

GL_API void GL_APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	PROFILE_START("glTranslatef");
	mat4 trans, result;
	mat4_identity(&trans);
	mat4_translate(&trans, x, y, z);
	mat4_multiply(&result, current_matrix_ptr(), &trans);
	mat4_copy(current_matrix_ptr(), &result);
	sync_current_matrix();
	PROFILE_END("glTranslatef");
}

GL_API void GL_APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y,
				  GLfloat z)
{
	PROFILE_START("glRotatef");
	mat4 rot, result;
	mat4_identity(&rot);
	mat4_rotate_axis(&rot, angle, x, y, z);
	mat4_multiply(&result, current_matrix_ptr(), &rot);
	mat4_copy(current_matrix_ptr(), &result);
	sync_current_matrix();
	PROFILE_END("glRotatef");
}

GL_API void GL_APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	PROFILE_START("glScalef");
	mat4 scale, result;
	mat4_identity(&scale);
	mat4_scale(&scale, x, y, z);
	mat4_multiply(&result, current_matrix_ptr(), &scale);
	mat4_copy(current_matrix_ptr(), &result);
	sync_current_matrix();
	PROFILE_END("glScalef");
}

GL_API void GL_APIENTRY glFrustumf(GLfloat l, GLfloat r, GLfloat b, GLfloat t,
				   GLfloat n, GLfloat f)
{
	if (n <= 0.0f || f <= 0.0f || l == r || b == t || n == f) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	mat4 frust, result;
	mat4_frustum(&frust, l, r, b, t, n, f);
	mat4_multiply(&result, current_matrix_ptr(), &frust);
	mat4_copy(current_matrix_ptr(), &result);
	sync_current_matrix();
}

GL_API void GL_APIENTRY glOrthof(GLfloat l, GLfloat r, GLfloat b, GLfloat t,
				 GLfloat n, GLfloat f)
{
	if (n == f || l == r || b == t) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	mat4 ortho, result;
	mat4_orthographic(&ortho, l, r, b, t, n, f);
	mat4_multiply(&result, current_matrix_ptr(), &ortho);
	mat4_copy(current_matrix_ptr(), &result);
	sync_current_matrix();
}
