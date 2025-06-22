#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include <GLES/gl.h>
#include <string.h>
#include "gl_utils.h"
#include "gl_thread.h"
#include "command_buffer.h"
#include "extensions/gl_ext_common.h"

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

GL_API void GL_APIENTRY glClipPlanex(GLenum plane, const GLfixed *equation)
{
	if (!equation) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat eq[4] = { fixed_to_float(equation[0]),
			  fixed_to_float(equation[1]),
			  fixed_to_float(equation[2]),
			  fixed_to_float(equation[3]) };
	glClipPlanef(plane, eq);
}

GL_API void GL_APIENTRY glFogf(GLenum pname, GLfloat param)
{
	GLfloat tmp[4] = { param, 0, 0, 0 };
	glFogfv(pname, tmp);
}

GL_API void GL_APIENTRY glFogx(GLenum pname, GLfixed param)
{
	glFogf(pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glFogxv(GLenum pname, const GLfixed *param)
{
	if (!param) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(param[0]), fixed_to_float(param[1]),
			    fixed_to_float(param[2]),
			    fixed_to_float(param[3]) };
	glFogfv(pname, vals);
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

GL_API void GL_APIENTRY glGetClipPlanex(GLenum plane, GLfixed *equation)
{
	if (!equation) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat eq[4];
	glGetClipPlanef(plane, eq);
	for (int i = 0; i < 4; ++i)
		equation[i] = float_to_fixed(eq[i]);
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

GL_API void GL_APIENTRY glGetLightxv(GLenum light, GLenum pname,
				     GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp[4];
	glGetLightfv(light, pname, tmp);
	for (int i = 0; i < 4; ++i)
		params[i] = float_to_fixed(tmp[i]);
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

GL_API void GL_APIENTRY glGetMaterialxv(GLenum face, GLenum pname,
					GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp[4];
	glGetMaterialfv(face, pname, tmp);
	for (int i = 0; i < 4; ++i)
		params[i] = float_to_fixed(tmp[i]);
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

GL_API void GL_APIENTRY glGetTexEnviv(GLenum target, GLenum pname,
				      GLint *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp[4];
	glGetTexEnvfv(target, pname, tmp);
	params[0] = (GLint)tmp[0];
}

GL_API void GL_APIENTRY glGetTexEnvxv(GLenum target, GLenum pname,
				      GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp[4];
	glGetTexEnvfv(target, pname, tmp);
	for (int i = 0; i < 4; ++i)
		params[i] = float_to_fixed(tmp[i]);
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
	glAlphaFunc(func, fixed_to_float(ref));
}

GL_API void GL_APIENTRY glClearDepthx(GLfixed depth)
{
	glClearDepthf(fixed_to_float(depth));
}

GL_API void GL_APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue,
				   GLubyte alpha)
{
	glColor4f(red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f);
}

GL_API void GL_APIENTRY glColor4x(GLfixed r, GLfixed g, GLfixed b, GLfixed a)
{
	glColor4f(fixed_to_float(r), fixed_to_float(g), fixed_to_float(b),
		  fixed_to_float(a));
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
	glDepthRangef(fixed_to_float(n), fixed_to_float(f));
}

GL_API void GL_APIENTRY glFrustumx(GLfixed l, GLfixed r, GLfixed b, GLfixed t,
				   GLfixed n, GLfixed f)
{
	glFrustumf(fixed_to_float(l), fixed_to_float(r), fixed_to_float(b),
		   fixed_to_float(t), fixed_to_float(n), fixed_to_float(f));
}

GL_API void GL_APIENTRY glLineWidthx(GLfixed width)
{
	glLineWidth(fixed_to_float(width));
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

GL_API void GL_APIENTRY glMultiTexCoord4x(GLenum texture, GLfixed s, GLfixed t,
					  GLfixed r, GLfixed q)
{
	glMultiTexCoord4f(texture, fixed_to_float(s), fixed_to_float(t),
			  fixed_to_float(r), fixed_to_float(q));
}

GL_API void GL_APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	RenderContext *ctx = GetCurrentContext();
	ctx->current_normal[0] = nx;
	ctx->current_normal[1] = ny;
	ctx->current_normal[2] = nz;
}

GL_API void GL_APIENTRY glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
	glNormal3f(fixed_to_float(nx), fixed_to_float(ny), fixed_to_float(nz));
}

GL_API void GL_APIENTRY glOrthox(GLfixed l, GLfixed r, GLfixed b, GLfixed t,
				 GLfixed n, GLfixed f)
{
	glOrthof(fixed_to_float(l), fixed_to_float(r), fixed_to_float(b),
		 fixed_to_float(t), fixed_to_float(n), fixed_to_float(f));
}

GL_API void GL_APIENTRY glRotatex(GLfixed angle, GLfixed x, GLfixed y,
				  GLfixed z)
{
	glRotatef(fixed_to_float(angle), fixed_to_float(x), fixed_to_float(y),
		  fixed_to_float(z));
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

GL_API void GL_APIENTRY glSampleCoveragex(GLclampx value, GLboolean invert)
{
	glSampleCoverage(fixed_to_float(value), invert);
}

GL_API void GL_APIENTRY glScalex(GLfixed x, GLfixed y, GLfixed z)
{
	glScalef(fixed_to_float(x), fixed_to_float(y), fixed_to_float(z));
}

GL_API void GL_APIENTRY glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
	glTranslatef(fixed_to_float(x), fixed_to_float(y), fixed_to_float(z));
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

GL_API const GLubyte *GL_APIENTRY glGetString(GLenum name)
{
	switch (name) {
	case GL_VENDOR:
		return (const GLubyte *)"microGLES";
	case GL_RENDERER:
		return (const GLubyte *)"microGLES Software Renderer";
	case GL_VERSION:
		return (const GLubyte *)"OpenGL ES 1.1";
	case GL_EXTENSIONS:
		return renderer_get_extensions();
	default:
		glSetError(GL_INVALID_ENUM);
		return NULL;
	}
}

GL_API void GL_APIENTRY glGetTexParameteriv(GLenum target, GLenum pname,
					    GLint *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp;
	glGetTexParameterfv(target, pname, &tmp);
	*params = (GLint)tmp;
}

GL_API void GL_APIENTRY glGetTexParameterxv(GLenum target, GLenum pname,
					    GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp;
	glGetTexParameterfv(target, pname, &tmp);
	*params = float_to_fixed(tmp);
}
