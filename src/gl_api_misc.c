#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include <GLES/gl.h>
#include <string.h>
#include "gl_utils.h"
#include "gl_thread.h"
#include "command_buffer.h"

GL_API void GL_APIENTRY glClipPlanef(GLenum plane, const GLfloat *equation)
{
	if (!equation) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (plane < GL_CLIP_PLANE0 || plane > GL_CLIP_PLANE5) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	memcpy(gl_state.clip_planes[plane - GL_CLIP_PLANE0], equation,
	       sizeof(GLfloat) * 4);
}

GL_API void GL_APIENTRY glFogf(GLenum pname, GLfloat param)
{
	GLfloat tmp[4] = { param, 0, 0, 0 };
	glFogfv(pname, tmp);
}

GL_API void GL_APIENTRY glFogfv(GLenum pname, const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	switch (pname) {
	case GL_FOG_COLOR:
		memcpy(gl_state.fog_color, params, sizeof(GLfloat) * 4);
		context_set_fog(pname, params);
		break;
	case GL_FOG_DENSITY:
		gl_state.fog_density = params[0];
		context_set_fog(pname, params);
		break;
	case GL_FOG_START:
		gl_state.fog_start = params[0];
		context_set_fog(pname, params);
		break;
	case GL_FOG_END:
		gl_state.fog_end = params[0];
		context_set_fog(pname, params);
		break;
	case GL_FOG_MODE:
		gl_state.fog_mode = (GLenum)params[0];
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glGetClipPlanef(GLenum plane, GLfloat *equation)
{
	if (!equation) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (plane < GL_CLIP_PLANE0 || plane > GL_CLIP_PLANE5) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	memcpy(equation, gl_state.clip_planes[plane - GL_CLIP_PLANE0],
	       sizeof(GLfloat) * 4);
}

GL_API void GL_APIENTRY glGetLightfv(GLenum light, GLenum pname,
				     GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (light < GL_LIGHT0 || light > GL_LIGHT7) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	Light *lt = &gl_state.lights[light - GL_LIGHT0];
	switch (pname) {
	case GL_AMBIENT:
		memcpy(params, lt->ambient, sizeof(GLfloat) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(params, lt->diffuse, sizeof(GLfloat) * 4);
		break;
	case GL_SPECULAR:
		memcpy(params, lt->specular, sizeof(GLfloat) * 4);
		break;
	case GL_POSITION:
		memcpy(params, lt->position, sizeof(GLfloat) * 4);
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glGetMaterialfv(GLenum face, GLenum pname,
					GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	Material *mat = NULL;
	switch (face) {
	case GL_FRONT:
		mat = &gl_state.material[0];
		break;
	case GL_BACK:
		mat = &gl_state.material[1];
		break;
	case GL_FRONT_AND_BACK:
		mat = &gl_state.material[0];
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (!mat)
		return;
	switch (pname) {
	case GL_AMBIENT:
		memcpy(params, mat->ambient, sizeof(GLfloat) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(params, mat->diffuse, sizeof(GLfloat) * 4);
		break;
	case GL_SPECULAR:
		memcpy(params, mat->specular, sizeof(GLfloat) * 4);
		break;
	case GL_EMISSION:
		memcpy(params, mat->emission, sizeof(GLfloat) * 4);
		break;
	case GL_SHININESS:
		params[0] = mat->shininess;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glGetTexEnvfv(GLenum target, GLenum pname,
				      GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	RenderContext *ctx = GetCurrentContext();
	int unit = ctx->active_texture - GL_TEXTURE0;
	switch (pname) {
	case GL_TEXTURE_ENV_COLOR:
		memcpy(params, ctx->texture_env[unit].env_color,
		       sizeof(GLfloat) * 4);
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glGetTexParameterfv(GLenum target, GLenum pname,
					    GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_EXTERNAL_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	RenderContext *ctx = GetCurrentContext();
	int unit = ctx->active_texture - GL_TEXTURE0;
	TextureOES *tex =
		context_find_texture(ctx->texture_env[unit].bound_texture);
	if (!tex) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}
	switch (pname) {
	case GL_TEXTURE_MIN_FILTER:
		*params = (GLfloat)tex->min_filter;
		break;
	case GL_TEXTURE_MAG_FILTER:
		*params = (GLfloat)tex->mag_filter;
		break;
	case GL_TEXTURE_WRAP_S:
		*params = (GLfloat)tex->wrap_s;
		break;
	case GL_TEXTURE_WRAP_T:
		*params = (GLfloat)tex->wrap_t;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glAlphaFuncx(GLenum func, GLfixed ref)
{
	glAlphaFunc(func, FIXED_TO_FLOAT(ref));
}

GL_API void GL_APIENTRY glClearDepthx(GLfixed depth)
{
	glClearDepthf(FIXED_TO_FLOAT(depth));
}

GL_API void GL_APIENTRY glColor4x(GLfixed r, GLfixed g, GLfixed b, GLfixed a)
{
	glColor4f(FIXED_TO_FLOAT(r), FIXED_TO_FLOAT(g), FIXED_TO_FLOAT(b),
		  FIXED_TO_FLOAT(a));
}

GL_API void GL_APIENTRY glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	RenderContext *ctx = GetCurrentContext();
	ctx->current_color[0] = r;
	ctx->current_color[1] = g;
	ctx->current_color[2] = b;
	ctx->current_color[3] = a;
}

GL_API void GL_APIENTRY glDepthRangex(GLfixed n, GLfixed f)
{
	glDepthRangef(FIXED_TO_FLOAT(n), FIXED_TO_FLOAT(f));
}

GL_API void GL_APIENTRY glFrustumx(GLfixed l, GLfixed r, GLfixed b, GLfixed t,
				   GLfixed n, GLfixed f)
{
	glFrustumf(FIXED_TO_FLOAT(l), FIXED_TO_FLOAT(r), FIXED_TO_FLOAT(b),
		   FIXED_TO_FLOAT(t), FIXED_TO_FLOAT(n), FIXED_TO_FLOAT(f));
}

GL_API void GL_APIENTRY glLineWidthx(GLfixed width)
{
	glLineWidth(FIXED_TO_FLOAT(width));
}

GL_API void GL_APIENTRY glLineWidth(GLfloat width)
{
	if (width <= 0.0f) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	gl_state.line_width = width;
}

GL_API void GL_APIENTRY glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t,
					  GLfloat r, GLfloat q)
{
	if (target < GL_TEXTURE0 || target >= GL_TEXTURE0 + 8) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	RenderContext *ctx = GetCurrentContext();
	GLfloat *coord = ctx->current_texcoord[target - GL_TEXTURE0];
	coord[0] = s;
	coord[1] = t;
	coord[2] = r;
	coord[3] = q;
}

GL_API void GL_APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	RenderContext *ctx = GetCurrentContext();
	ctx->current_normal[0] = nx;
	ctx->current_normal[1] = ny;
	ctx->current_normal[2] = nz;
}

GL_API void GL_APIENTRY glOrthox(GLfixed l, GLfixed r, GLfixed b, GLfixed t,
				 GLfixed n, GLfixed f)
{
	glOrthof(FIXED_TO_FLOAT(l), FIXED_TO_FLOAT(r), FIXED_TO_FLOAT(b),
		 FIXED_TO_FLOAT(t), FIXED_TO_FLOAT(n), FIXED_TO_FLOAT(f));
}

GL_API void GL_APIENTRY glRotatex(GLfixed angle, GLfixed x, GLfixed y,
				  GLfixed z)
{
	glRotatef(FIXED_TO_FLOAT(angle), FIXED_TO_FLOAT(x), FIXED_TO_FLOAT(y),
		  FIXED_TO_FLOAT(z));
}

GL_API void GL_APIENTRY glSampleCoverage(GLclampf value, GLboolean invert)
{
	if (value < 0.0f)
		value = 0.0f;
	if (value > 1.0f)
		value = 1.0f;
	gl_state.sample_coverage_value = value;
	gl_state.sample_coverage_invert = invert;
}

GL_API void GL_APIENTRY glScalex(GLfixed x, GLfixed y, GLfixed z)
{
	glScalef(FIXED_TO_FLOAT(x), FIXED_TO_FLOAT(y), FIXED_TO_FLOAT(z));
}

GL_API void GL_APIENTRY glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
	glTranslatef(FIXED_TO_FLOAT(x), FIXED_TO_FLOAT(y), FIXED_TO_FLOAT(z));
}

GL_API void GL_APIENTRY glFinish(void)
{
	command_buffer_flush();
	thread_pool_wait_timeout(1000);
}

GL_API void GL_APIENTRY glFlush(void)
{
	command_buffer_flush();
}

GLenum glGetError(void)
{
	return glGetErrorAndClear();
}
