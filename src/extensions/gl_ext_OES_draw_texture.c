#include "gl_errors.h"
#include "../gl_utils.h"
#include "../gl_logger.h"
#include "gl_ext_common.h"
#include <GLES/glext.h>
EXT_REGISTER("GL_OES_draw_texture")
__attribute__((used)) int ext_link_dummy_OES_draw_texture = 0;

static void draw_tex_rect(GLfloat x, GLfloat y, GLfloat z, GLfloat width,
			  GLfloat height)
{
	RenderContext *ctx = GetCurrentContext();
	TextureOES *tex =
		find_texture(ctx->texture_env[ctx->active_texture - GL_TEXTURE0]
				     .bound_texture);
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
	draw_tex_rect(fixed_to_float(x), fixed_to_float(y), fixed_to_float(z),
		      fixed_to_float(width), fixed_to_float(height));
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
	draw_tex_rect(fixed_to_float(coords[0]), fixed_to_float(coords[1]),
		      fixed_to_float(coords[2]), fixed_to_float(coords[3]),
		      fixed_to_float(coords[4]));
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

GL_API void GL_APIENTRY glTexParameterivOES(GLenum target, GLenum pname,
					    const GLint *params)
{
	if (target != GL_TEXTURE_2D) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	RenderContext *ctx = GetCurrentContext();
	TextureOES *tex =
		find_texture(ctx->texture_env[ctx->active_texture - GL_TEXTURE0]
				     .bound_texture);
	if (!tex) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}
	if (pname == GL_TEXTURE_CROP_RECT_OES) {
		for (int i = 0; i < 4; ++i)
			tex->crop_rect[i] = params[i];
	} else {
		glTexParameteri(target, pname, params[0]);
	}
}
