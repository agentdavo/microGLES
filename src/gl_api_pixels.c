#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include "pipeline/gl_framebuffer.h"
#include "gl_utils.h"
#include "gl_thread.h"
#include "function_profile.h"
#include <GLES/gl.h>
#include <string.h>

GL_API void GL_APIENTRY glClear(GLbitfield mask)
{
	PROFILE_START("glClear");
	const GLbitfield valid = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
				 GL_STENCIL_BUFFER_BIT;
	if (mask & ~valid) {
		glSetError(GL_INVALID_VALUE);
		PROFILE_END("glClear");
		return;
	}
	Framebuffer *fb = NULL;
	if (gl_state.bound_framebuffer)
		fb = gl_state.bound_framebuffer->fb;
	if (fb) {
		uint32_t color =
			((uint32_t)(gl_state.clear_color[3] * 255.0f) << 24) |
			((uint32_t)(gl_state.clear_color[2] * 255.0f) << 16) |
			((uint32_t)(gl_state.clear_color[1] * 255.0f) << 8) |
			((uint32_t)(gl_state.clear_color[0] * 255.0f));
		framebuffer_clear_async(fb, color, gl_state.clear_depth,
					(uint8_t)gl_state.clear_stencil);
	}
	PROFILE_END("glClear");
}

GL_API void GL_APIENTRY glReadPixels(GLint x, GLint y, GLsizei width,
				     GLsizei height, GLenum format, GLenum type,
				     void *pixels)
{
	(void)x;
	(void)y;
	if (width < 0 || height < 0 || !pixels) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	Framebuffer *fb = NULL;
	if (gl_state.bound_framebuffer)
		fb = gl_state.bound_framebuffer->fb;
	if (!fb) {
		memset(pixels, 0, (size_t)width * height * 4);
		return;
	}
	for (GLsizei j = 0; j < height; ++j) {
		for (GLsizei i = 0; i < width; ++i) {
			uint32_t c = framebuffer_get_pixel(
				fb, (uint32_t)(x + i), (uint32_t)(y + j));
			uint8_t *dst =
				(uint8_t *)pixels + (size_t)(j * width + i) * 4;
			dst[0] = c & 0xFF;
			dst[1] = (c >> 8) & 0xFF;
			dst[2] = (c >> 16) & 0xFF;
			dst[3] = (c >> 24) & 0xFF;
		}
	}
}

GL_API void GL_APIENTRY glColorMask(GLboolean r, GLboolean g, GLboolean b,
				    GLboolean a)
{
	gl_state.color_mask[0] = r;
	gl_state.color_mask[1] = g;
	gl_state.color_mask[2] = b;
	gl_state.color_mask[3] = a;
}

GL_API void GL_APIENTRY glDepthRangef(GLfloat n, GLfloat f)
{
	if (n > f)
		n = f;
	if (n < 0.0f)
		n = 0.0f;
	if (f > 1.0f)
		f = 1.0f;
	gl_state.depth_range_near = n;
	gl_state.depth_range_far = f;
}

GL_API void GL_APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue,
				     GLfloat alpha)
{
	PROFILE_START("glClearColor");
	gl_state.clear_color[0] = red;
	gl_state.clear_color[1] = green;
	gl_state.clear_color[2] = blue;
	gl_state.clear_color[3] = alpha;
	PROFILE_END("glClearColor");
}

GL_API void GL_APIENTRY glClearColorx(GLfixed red, GLfixed green, GLfixed blue,
				      GLfixed alpha)
{
	glClearColor(fixed_to_float(red), fixed_to_float(green),
		     fixed_to_float(blue), fixed_to_float(alpha));
}

GL_API void GL_APIENTRY glPointSize(GLfloat size)
{
	if (size <= 0.0f) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	gl_state.point_size = size;
}

GL_API void GL_APIENTRY glPointSizex(GLfixed size)
{
	glPointSize(fixed_to_float(size));
}

GL_API void GL_APIENTRY glPointParameterf(GLenum pname, GLfloat param)
{
	switch (pname) {
	case GL_POINT_SIZE_MIN:
		gl_state.point_size_min = param;
		break;
	case GL_POINT_SIZE_MAX:
		gl_state.point_size_max = param;
		break;
	case GL_POINT_FADE_THRESHOLD_SIZE:
		gl_state.point_fade_threshold_size = param;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glPointParameterx(GLenum pname, GLfixed param)
{
	glPointParameterf(pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glPointParameterfv(GLenum pname, const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	glPointParameterf(pname, params[0]);
}

GL_API void GL_APIENTRY glPointParameterxv(GLenum pname, const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	glPointParameterf(pname, fixed_to_float(params[0]));
}

GL_API void GL_APIENTRY glPixelStorei(GLenum pname, GLint param)
{
	switch (pname) {
	case GL_PACK_ALIGNMENT:
		if (param == 1 || param == 2 || param == 4 || param == 8)
			gl_state.pack_alignment = param;
		else
			glSetError(GL_INVALID_VALUE);
		break;
	case GL_UNPACK_ALIGNMENT:
		if (param == 1 || param == 2 || param == 4 || param == 8)
			gl_state.unpack_alignment = param;
		else
			glSetError(GL_INVALID_VALUE);
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glPointSizexOES(GLfixed size)
{
	glPointSize(fixed_to_float(size));
}

GL_API void GL_APIENTRY glPointParameterxOES(GLenum pname, GLfixed param)
{
	glPointParameterf(pname, fixed_to_float(param));
}

GL_API void GL_APIENTRY glPointParameterxvOES(GLenum pname,
					      const GLfixed *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	glPointParameterf(pname, fixed_to_float(params[0]));
}

GL_API void GL_APIENTRY glPointParameterfOES(GLenum pname, GLfloat param)
{
	glPointParameterf(pname, param);
}

GL_API void GL_APIENTRY glPointParameterfvOES(GLenum pname,
					      const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	glPointParameterf(pname, params[0]);
}
