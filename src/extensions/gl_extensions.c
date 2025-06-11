#include "gl_errors.h"
#include "gl_state.h"
#include "gl_types.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <string.h>
#include <math.h>

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

static const char *EXT_STRING =
	"GL_OES_draw_texture GL_OES_matrix_get GL_OES_point_size_array "
	"GL_OES_point_sprite GL_OES_framebuffer_object GL_OES_EGL_image "
	"GL_OES_EGL_image_external GL_OES_required_internalformat "
	"GL_OES_fixed_point GL_OES_texture_env_crossbar "
	"GL_OES_texture_mirrored_repeat GL_OES_texture_cube_map "
	"GL_OES_blend_subtract GL_OES_blend_func_separate "
	"GL_OES_blend_equation_separate GL_OES_stencil_wrap "
	"GL_OES_extended_matrix_palette";

extern GLState gl_state;
#ifdef __cplusplus
extern "C" {
#endif
GL_API void GL_APIENTRY glBlendEquationSeparateOES(GLenum modeRGB,
						   GLenum modeAlpha);
GL_API void GL_APIENTRY glTexGenfvOES(GLenum coord, GLenum pname,
				      const GLfloat *params);
#ifdef __cplusplus
}
#endif
#define MAX_PALETTE_MATRICES 32
#define FIXED_TO_FLOAT(x) ((GLfloat)(x) / 65536.0f)

static BufferObject *find_buffer(GLuint id)
{
	for (GLint i = 0; i < gl_state.buffer_count; ++i) {
		if (gl_state.buffers[i]->id == id)
			return gl_state.buffers[i];
	}
	return NULL;
}

static VertexArrayObject *find_vao(GLuint id)
{
	for (GLint i = 0; i < gl_state.vao_count; ++i) {
		if (gl_state.vaos[i]->id == id)
			return gl_state.vaos[i];
	}
	return NULL;
}

static TextureOES *find_texture(GLuint id)
{
	for (GLuint i = 0; i < gl_state.texture_count; ++i) {
		if (gl_state.textures[i]->id == id)
			return gl_state.textures[i];
	}
	return NULL;
}

const GLubyte *renderer_get_extensions(void)
{
	return (const GLubyte *)EXT_STRING;
}

static void draw_tex_rect(GLfloat x, GLfloat y, GLfloat z, GLfloat width,
			  GLfloat height)
{
	TextureOES *tex = find_texture(gl_state.bound_texture);
	if (!tex) {
		LOG_WARN("glDrawTex* called with no bound texture.");
		return;
	}
	LOG_INFO("DrawTex: (%f,%f,%f) %fx%f crop=%d,%d,%d,%d", x, y, z, width,
		 height, tex->crop_rect[0], tex->crop_rect[1],
		 tex->crop_rect[2], tex->crop_rect[3]);
}

GL_API void GL_APIENTRY glDrawTexsOES(GLshort x, GLshort y, GLshort z,
				      GLshort width, GLshort height)
{
	draw_tex_rect((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width,
		      (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexiOES(GLint x, GLint y, GLint z, GLint width,
				      GLint height)
{
	draw_tex_rect((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width,
		      (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z,
				      GLfixed width, GLfixed height)
{
	draw_tex_rect(FIXED_TO_FLOAT(x), FIXED_TO_FLOAT(y), FIXED_TO_FLOAT(z),
		      FIXED_TO_FLOAT(width), FIXED_TO_FLOAT(height));
}

GL_API void GL_APIENTRY glDrawTexsvOES(const GLshort *coords)
{
	if (!coords) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	draw_tex_rect((GLfloat)coords[0], (GLfloat)coords[1],
		      (GLfloat)coords[2], (GLfloat)coords[3],
		      (GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexivOES(const GLint *coords)
{
	if (!coords) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	draw_tex_rect((GLfloat)coords[0], (GLfloat)coords[1],
		      (GLfloat)coords[2], (GLfloat)coords[3],
		      (GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexxvOES(const GLfixed *coords)
{
	if (!coords) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	draw_tex_rect(FIXED_TO_FLOAT(coords[0]), FIXED_TO_FLOAT(coords[1]),
		      FIXED_TO_FLOAT(coords[2]), FIXED_TO_FLOAT(coords[3]),
		      FIXED_TO_FLOAT(coords[4]));
}

GL_API void GL_APIENTRY glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z,
				      GLfloat width, GLfloat height)
{
	draw_tex_rect(x, y, z, width, height);
}

GL_API void GL_APIENTRY glDrawTexfvOES(const GLfloat *coords)
{
	if (!coords) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	draw_tex_rect(coords[0], coords[1], coords[2], coords[3], coords[4]);
}

/* Blend and TexGen extension stubs */
GL_API void GL_APIENTRY glBlendEquationOES(GLenum mode)
{
	glBlendEquationSeparateOES(mode, mode);
}

GL_API void GL_APIENTRY glBlendFuncSeparateOES(GLenum srcRGB, GLenum dstRGB,
					       GLenum srcAlpha, GLenum dstAlpha)
{
	gl_state.blend_sfactor = srcRGB;
	gl_state.blend_dfactor = dstRGB;
	gl_state.blend_sfactor_alpha = srcAlpha;
	gl_state.blend_dfactor_alpha = dstAlpha;
	glBlendFunc(srcRGB, dstRGB);
}

static GLboolean valid_blend_equation(GLenum mode)
{
	switch (mode) {
	case GL_FUNC_ADD_OES:
	case GL_FUNC_SUBTRACT_OES:
	case GL_FUNC_REVERSE_SUBTRACT_OES:
		return GL_TRUE;
	default:
		return GL_FALSE;
	}
}

GL_API void GL_APIENTRY glBlendEquationSeparateOES(GLenum modeRGB,
						   GLenum modeAlpha)
{
	if (!valid_blend_equation(modeRGB) ||
	    !valid_blend_equation(modeAlpha)) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	gl_state.blend_equation_rgb = modeRGB;
	gl_state.blend_equation_alpha = modeAlpha;
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

GL_API void GL_APIENTRY glTexGeniOES(GLenum coord, GLenum pname, GLint param)
{
	if (pname != GL_TEXTURE_GEN_MODE_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	GLfloat fp = (GLfloat)param;
	glTexGenfvOES(coord, pname, &fp);
}

GL_API void GL_APIENTRY glTexGenivOES(GLenum coord, GLenum pname,
				      const GLint *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat fp[4];
	for (int i = 0; i < 4; ++i)
		fp[i] = (GLfloat)params[i];
	glTexGenfvOES(coord, pname, fp);
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

GL_API void GL_APIENTRY glGetTexGenivOES(GLenum coord, GLenum pname,
					 GLint *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat fp[4] = { 0, 0, 0, 0 };
	glGetTexGenfvOES(coord, pname, fp);
	for (int i = 0; i < 4; ++i)
		params[i] = (GLint)fp[i];
}

GL_API void GL_APIENTRY glCurrentPaletteMatrixOES(GLuint matrixpaletteindex)
{
	if (matrixpaletteindex >= 32) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	gl_state.current_palette_matrix = matrixpaletteindex;
}

GL_API void GL_APIENTRY glLoadPaletteFromModelViewMatrixOES(void)
{
	if (gl_state.current_palette_matrix < 32) {
		gl_state.palette_matrices[gl_state.current_palette_matrix] =
			gl_state.modelview_matrix;
	}
}

GL_API void GL_APIENTRY glMatrixIndexPointerOES(GLint size, GLenum type,
						GLsizei stride,
						const void *pointer)
{
	gl_state.matrix_index_array_size = size;
	gl_state.matrix_index_array_type = type;
	gl_state.matrix_index_array_stride = stride;
	gl_state.matrix_index_array_pointer = pointer;
	if (gl_state.bound_vao) {
		gl_state.bound_vao->matrix_index_array_size = size;
		gl_state.bound_vao->matrix_index_array_type = type;
		gl_state.bound_vao->matrix_index_array_stride = stride;
		gl_state.bound_vao->matrix_index_array_pointer = pointer;
	}
}

GL_API void GL_APIENTRY glWeightPointerOES(GLint size, GLenum type,
					   GLsizei stride, const void *pointer)
{
	gl_state.weight_array_size = size;
	gl_state.weight_array_type = type;
	gl_state.weight_array_stride = stride;
	gl_state.weight_array_pointer = pointer;
	if (gl_state.bound_vao) {
		gl_state.bound_vao->weight_array_size = size;
		gl_state.bound_vao->weight_array_type = type;
		gl_state.bound_vao->weight_array_stride = stride;
		gl_state.bound_vao->weight_array_pointer = pointer;
	}
}

GL_API void GL_APIENTRY glPointSizePointerOES(GLenum type, GLsizei stride,
					      const void *pointer)
{
	gl_state.point_size_array_type = type;
	gl_state.point_size_array_stride = stride;
	gl_state.point_size_array_pointer = pointer;
	if (gl_state.bound_vao) {
		gl_state.bound_vao->point_size_array_type = type;
		gl_state.bound_vao->point_size_array_stride = stride;
		gl_state.bound_vao->point_size_array_pointer = pointer;
	}
	LOG_INFO("glPointSizePointerOES set pointer=%p type=0x%X stride=%d.",
		 pointer, type, stride);
}

/* Helper to query current point size array pointer, used for testing */
const void *getPointSizePointerOES(GLenum *type, GLsizei *stride)
{
	if (type)
		*type = gl_state.point_size_array_type;
	if (stride)
		*stride = gl_state.point_size_array_stride;
	return gl_state.point_size_array_pointer;
}
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
	GLfloat eq[4] = { FIXED_TO_FLOAT(equation[0]),
			  FIXED_TO_FLOAT(equation[1]),
			  FIXED_TO_FLOAT(equation[2]),
			  FIXED_TO_FLOAT(equation[3]) };
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
	glFogf(pname, FIXED_TO_FLOAT(param));
}

GL_API void GL_APIENTRY glFogxvOES(GLenum pname, const GLfixed *param)
{
	if (!param) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4];
	vals[0] = FIXED_TO_FLOAT(param[0]);
	vals[1] = FIXED_TO_FLOAT(param[1]);
	vals[2] = FIXED_TO_FLOAT(param[2]);
	vals[3] = FIXED_TO_FLOAT(param[3]);
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
	equation[0] = (GLfixed)(eq[0] * 65536.0f);
	equation[1] = (GLfixed)(eq[1] * 65536.0f);
	equation[2] = (GLfixed)(eq[2] * 65536.0f);
	equation[3] = (GLfixed)(eq[3] * 65536.0f);
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
		params[i] = (GLfixed)(tmp[i] * 65536.0f);
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
	params[0] = (GLfixed)(tmp[0] * 65536.0f);
	params[1] = (GLfixed)(tmp[1] * 65536.0f);
	params[2] = (GLfixed)(tmp[2] * 65536.0f);
	params[3] = (GLfixed)(tmp[3] * 65536.0f);
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
	params[0] = (GLfixed)(tmp[0] * 65536.0f);
	params[1] = (GLfixed)(tmp[1] * 65536.0f);
	params[2] = (GLfixed)(tmp[2] * 65536.0f);
	params[3] = (GLfixed)(tmp[3] * 65536.0f);
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
	params[0] = (GLfixed)(tmp[0] * 65536.0f);
	params[1] = (GLfixed)(tmp[1] * 65536.0f);
	params[2] = (GLfixed)(tmp[2] * 65536.0f);
	params[3] = (GLfixed)(tmp[3] * 65536.0f);
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
	params[0] = (GLfixed)(tmp[0] * 65536.0f);
	params[1] = (GLfixed)(tmp[1] * 65536.0f);
	params[2] = (GLfixed)(tmp[2] * 65536.0f);
	params[3] = (GLfixed)(tmp[3] * 65536.0f);
}

GL_API void GL_APIENTRY glLightModelxOES(GLenum pname, GLfixed param)
{
	glLightModelf(pname, FIXED_TO_FLOAT(param));
}

GL_API void GL_APIENTRY glLightModelxvOES(GLenum pname, const GLfixed *param)
{
	if (!param) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { FIXED_TO_FLOAT(param[0]), FIXED_TO_FLOAT(param[1]),
			    FIXED_TO_FLOAT(param[2]),
			    FIXED_TO_FLOAT(param[3]) };
	glLightModelfv(pname, vals);
}

GL_API void GL_APIENTRY glLightxOES(GLenum light, GLenum pname, GLfixed param)
{
	glLightf(light, pname, FIXED_TO_FLOAT(param));
}

GL_API void GL_APIENTRY glLightxvOES(GLenum light, GLenum pname,
				     const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4];
	vals[0] = FIXED_TO_FLOAT(params[0]);
	vals[1] = FIXED_TO_FLOAT(params[1]);
	vals[2] = FIXED_TO_FLOAT(params[2]);
	vals[3] = FIXED_TO_FLOAT(params[3]);
	glLightfv(light, pname, vals);
}

GL_API void GL_APIENTRY glLineWidthxOES(GLfixed width)
{
	glLineWidth(FIXED_TO_FLOAT(width));
}

GL_API void GL_APIENTRY glLoadMatrixxOES(const GLfixed *m)
{
	if (!m) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat mf[16];
	for (int i = 0; i < 16; ++i)
		mf[i] = FIXED_TO_FLOAT(m[i]);
	glLoadMatrixf(mf);
}

GL_API void GL_APIENTRY glMaterialxOES(GLenum face, GLenum pname, GLfixed param)
{
	glMaterialf(face, pname, FIXED_TO_FLOAT(param));
}

GL_API void GL_APIENTRY glMaterialxvOES(GLenum face, GLenum pname,
					const GLfixed *param)
{
	if (!param) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { FIXED_TO_FLOAT(param[0]), FIXED_TO_FLOAT(param[1]),
			    FIXED_TO_FLOAT(param[2]),
			    FIXED_TO_FLOAT(param[3]) };
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
		mf[i] = FIXED_TO_FLOAT(m[i]);
	glMultMatrixf(mf);
}

GL_API void GL_APIENTRY glMultiTexCoord4xOES(GLenum texture, GLfixed s,
					     GLfixed t, GLfixed r, GLfixed q)
{
	glMultiTexCoord4f(texture, FIXED_TO_FLOAT(s), FIXED_TO_FLOAT(t),
			  FIXED_TO_FLOAT(r), FIXED_TO_FLOAT(q));
}

GL_API void GL_APIENTRY glNormal3xOES(GLfixed nx, GLfixed ny, GLfixed nz)
{
	glNormal3f(FIXED_TO_FLOAT(nx), FIXED_TO_FLOAT(ny), FIXED_TO_FLOAT(nz));
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
	glSampleCoverage(FIXED_TO_FLOAT(value), invert);
}

GL_API void GL_APIENTRY glScalexOES(GLfixed x, GLfixed y, GLfixed z)
{
	glScalex(x, y, z);
}

GL_API void GL_APIENTRY glTranslatexOES(GLfixed x, GLfixed y, GLfixed z)
{
	glTranslatex(x, y, z);
}

/* Additional extension wrappers and stubs */
GL_API void GL_APIENTRY glClearDepthfOES(GLclampf depth)
{
	glClearDepthf(depth);
}

GL_API void GL_APIENTRY glDepthRangefOES(GLclampf n, GLclampf f)
{
	glDepthRangef(n, f);
}

GL_API void GL_APIENTRY glFrustumfOES(GLfloat l, GLfloat r, GLfloat b,
				      GLfloat t, GLfloat n, GLfloat f)
{
	glFrustumf(l, r, b, t, n, f);
}

GL_API void GL_APIENTRY glOrthofOES(GLfloat l, GLfloat r, GLfloat b, GLfloat t,
				    GLfloat n, GLfloat f)
{
	glOrthof(l, r, b, t, n, f);
}

GL_API void GL_APIENTRY glClipPlanefOES(GLenum plane, const GLfloat *equation)
{
	glClipPlanef(plane, equation);
}

GL_API void GL_APIENTRY glGetClipPlanefOES(GLenum plane, GLfloat *equation)
{
	glGetClipPlanef(plane, equation);
}

GL_API void GL_APIENTRY glClipPlanefIMG(GLenum p, const GLfloat *eqn)
{
	glClipPlanef(p, eqn);
}

GL_API void GL_APIENTRY glClipPlanexIMG(GLenum p, const GLfixed *eqn)
{
	if (!eqn) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat eq[4] = { FIXED_TO_FLOAT(eqn[0]), FIXED_TO_FLOAT(eqn[1]),
			  FIXED_TO_FLOAT(eqn[2]), FIXED_TO_FLOAT(eqn[3]) };
	glClipPlanef(p, eq);
}

GL_API void GL_APIENTRY glEGLImageTargetTexture2DOES(GLenum target,
						     GLeglImageOES image)
{
	if (target != GL_TEXTURE_2D) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (!image) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	LOG_INFO(
		"glEGLImageTargetTexture2DOES called - EGLImage not supported");
	glSetError(GL_INVALID_OPERATION);
}

GL_API void GL_APIENTRY
glEGLImageTargetRenderbufferStorageOES(GLenum target, GLeglImageOES image)
{
	if (target != GL_RENDERBUFFER_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (!image) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	LOG_INFO(
		"glEGLImageTargetRenderbufferStorageOES called - EGLImage not supported");
	glSetError(GL_INVALID_OPERATION);
}

GL_API void GL_APIENTRY glBindVertexArrayOES(GLuint array)
{
	if (array == 0) {
		gl_state.bound_vao = NULL;
		return;
	}

	VertexArrayObject *vao = find_vao(array);
	if (!vao) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	gl_state.bound_vao = vao;

	gl_state.vertex_array_enabled = vao->vertex_array_enabled;
	gl_state.vertex_array_size = vao->vertex_array_size;
	gl_state.vertex_array_type = vao->vertex_array_type;
	gl_state.vertex_array_stride = vao->vertex_array_stride;
	gl_state.vertex_array_pointer = vao->vertex_array_pointer;

	gl_state.color_array_enabled = vao->color_array_enabled;
	gl_state.color_array_size = vao->color_array_size;
	gl_state.color_array_type = vao->color_array_type;
	gl_state.color_array_stride = vao->color_array_stride;
	gl_state.color_array_pointer = vao->color_array_pointer;

	gl_state.normal_array_enabled = vao->normal_array_enabled;
	gl_state.normal_array_type = vao->normal_array_type;
	gl_state.normal_array_stride = vao->normal_array_stride;
	gl_state.normal_array_pointer = vao->normal_array_pointer;

	gl_state.texcoord_array_enabled = vao->texcoord_array_enabled;
	gl_state.texcoord_array_size = vao->texcoord_array_size;
	gl_state.texcoord_array_type = vao->texcoord_array_type;
	gl_state.texcoord_array_stride = vao->texcoord_array_stride;
	gl_state.texcoord_array_pointer = vao->texcoord_array_pointer;

	gl_state.point_size_array_type = vao->point_size_array_type;
	gl_state.point_size_array_stride = vao->point_size_array_stride;
	gl_state.point_size_array_pointer = vao->point_size_array_pointer;

	gl_state.matrix_index_array_size = vao->matrix_index_array_size;
	gl_state.matrix_index_array_type = vao->matrix_index_array_type;
	gl_state.matrix_index_array_stride = vao->matrix_index_array_stride;
	gl_state.matrix_index_array_pointer = vao->matrix_index_array_pointer;

	gl_state.weight_array_size = vao->weight_array_size;
	gl_state.weight_array_type = vao->weight_array_type;
	gl_state.weight_array_stride = vao->weight_array_stride;
	gl_state.weight_array_pointer = vao->weight_array_pointer;
}

GL_API void GL_APIENTRY glDeleteVertexArraysOES(GLsizei n, const GLuint *arrays)
{
	if (!arrays || n < 0)
		return;
	for (GLsizei i = 0; i < n; ++i) {
		VertexArrayObject *vao = find_vao(arrays[i]);
		if (!vao)
			continue;
		if (gl_state.bound_vao == vao)
			gl_state.bound_vao = NULL;
		for (GLint j = 0; j < gl_state.vao_count; ++j) {
			if (gl_state.vaos[j] == vao) {
				memmove(&gl_state.vaos[j],
					&gl_state.vaos[j + 1],
					(size_t)(gl_state.vao_count - j - 1) *
						sizeof(VertexArrayObject *));
				break;
			}
		}
		gl_state.vao_count--;
		tracked_free(vao, sizeof(VertexArrayObject));
	}
}

GL_API void GL_APIENTRY glGenVertexArraysOES(GLsizei n, GLuint *arrays)
{
	if (!arrays || n < 0)
		return;
	for (GLsizei i = 0; i < n; ++i) {
		if (gl_state.vao_count >= MAX_VERTEX_ARRAYS) {
			arrays[i] = 0;
			continue;
		}
		VertexArrayObject *vao = (VertexArrayObject *)tracked_malloc(
			sizeof(VertexArrayObject));
		memset(vao, 0, sizeof(VertexArrayObject));
		vao->id = gl_state.next_vertex_array_id++;
		gl_state.vaos[gl_state.vao_count++] = vao;
		arrays[i] = vao->id;
	}
}

GLboolean glIsVertexArrayOES(GLuint array)
{
	return find_vao(array) != NULL;
}

GL_API void GL_APIENTRY glGetBufferPointervOES(GLenum target, GLenum pname,
					       void **params)
{
	if (!params || pname != GL_BUFFER_MAP_POINTER_OES)
		return;
	BufferObject *obj = NULL;
	if (target == GL_ARRAY_BUFFER)
		obj = find_buffer(gl_state.array_buffer_binding);
	else if (target == GL_ELEMENT_ARRAY_BUFFER)
		obj = find_buffer(gl_state.element_array_buffer_binding);
	if (obj)
		*params = obj->data;
	else
		*params = NULL;
}

void *glMapBufferOES(GLenum target, GLenum access)
{
	BufferObject *obj = NULL;
	if (target == GL_ARRAY_BUFFER)
		obj = find_buffer(gl_state.array_buffer_binding);
	else if (target == GL_ELEMENT_ARRAY_BUFFER)
		obj = find_buffer(gl_state.element_array_buffer_binding);
	(void)access;
	if (!obj || !obj->data) {
		glSetError(GL_INVALID_OPERATION);
		return NULL;
	}
	return obj->data;
}

GLboolean glUnmapBufferOES(GLenum target)
{
	BufferObject *obj = NULL;
	if (target == GL_ARRAY_BUFFER)
		obj = find_buffer(gl_state.array_buffer_binding);
	else if (target == GL_ELEMENT_ARRAY_BUFFER)
		obj = find_buffer(gl_state.element_array_buffer_binding);
	if (!obj) {
		glSetError(GL_INVALID_OPERATION);
		return GL_FALSE;
	}
	return GL_TRUE;
}

GLbitfield glQueryMatrixxOES(GLfixed *mantissa, GLint *exponent)
{
	if (!mantissa || !exponent)
		return 0;
	GLbitfield status = 0;
	for (int i = 0; i < 16; ++i) {
		float val = gl_state.modelview_matrix.data[i];
		int exp;
		float m = frexpf(val, &exp);
		mantissa[i] = (GLfixed)(m * 65536.0f);
		exponent[i] = exp;
	}
	return status;
}

GL_API void GL_APIENTRY glTexGenxOES(GLenum coord, GLenum pname, GLfixed param)
{
	if (pname != GL_TEXTURE_GEN_MODE_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	glTexGenfOES(coord, pname, FIXED_TO_FLOAT(param));
}

GL_API void GL_APIENTRY glTexGenxvOES(GLenum coord, GLenum pname,
				      const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat fp[4] = { FIXED_TO_FLOAT(params[0]), FIXED_TO_FLOAT(params[1]),
			  FIXED_TO_FLOAT(params[2]),
			  FIXED_TO_FLOAT(params[3]) };
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
	params[0] = (GLfixed)(fp[0] * 65536.0f);
	params[1] = (GLfixed)(fp[1] * 65536.0f);
	params[2] = (GLfixed)(fp[2] * 65536.0f);
	params[3] = (GLfixed)(fp[3] * 65536.0f);
}

GL_API void GL_APIENTRY glRenderbufferStorageMultisampleIMG(
	GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
	GLsizei height)
{
	if (samples != 0) {
		LOG_INFO(
			"glRenderbufferStorageMultisampleIMG: multisampling not supported, using single-sample");
	}
	glRenderbufferStorageOES(target, internalformat, width, height);
}

GL_API void GL_APIENTRY glFramebufferTexture2DMultisampleIMG(
	GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
	GLint level, GLsizei samples)
{
	if (samples != 0) {
		LOG_INFO(
			"glFramebufferTexture2DMultisampleIMG: multisampling not supported, using single-sample");
	}
	glFramebufferTexture2DOES(target, attachment, textarget, texture,
				  level);
}

/* EXT_multi_draw_arrays wrappers */
GL_API void GL_APIENTRY glMultiDrawArraysEXT(GLenum mode, const GLint *first,
					     const GLsizei *count,
					     GLsizei primcount)
{
	if (!first || !count || primcount < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < primcount; ++i)
		glDrawArrays(mode, first[i], count[i]);
}

GL_API void GL_APIENTRY glMultiDrawElementsEXT(GLenum mode,
					       const GLsizei *count,
					       GLenum type,
					       const void *const *indices,
					       GLsizei primcount)
{
	if (!count || !indices || primcount < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < primcount; ++i)
		glDrawElements(mode, count[i], type, indices[i]);
}
