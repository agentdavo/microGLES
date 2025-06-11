#include "gl_state.h"
#include "gl_context.h"
#include "gl_errors.h"
#include "pipeline/gl_framebuffer.h"
#include <GLES/gl.h>
#include <string.h>

GL_API void GL_APIENTRY glClear(GLbitfield mask)
{
	const GLbitfield valid = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
				 GL_STENCIL_BUFFER_BIT;
	if (mask & ~valid) {
		glSetError(GL_INVALID_VALUE);
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
