#include "gl_errors.h"
#include "../gl_utils.h"
#include "../gl_state.h"
#include "gl_ext_common.h"
#include <GLES/gl.h>
#include <GLES/glext.h>

/* Tokens missing from the ES headers used by TexGen */
#ifndef GL_OBJECT_PLANE_OES
#define GL_OBJECT_PLANE_OES 0x2501
#endif
#ifndef GL_EYE_PLANE_OES
#define GL_EYE_PLANE_OES 0x2502
#endif
#ifndef GL_S
#define GL_S 0x2000
#define GL_T 0x2001
#define GL_R 0x2002
#define GL_Q 0x2003
#endif

EXT_REGISTER("GL_OES_fixed_point")
__attribute__((used)) int ext_link_dummy_OES_fixed_point = 0;

GL_API void GL_APIENTRY glAlphaFuncxOES(GLenum func, GLfixed ref)
{
	glAlphaFuncx(func, ref);
}

GL_API void GL_APIENTRY glClearColorxOES(GLfixed red, GLfixed green,
					 GLfixed blue, GLfixed alpha)
{
	glClearColorx(red, green, blue, alpha);
}

GL_API void GL_APIENTRY glClearDepthxOES(GLfixed depth)
{
	glClearDepthx(depth);
}

GL_API void GL_APIENTRY glClipPlanexOES(GLenum plane, const GLfixed *equation)
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

GL_API void GL_APIENTRY glColor4xOES(GLfixed red, GLfixed green, GLfixed blue,
				     GLfixed alpha)
{
	glColor4x(red, green, blue, alpha);
}

GL_API void GL_APIENTRY glDepthRangexOES(GLfixed n, GLfixed f)
{
	glDepthRangex(n, f);
}

GL_API void GL_APIENTRY glFogxOES(GLenum pname, GLfixed param)
{
	glFogf(pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glFogxvOES(GLenum pname, const GLfixed *param)
{
	if (!param) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4];
	vals[0] = fixed_to_float(param[0]);
	vals[1] = fixed_to_float(param[1]);
	vals[2] = fixed_to_float(param[2]);
	vals[3] = fixed_to_float(param[3]);
	glFogfv(pname, vals);
}

GL_API void GL_APIENTRY glFrustumxOES(GLfixed l, GLfixed r, GLfixed b,
				      GLfixed t, GLfixed n, GLfixed f)
{
	glFrustumx(l, r, b, t, n, f);
}

GL_API void GL_APIENTRY glGetClipPlanexOES(GLenum plane, GLfixed *equation)
{
	if (!equation) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat eq[4] = { 0, 0, 0, 0 };
	glGetClipPlanef(plane, eq);
	equation[0] = float_to_fixed(eq[0]);
	equation[1] = float_to_fixed(eq[1]);
	equation[2] = float_to_fixed(eq[2]);
	equation[3] = float_to_fixed(eq[3]);
}

GL_API void GL_APIENTRY glGetFixedvOES(GLenum pname, GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp[4] = { 0, 0, 0, 0 };
	glGetFloatv(pname, tmp);
	for (int i = 0; i < 4; ++i)
		params[i] = float_to_fixed(tmp[i]);
}

GL_API void GL_APIENTRY glGetLightxvOES(GLenum light, GLenum pname,
					GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp[4] = { 0, 0, 0, 0 };
	glGetLightfv(light, pname, tmp);
	params[0] = float_to_fixed(tmp[0]);
	params[1] = float_to_fixed(tmp[1]);
	params[2] = float_to_fixed(tmp[2]);
	params[3] = float_to_fixed(tmp[3]);
}

GL_API void GL_APIENTRY glGetMaterialxvOES(GLenum face, GLenum pname,
					   GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp[4] = { 0, 0, 0, 0 };
	glGetMaterialfv(face, pname, tmp);
	params[0] = float_to_fixed(tmp[0]);
	params[1] = float_to_fixed(tmp[1]);
	params[2] = float_to_fixed(tmp[2]);
	params[3] = float_to_fixed(tmp[3]);
}

GL_API void GL_APIENTRY glGetTexEnvxvOES(GLenum target, GLenum pname,
					 GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp[4] = { 0, 0, 0, 0 };
	glGetTexEnvfv(target, pname, tmp);
	params[0] = float_to_fixed(tmp[0]);
	params[1] = float_to_fixed(tmp[1]);
	params[2] = float_to_fixed(tmp[2]);
	params[3] = float_to_fixed(tmp[3]);
}

GL_API void GL_APIENTRY glGetTexParameterxvOES(GLenum target, GLenum pname,
					       GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat tmp[4] = { 0, 0, 0, 0 };
	glGetTexParameterfv(target, pname, tmp);
	params[0] = float_to_fixed(tmp[0]);
	params[1] = float_to_fixed(tmp[1]);
	params[2] = float_to_fixed(tmp[2]);
	params[3] = float_to_fixed(tmp[3]);
}

GL_API void GL_APIENTRY glLightModelxOES(GLenum pname, GLfixed param)
{
	glLightModelf(pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glLightModelxvOES(GLenum pname, const GLfixed *param)
{
	if (!param) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(param[0]), fixed_to_float(param[1]),
			    fixed_to_float(param[2]),
			    fixed_to_float(param[3]) };
	glLightModelfv(pname, vals);
}

GL_API void GL_APIENTRY glLightxOES(GLenum light, GLenum pname, GLfixed param)
{
	glLightf(light, pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glLightxvOES(GLenum light, GLenum pname,
				     const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4];
	vals[0] = fixed_to_float(params[0]);
	vals[1] = fixed_to_float(params[1]);
	vals[2] = fixed_to_float(params[2]);
	vals[3] = fixed_to_float(params[3]);
	glLightfv(light, pname, vals);
}

GL_API void GL_APIENTRY glLineWidthxOES(GLfixed width)
{
	glLineWidth(fixed_to_float(width));
}

GL_API void GL_APIENTRY glLoadMatrixxOES(const GLfixed *m)
{
	if (!m) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat mf[16];
	for (int i = 0; i < 16; ++i)
		mf[i] = fixed_to_float(m[i]);
	glLoadMatrixf(mf);
}

GL_API void GL_APIENTRY glMaterialxOES(GLenum face, GLenum pname, GLfixed param)
{
	glMaterialf(face, pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glMaterialxvOES(GLenum face, GLenum pname,
					const GLfixed *param)
{
	if (!param) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(param[0]), fixed_to_float(param[1]),
			    fixed_to_float(param[2]),
			    fixed_to_float(param[3]) };
	glMaterialfv(face, pname, vals);
}

GL_API void GL_APIENTRY glMultMatrixxOES(const GLfixed *m)
{
	if (!m) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat mf[16];
	for (int i = 0; i < 16; ++i)
		mf[i] = fixed_to_float(m[i]);
	glMultMatrixf(mf);
}

GL_API void GL_APIENTRY glMultiTexCoord4xOES(GLenum texture, GLfixed s,
					     GLfixed t, GLfixed r, GLfixed q)
{
	glMultiTexCoord4f(texture, fixed_to_float(s), fixed_to_float(t),
			  fixed_to_float(r), fixed_to_float(q));
}

GL_API void GL_APIENTRY glNormal3xOES(GLfixed nx, GLfixed ny, GLfixed nz)
{
	glNormal3f(fixed_to_float(nx), fixed_to_float(ny), fixed_to_float(nz));
}

GL_API void GL_APIENTRY glOrthoxOES(GLfixed l, GLfixed r, GLfixed b, GLfixed t,
				    GLfixed n, GLfixed f)
{
	glOrthox(l, r, b, t, n, f);
}

GL_API void GL_APIENTRY glRotatexOES(GLfixed angle, GLfixed x, GLfixed y,
				     GLfixed z)
{
	glRotatex(angle, x, y, z);
}

GL_API void GL_APIENTRY glSampleCoveragexOES(GLclampx value, GLboolean invert)
{
	glSampleCoverage(fixed_to_float(value), invert);
}

GL_API void GL_APIENTRY glScalexOES(GLfixed x, GLfixed y, GLfixed z)
{
	glScalex(x, y, z);
}

GL_API void GL_APIENTRY glTranslatexOES(GLfixed x, GLfixed y, GLfixed z)
{
	glTranslatex(x, y, z);
}

GL_API void GL_APIENTRY glTexGenfOES(GLenum coord, GLenum pname, GLfloat param)
{
	if (pname != GL_TEXTURE_GEN_MODE_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	glTexGenfvOES(coord, pname, &param);
}

GL_API void GL_APIENTRY glTexGenfvOES(GLenum coord, GLenum pname,
				      const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	int idx;
	switch (coord) {
	case GL_S:
		idx = 0;
		break;
	case GL_T:
		idx = 1;
		break;
	case GL_R:
		idx = 2;
		break;
	case GL_Q:
		idx = 3;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (pname == GL_TEXTURE_GEN_MODE_OES) {
		gl_state.tex_gen_mode[idx] = (GLenum)params[0];
	} else if (pname == GL_OBJECT_PLANE_OES || pname == GL_EYE_PLANE_OES) {
		for (int i = 0; i < 4; ++i)
			gl_state.tex_gen_plane[idx][i] = params[i];
	} else {
		glSetError(GL_INVALID_ENUM);
		return;
	}
}

GL_API void GL_APIENTRY glGetTexGenfvOES(GLenum coord, GLenum pname,
					 GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	int idx;
	switch (coord) {
	case GL_S:
		idx = 0;
		break;
	case GL_T:
		idx = 1;
		break;
	case GL_R:
		idx = 2;
		break;
	case GL_Q:
		idx = 3;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}

	switch (pname) {
	case GL_TEXTURE_GEN_MODE_OES:
		params[0] = (GLfloat)gl_state.tex_gen_mode[idx];
		break;
	case GL_OBJECT_PLANE_OES:
	case GL_EYE_PLANE_OES:
		for (int i = 0; i < 4; ++i)
			params[i] = gl_state.tex_gen_plane[idx][i];
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GL_API void GL_APIENTRY glTexGenxOES(GLenum coord, GLenum pname, GLfixed param)
{
	if (pname != GL_TEXTURE_GEN_MODE_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	glTexGenfOES(coord, pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glTexGenxvOES(GLenum coord, GLenum pname,
				      const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat fp[4] = { fixed_to_float(params[0]), fixed_to_float(params[1]),
			  fixed_to_float(params[2]),
			  fixed_to_float(params[3]) };
	glTexGenfvOES(coord, pname, fp);
}

GL_API void GL_APIENTRY glGetTexGenxvOES(GLenum coord, GLenum pname,
					 GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat fp[4] = { 0, 0, 0, 0 };
	glGetTexGenfvOES(coord, pname, fp);
	params[0] = float_to_fixed(fp[0]);
	params[1] = float_to_fixed(fp[1]);
	params[2] = float_to_fixed(fp[2]);
	params[3] = float_to_fixed(fp[3]);
}
