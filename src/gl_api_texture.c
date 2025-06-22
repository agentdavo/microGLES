#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include "gl_utils.h"
#include "gl_thread.h"
#include "function_profile.h"
#include <GLES/gl.h>
#include <string.h>

GL_API void GL_APIENTRY glActiveTexture(GLenum texture)
{
	if (texture < GL_TEXTURE0 || texture > GL_TEXTURE1) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	context_active_texture(texture);
}

GL_API void GL_APIENTRY glBindTexture(GLenum target, GLuint texture)
{
	PROFILE_START("glBindTexture");
	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_EXTERNAL_OES) {
		glSetError(GL_INVALID_ENUM);
		PROFILE_END("glBindTexture");
		return;
	}
	RenderContext *ctx = GetCurrentContext();
	context_bind_texture(ctx->active_texture - GL_TEXTURE0, target,
			     texture);
	PROFILE_END("glBindTexture");
}

GL_API void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
	if (n < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (!textures)
		return;
	context_gen_textures(n, textures);
}

GL_API void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
	if (n < 0 || !textures)
		return;
	context_delete_textures(n, textures);
}

GL_API void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname,
					GLfloat param)
{
	glTexParameteri(target, pname, (GLint)param);
}

GL_API void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname,
					 const GLfloat *params)
{
	glTexParameteri(target, pname, (GLint)params[0]);
}

GL_API void GL_APIENTRY glTexParameterx(GLenum target, GLenum pname,
					GLfixed param)
{
	glTexParameterf(target, pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname,
					 const GLint *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	glTexParameteri(target, pname, params[0]);
}

GL_API void GL_APIENTRY glTexParameterxv(GLenum target, GLenum pname,
					 const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(params[0]),
			    fixed_to_float(params[1]),
			    fixed_to_float(params[2]),
			    fixed_to_float(params[3]) };
	glTexParameterfv(target, pname, vals);
}

GL_API void GL_APIENTRY glTexParameterxOES(GLenum target, GLenum pname,
					   GLfixed param)
{
	glTexParameterf(target, pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glTexParameterxvOES(GLenum target, GLenum pname,
					    const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(params[0]),
			    fixed_to_float(params[1]),
			    fixed_to_float(params[2]),
			    fixed_to_float(params[3]) };
	glTexParameterfv(target, pname, vals);
}

GL_API void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname,
					GLint param)
{
	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_EXTERNAL_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	context_tex_parameterf(target, pname, (GLfloat)param);
}

GL_API void GL_APIENTRY glTexImage2D(GLenum target, GLint level,
				     GLint internalformat, GLsizei width,
				     GLsizei height, GLint border,
				     GLenum format, GLenum type,
				     const void *pixels)
{
	PROFILE_START("glTexImage2D");
	(void)border;
	context_tex_image_2d(target, level, internalformat, width, height,
			     format, type, pixels);
	PROFILE_END("glTexImage2D");
}

GL_API void GL_APIENTRY glTexSubImage2D(GLenum target, GLint level,
					GLint xoffset, GLint yoffset,
					GLsizei width, GLsizei height,
					GLenum format, GLenum type,
					const void *pixels)
{
	PROFILE_START("glTexSubImage2D");
	context_tex_sub_image_2d(target, level, xoffset, yoffset, width, height,
				 format, type, pixels);
	PROFILE_END("glTexSubImage2D");
}

GL_API void GL_APIENTRY glCompressedTexImage2D(GLenum target, GLint level,
					       GLenum internalformat,
					       GLsizei width, GLsizei height,
					       GLint border, GLsizei imageSize,
					       const void *data)
{
	(void)target;
	(void)level;
	(void)internalformat;
	(void)width;
	(void)height;
	(void)border;
	(void)imageSize;
	(void)data;
	glSetError(GL_INVALID_OPERATION);
}

GL_API void GL_APIENTRY glCompressedTexSubImage2D(
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
	GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
	(void)target;
	(void)level;
	(void)xoffset;
	(void)yoffset;
	(void)width;
	(void)height;
	(void)format;
	(void)imageSize;
	(void)data;
	glSetError(GL_INVALID_OPERATION);
}

GL_API void GL_APIENTRY glCopyTexImage2D(GLenum target, GLint level,
					 GLenum internalformat, GLint x,
					 GLint y, GLsizei width, GLsizei height,
					 GLint border)
{
	(void)target;
	(void)level;
	(void)internalformat;
	(void)x;
	(void)y;
	(void)width;
	(void)height;
	(void)border;
	glSetError(GL_INVALID_OPERATION);
}

GL_API void GL_APIENTRY glCopyTexSubImage2D(GLenum target, GLint level,
					    GLint xoffset, GLint yoffset,
					    GLint x, GLint y, GLsizei width,
					    GLsizei height)
{
	(void)target;
	(void)level;
	(void)xoffset;
	(void)yoffset;
	(void)x;
	(void)y;
	(void)width;
	(void)height;
	glSetError(GL_INVALID_OPERATION);
}
GL_API void GL_APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
	if (target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	int unit = GetCurrentContext()->active_texture - GL_TEXTURE0;
	switch (pname) {
	case GL_TEXTURE_ENV_MODE:
		switch ((GLenum)param) {
		case GL_MODULATE:
		case GL_DECAL:
		case GL_BLEND:
		case GL_ADD:
		case GL_REPLACE:
		case GL_COMBINE:
			break;
		default:
			glSetError(GL_INVALID_ENUM);
			break;
		}
		break;
	case GL_COMBINE_RGB:
		gl_state.tex_env_combine_rgb[unit] = (GLenum)param;
		break;
	case GL_COMBINE_ALPHA:
		gl_state.tex_env_combine_alpha[unit] = (GLenum)param;
		break;
	case GL_SRC0_RGB:
		gl_state.tex_env_src_rgb[unit][0] = (GLenum)param;
		break;
	case GL_SRC1_RGB:
		gl_state.tex_env_src_rgb[unit][1] = (GLenum)param;
		break;
	case GL_SRC2_RGB:
		gl_state.tex_env_src_rgb[unit][2] = (GLenum)param;
		break;
	case GL_SRC0_ALPHA:
		gl_state.tex_env_src_alpha[unit][0] = (GLenum)param;
		break;
	case GL_SRC1_ALPHA:
		gl_state.tex_env_src_alpha[unit][1] = (GLenum)param;
		break;
	case GL_SRC2_ALPHA:
		gl_state.tex_env_src_alpha[unit][2] = (GLenum)param;
		break;
	case GL_OPERAND0_RGB:
		gl_state.tex_env_operand_rgb[unit][0] = (GLenum)param;
		break;
	case GL_OPERAND1_RGB:
		gl_state.tex_env_operand_rgb[unit][1] = (GLenum)param;
		break;
	case GL_OPERAND2_RGB:
		gl_state.tex_env_operand_rgb[unit][2] = (GLenum)param;
		break;
	case GL_OPERAND0_ALPHA:
		gl_state.tex_env_operand_alpha[unit][0] = (GLenum)param;
		break;
	case GL_OPERAND1_ALPHA:
		gl_state.tex_env_operand_alpha[unit][1] = (GLenum)param;
		break;
	case GL_OPERAND2_ALPHA:
		gl_state.tex_env_operand_alpha[unit][2] = (GLenum)param;
		break;
	case GL_RGB_SCALE:
		if (param == 1.0f || param == 2.0f || param == 4.0f)
			gl_state.tex_env_rgb_scale[unit] = param;
		else
			glSetError(GL_INVALID_VALUE);
		break;
	case GL_ALPHA_SCALE:
		if (param == 1.0f || param == 2.0f || param == 4.0f)
			gl_state.tex_env_alpha_scale[unit] = param;
		else
			glSetError(GL_INVALID_VALUE);
		break;
	case GL_COORD_REPLACE_OES:
		gl_state.tex_env_coord_replace[unit] = param ? GL_TRUE :
							       GL_FALSE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (pname == GL_TEXTURE_ENV_MODE)
		context_set_texture_env(unit, pname, &param);
}

GL_API void GL_APIENTRY glTexEnvfv(GLenum target, GLenum pname,
				   const GLfloat *params)
{
	if ((target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) ||
	    !params) {
		if (!params)
			glSetError(GL_INVALID_VALUE);
		else
			glSetError(GL_INVALID_ENUM);
		return;
	}
	int unit = GetCurrentContext()->active_texture - GL_TEXTURE0;
	switch (pname) {
	case GL_TEXTURE_ENV_COLOR:
		context_set_texture_env(unit, pname, params);
		break;
	default:
		glTexEnvf(target, pname, params[0]);
		break;
	}
}

GL_API void GL_APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
	glTexEnvf(target, pname, (GLfloat)param);
}

GL_API void GL_APIENTRY glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
	glTexEnvf(target, pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glTexEnviv(GLenum target, GLenum pname,
				   const GLint *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	glTexEnvi(target, pname, params[0]);
}

GL_API void GL_APIENTRY glTexEnvxv(GLenum target, GLenum pname,
				   const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(params[0]),
			    fixed_to_float(params[1]),
			    fixed_to_float(params[2]),
			    fixed_to_float(params[3]) };
	glTexEnvfv(target, pname, vals);
}

GL_API void GL_APIENTRY glTexEnvxOES(GLenum target, GLenum pname, GLfixed param)
{
	glTexEnvf(target, pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glTexEnvxvOES(GLenum target, GLenum pname,
				      const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	GLfloat vals[4] = { fixed_to_float(params[0]),
			    fixed_to_float(params[1]),
			    fixed_to_float(params[2]),
			    fixed_to_float(params[3]) };
	glTexEnvfv(target, pname, vals);
}
