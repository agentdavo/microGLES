#include "gl_errors.h"
#include "gl_state.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <string.h>
#include "pipeline/gl_vertex.h"
#include "pipeline/gl_primitive.h"
#include "pipeline/gl_raster.h"
#include "pipeline/gl_fragment.h"
#include "pipeline/gl_framebuffer.h"

extern GLState gl_state;
extern const GLubyte *renderer_get_extensions(void);
#ifndef GL_DEPTH_COMPONENT
#define GL_DEPTH_COMPONENT 0x1902
#endif
const void *getPointSizePointerOES(GLenum *type, GLsizei *stride);

#define FIXED_TO_FLOAT(x) ((GLfloat)(x) / 65536.0f)

static const GLubyte VENDOR_STRING[] = "microGLES";
static const GLubyte RENDERER_STRING[] = "Software Renderer";
static const GLubyte VERSION_STRING[] = "OpenGL ES-CM 1.1";

#define MODELVIEW_STACK_MAX 32
#define PROJECTION_STACK_MAX 32
#define TEXTURE_STACK_MAX 32

static GLboolean is_power_of_two(GLsizei v)
{
	return (v > 0) && ((v & (v - 1)) == 0);
}

static Material *select_material(GLenum face, int *count);

static GLboolean valid_light_enum(GLenum light)
{
	return light >= GL_LIGHT0 && light <= GL_LIGHT7;
}

static BufferObject *find_buffer(GLuint id)
{
	for (GLint i = 0; i < gl_state.buffer_count; ++i) {
		if (gl_state.buffers[i]->id == id)
			return gl_state.buffers[i];
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

/* Core OpenGL ES 1.1 entry point stubs */

GL_API void GL_APIENTRY glActiveTexture(GLenum texture)
{
	gl_state.active_texture = texture;
}
GL_API void GL_APIENTRY glAlphaFunc(GLenum func, GLfloat ref)
{
	gl_state.alpha_func = func;
	gl_state.alpha_ref = ref;
}
GL_API void GL_APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
	switch (target) {
	case GL_ARRAY_BUFFER:
		gl_state.array_buffer_binding = buffer;
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		gl_state.element_array_buffer_binding = buffer;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (buffer != 0 && !find_buffer(buffer)) {
		if (gl_state.buffer_count >= MAX_BUFFERS) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		BufferObject *obj =
			(BufferObject *)tracked_malloc(sizeof(BufferObject));
		if (!obj) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		obj->id = buffer;
		obj->size = 0;
		obj->usage = GL_STATIC_DRAW;
		obj->data = NULL;
		gl_state.buffers[gl_state.buffer_count++] = obj;
	}
}
GL_API void GL_APIENTRY glBindTexture(GLenum target, GLuint texture)
{
	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_EXTERNAL_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (target == GL_TEXTURE_EXTERNAL_OES)
		gl_state.bound_texture_external = texture;
	else
		gl_state.bound_texture = texture;
	if (texture != 0 && !find_texture(texture)) {
		if (gl_state.texture_count >= MAX_TEXTURES) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		TextureOES *tex =
			CreateTextureOES(target, GL_RGBA, 0, 0, GL_FALSE);
		if (!tex) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		tex->id = texture;
		gl_state.textures[gl_state.texture_count++] = tex;
	}
}
GL_API void GL_APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	gl_state.blend_sfactor = sfactor;
	gl_state.blend_dfactor = dfactor;
	gl_state.blend_sfactor_alpha = sfactor;
	gl_state.blend_dfactor_alpha = dfactor;
}
GL_API void GL_APIENTRY glBufferData(GLenum target, GLsizeiptr size,
				     const void *data, GLenum usage)
{
	BufferObject *obj = NULL;
	if (target == GL_ARRAY_BUFFER) {
		obj = find_buffer(gl_state.array_buffer_binding);
	} else if (target == GL_ELEMENT_ARRAY_BUFFER) {
		obj = find_buffer(gl_state.element_array_buffer_binding);
	} else {
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (!obj) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	if (obj->data) {
		tracked_free(obj->data, obj->size);
	}
	obj->data = NULL;
	obj->size = size;
	obj->usage = usage;
	if (size > 0) {
		obj->data = tracked_malloc(size);
		if (!obj->data) {
			glSetError(GL_OUT_OF_MEMORY);
			obj->size = 0;
			return;
		}
		if (data)
			memcpy(obj->data, data, size);
	}
}
GL_API void GL_APIENTRY glBufferSubData(GLenum target, GLintptr offset,
					GLsizeiptr size, const void *data)
{
	BufferObject *obj = NULL;
	if (target == GL_ARRAY_BUFFER) {
		obj = find_buffer(gl_state.array_buffer_binding);
	} else if (target == GL_ELEMENT_ARRAY_BUFFER) {
		obj = find_buffer(gl_state.element_array_buffer_binding);
	} else {
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (!obj || !obj->data || offset < 0 || offset + size > obj->size) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (size > 0 && data)
		memcpy((char *)obj->data + offset, data, size);
}
#include "gl_init.h"
#include "gl_thread.h"

GL_API void GL_APIENTRY glClear(GLbitfield mask)
{
	const GLbitfield valid = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
				 GL_STENCIL_BUFFER_BIT;
	if (mask & ~valid) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	Framebuffer *fb = GL_get_default_framebuffer();
	if (fb) {
		uint32_t color =
			((uint32_t)(gl_state.clear_color[3] * 255.0f) << 24) |
			((uint32_t)(gl_state.clear_color[2] * 255.0f) << 16) |
			((uint32_t)(gl_state.clear_color[1] * 255.0f) << 8) |
			((uint32_t)(gl_state.clear_color[0] * 255.0f));
		framebuffer_clear_async(fb, color, gl_state.clear_depth);
	}
}
GL_API void GL_APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue,
				     GLfloat alpha)
{
	gl_state.clear_color[0] = red;
	gl_state.clear_color[1] = green;
	gl_state.clear_color[2] = blue;
	gl_state.clear_color[3] = alpha;
}
GL_API void GL_APIENTRY glClearDepthf(GLfloat d)
{
	gl_state.clear_depth = d;
}
GL_API void GL_APIENTRY glClearStencil(GLint s)
{
	gl_state.clear_stencil = s;
}
GL_API void GL_APIENTRY glClientActiveTexture(GLenum texture)
{
	gl_state.client_active_texture = texture;
}
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
GL_API void GL_APIENTRY glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	gl_state.current_color[0] = r;
	gl_state.current_color[1] = g;
	gl_state.current_color[2] = b;
	gl_state.current_color[3] = a;
}

GL_API void GL_APIENTRY glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
	glColor4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

GL_API void GL_APIENTRY glColor4x(GLfixed r, GLfixed g, GLfixed b, GLfixed a)
{
	glColor4f(FIXED_TO_FLOAT(r), FIXED_TO_FLOAT(g), FIXED_TO_FLOAT(b),
		  FIXED_TO_FLOAT(a));
}
GL_API void GL_APIENTRY glColorMask(GLboolean r, GLboolean g, GLboolean b,
				    GLboolean a)
{
	gl_state.color_mask[0] = r;
	gl_state.color_mask[1] = g;
	gl_state.color_mask[2] = b;
	gl_state.color_mask[3] = a;
}
GL_API void GL_APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride,
				       const void *ptr)
{
	gl_state.color_array_size = size;
	gl_state.color_array_type = type;
	gl_state.color_array_stride = stride;
	gl_state.color_array_pointer = ptr;
	if (gl_state.bound_vao) {
		gl_state.bound_vao->color_array_size = size;
		gl_state.bound_vao->color_array_type = type;
		gl_state.bound_vao->color_array_stride = stride;
		gl_state.bound_vao->color_array_pointer = ptr;
	}
}
GL_API void GL_APIENTRY glCompressedTexImage2D(GLenum target, GLint level,
					       GLenum internalformat,
					       GLsizei width, GLsizei height,
					       GLint border, GLsizei imageSize,
					       const void *data)
{
	(void)imageSize; /* Compression not actually handled */
	glTexImage2D(target, level, internalformat, width, height, border,
		     internalformat, GL_UNSIGNED_BYTE, data);
}
GL_API void GL_APIENTRY glCompressedTexSubImage2D(
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
	GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
	(void)imageSize;
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, format,
			GL_UNSIGNED_BYTE, data);
}
GL_API void GL_APIENTRY glCopyTexImage2D(GLenum target, GLint level,
					 GLenum internalformat, GLint x,
					 GLint y, GLsizei width, GLsizei height,
					 GLint border)
{
	(void)x;
	(void)y;
	glTexImage2D(target, level, internalformat, width, height, border,
		     internalformat, GL_UNSIGNED_BYTE, NULL);
}
GL_API void GL_APIENTRY glCopyTexSubImage2D(GLenum target, GLint level,
					    GLint xoffset, GLint yoffset,
					    GLint x, GLint y, GLsizei width,
					    GLsizei height)
{
	(void)x;
	(void)y;
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, GL_RGBA,
			GL_UNSIGNED_BYTE, NULL);
}
GL_API void GL_APIENTRY glCullFace(GLenum mode)
{
	gl_state.cull_face_mode = mode;
}
GL_API void GL_APIENTRY glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
	if (n < 0)
		return;
	if (!buffers)
		return;
	for (GLsizei i = 0; i < n; ++i) {
		BufferObject *obj = find_buffer(buffers[i]);
		if (!obj)
			continue;
		if (gl_state.array_buffer_binding == buffers[i])
			gl_state.array_buffer_binding = 0;
		if (gl_state.element_array_buffer_binding == buffers[i])
			gl_state.element_array_buffer_binding = 0;
		tracked_free(obj->data, obj->size);
		tracked_free(obj, sizeof(BufferObject));
		for (GLint j = 0; j < gl_state.buffer_count; ++j) {
			if (gl_state.buffers[j] == obj) {
				memmove(&gl_state.buffers[j],
					&gl_state.buffers[j + 1],
					sizeof(BufferObject *) *
						(gl_state.buffer_count - j -
						 1));
				gl_state.buffer_count--;
				break;
			}
		}
	}
}
GL_API void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
	if (n < 0 || !textures)
		return;
	for (GLsizei i = 0; i < n; ++i) {
		TextureOES *tex = find_texture(textures[i]);
		if (!tex)
			continue;
		if (gl_state.bound_texture == textures[i])
			gl_state.bound_texture = 0;
		for (GLuint j = 0; j < gl_state.texture_count; ++j) {
			if (gl_state.textures[j] == tex) {
				tracked_free(tex, sizeof(TextureOES));
				memmove(&gl_state.textures[j],
					&gl_state.textures[j + 1],
					sizeof(TextureOES *) *
						(gl_state.texture_count - j -
						 1));
				gl_state.texture_count--;
				break;
			}
		}
	}
}
GL_API void GL_APIENTRY glDepthFunc(GLenum func)
{
	gl_state.depth_func = func;
}
GL_API void GL_APIENTRY glDepthMask(GLboolean flag)
{
	gl_state.depth_mask = flag;
}
GL_API void GL_APIENTRY glDepthRangef(GLfloat n, GLfloat f)
{
	if (n < 0.0f)
		n = 0.0f;
	if (n > 1.0f)
		n = 1.0f;
	if (f < 0.0f)
		f = 0.0f;
	if (f > 1.0f)
		f = 1.0f;
	gl_state.depth_range_near = n;
	gl_state.depth_range_far = f;
}
GL_API void GL_APIENTRY glDisable(GLenum cap)
{
	switch (cap) {
	case GL_ALPHA_TEST:
		gl_state.alpha_test_enabled = GL_FALSE;
		break;
	case GL_BLEND:
		gl_state.blend_enabled = GL_FALSE;
		break;
	case GL_COLOR_LOGIC_OP:
		gl_state.color_logic_op_enabled = GL_FALSE;
		break;
	case GL_COLOR_MATERIAL:
		gl_state.color_material_enabled = GL_FALSE;
		break;
	case GL_CULL_FACE:
		gl_state.cull_face_enabled = GL_FALSE;
		break;
	case GL_DEPTH_TEST:
		gl_state.depth_test_enabled = GL_FALSE;
		break;
	case GL_DITHER:
		gl_state.dither_enabled = GL_FALSE;
		break;
	case GL_FOG:
		gl_state.fog_enabled = GL_FALSE;
		break;
	case GL_LIGHTING:
		gl_state.lighting_enabled = GL_FALSE;
		break;
	case GL_LINE_SMOOTH:
		gl_state.line_smooth_enabled = GL_FALSE;
		break;
	case GL_MULTISAMPLE:
		gl_state.multisample_enabled = GL_FALSE;
		break;
	case GL_NORMALIZE:
		gl_state.normalize_enabled = GL_FALSE;
		break;
	case GL_POINT_SMOOTH:
		gl_state.point_smooth_enabled = GL_FALSE;
		break;
	case GL_POINT_SPRITE_OES:
		gl_state.point_sprite_enabled = GL_FALSE;
		break;
	case GL_POLYGON_OFFSET_FILL:
		gl_state.polygon_offset_fill_enabled = GL_FALSE;
		break;
	case GL_RESCALE_NORMAL:
		gl_state.rescale_normal_enabled = GL_FALSE;
		break;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		gl_state.sample_alpha_to_coverage_enabled = GL_FALSE;
		break;
	case GL_SAMPLE_ALPHA_TO_ONE:
		gl_state.sample_alpha_to_one_enabled = GL_FALSE;
		break;
	case GL_SAMPLE_COVERAGE:
		gl_state.sample_coverage_enabled = GL_FALSE;
		break;
	case GL_SCISSOR_TEST:
		gl_state.scissor_test_enabled = GL_FALSE;
		break;
	case GL_STENCIL_TEST:
		gl_state.stencil_test_enabled = GL_FALSE;
		break;
	case GL_TEXTURE_2D:
		gl_state.texture_2d_enabled = GL_FALSE;
		break;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:
		gl_state.clip_plane_enabled[cap - GL_CLIP_PLANE0] = GL_FALSE;
		break;
	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		gl_state.light_enabled[cap - GL_LIGHT0] = GL_FALSE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GL_API void GL_APIENTRY glDisableClientState(GLenum array)
{
	switch (array) {
	case GL_VERTEX_ARRAY:
		gl_state.vertex_array_enabled = GL_FALSE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->vertex_array_enabled = GL_FALSE;
		break;
	case GL_COLOR_ARRAY:
		gl_state.color_array_enabled = GL_FALSE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->color_array_enabled = GL_FALSE;
		break;
	case GL_NORMAL_ARRAY:
		gl_state.normal_array_enabled = GL_FALSE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->normal_array_enabled = GL_FALSE;
		break;
	case GL_TEXTURE_COORD_ARRAY:
		gl_state.texcoord_array_enabled = GL_FALSE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->texcoord_array_enabled = GL_FALSE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GL_API void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	switch (mode) {
	case GL_POINTS:
	case GL_LINE_STRIP:
	case GL_LINE_LOOP:
	case GL_LINES:
	case GL_TRIANGLE_STRIP:
	case GL_TRIANGLE_FAN:
	case GL_TRIANGLES:
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (first < 0 || count < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	/* Software rasterizer not implemented */
}
GL_API void GL_APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type,
				       const void *indices)
{
	switch (mode) {
	case GL_POINTS:
	case GL_LINE_STRIP:
	case GL_LINE_LOOP:
	case GL_LINES:
	case GL_TRIANGLE_STRIP:
	case GL_TRIANGLE_FAN:
	case GL_TRIANGLES:
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (count < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}

	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT) {
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (!indices && gl_state.element_array_buffer_binding == 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}

	const GLubyte *u8_indices = NULL;
	const GLushort *u16_indices = NULL;
	if (gl_state.element_array_buffer_binding != 0) {
		BufferObject *obj =
			find_buffer(gl_state.element_array_buffer_binding);
		if (!obj || !obj->data) {
			glSetError(GL_INVALID_OPERATION);
			return;
		}
		size_t offset = (size_t)indices;
		size_t elem_size = type == GL_UNSIGNED_BYTE ? sizeof(GLubyte) :
							      sizeof(GLushort);
		if (offset + elem_size * (size_t)count > (size_t)obj->size) {
			glSetError(GL_INVALID_OPERATION);
			return;
		}
		if (type == GL_UNSIGNED_BYTE)
			u8_indices = (const GLubyte *)obj->data + offset;
		else
			u16_indices =
				(const GLushort *)((const GLubyte *)obj->data +
						   offset);
	} else {
		if (type == GL_UNSIGNED_BYTE)
			u8_indices = (const GLubyte *)indices;
		else
			u16_indices = (const GLushort *)indices;
	}

	for (GLsizei i = 0; i < count; ++i) {
		GLuint idx = (type == GL_UNSIGNED_BYTE) ?
				     (GLuint)u8_indices[i] :
				     (GLuint)u16_indices[i];
		glDrawArrays(mode, (GLint)idx, 1);
	}
}
GL_API void GL_APIENTRY glEnable(GLenum cap)
{
	switch (cap) {
	case GL_ALPHA_TEST:
		gl_state.alpha_test_enabled = GL_TRUE;
		break;
	case GL_BLEND:
		gl_state.blend_enabled = GL_TRUE;
		break;
	case GL_COLOR_LOGIC_OP:
		gl_state.color_logic_op_enabled = GL_TRUE;
		break;
	case GL_COLOR_MATERIAL:
		gl_state.color_material_enabled = GL_TRUE;
		break;
	case GL_CULL_FACE:
		gl_state.cull_face_enabled = GL_TRUE;
		break;
	case GL_DEPTH_TEST:
		gl_state.depth_test_enabled = GL_TRUE;
		break;
	case GL_DITHER:
		gl_state.dither_enabled = GL_TRUE;
		break;
	case GL_FOG:
		gl_state.fog_enabled = GL_TRUE;
		break;
	case GL_LIGHTING:
		gl_state.lighting_enabled = GL_TRUE;
		break;
	case GL_LINE_SMOOTH:
		gl_state.line_smooth_enabled = GL_TRUE;
		break;
	case GL_MULTISAMPLE:
		gl_state.multisample_enabled = GL_TRUE;
		break;
	case GL_NORMALIZE:
		gl_state.normalize_enabled = GL_TRUE;
		break;
	case GL_POINT_SMOOTH:
		gl_state.point_smooth_enabled = GL_TRUE;
		break;
	case GL_POINT_SPRITE_OES:
		gl_state.point_sprite_enabled = GL_TRUE;
		break;
	case GL_POLYGON_OFFSET_FILL:
		gl_state.polygon_offset_fill_enabled = GL_TRUE;
		break;
	case GL_RESCALE_NORMAL:
		gl_state.rescale_normal_enabled = GL_TRUE;
		break;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		gl_state.sample_alpha_to_coverage_enabled = GL_TRUE;
		break;
	case GL_SAMPLE_ALPHA_TO_ONE:
		gl_state.sample_alpha_to_one_enabled = GL_TRUE;
		break;
	case GL_SAMPLE_COVERAGE:
		gl_state.sample_coverage_enabled = GL_TRUE;
		break;
	case GL_SCISSOR_TEST:
		gl_state.scissor_test_enabled = GL_TRUE;
		break;
	case GL_STENCIL_TEST:
		gl_state.stencil_test_enabled = GL_TRUE;
		break;
	case GL_TEXTURE_2D:
		gl_state.texture_2d_enabled = GL_TRUE;
		break;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:
		gl_state.clip_plane_enabled[cap - GL_CLIP_PLANE0] = GL_TRUE;
		break;
	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		gl_state.light_enabled[cap - GL_LIGHT0] = GL_TRUE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GL_API void GL_APIENTRY glEnableClientState(GLenum array)
{
	switch (array) {
	case GL_VERTEX_ARRAY:
		gl_state.vertex_array_enabled = GL_TRUE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->vertex_array_enabled = GL_TRUE;
		break;
	case GL_COLOR_ARRAY:
		gl_state.color_array_enabled = GL_TRUE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->color_array_enabled = GL_TRUE;
		break;
	case GL_NORMAL_ARRAY:
		gl_state.normal_array_enabled = GL_TRUE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->normal_array_enabled = GL_TRUE;
		break;
	case GL_TEXTURE_COORD_ARRAY:
		gl_state.texcoord_array_enabled = GL_TRUE;
		if (gl_state.bound_vao)
			gl_state.bound_vao->texcoord_array_enabled = GL_TRUE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GL_API void GL_APIENTRY glFinish(void)
{
}
GL_API void GL_APIENTRY glFlush(void)
{
}
GL_API void GL_APIENTRY glFogf(GLenum pname, GLfloat param)
{
	if (pname == GL_FOG_DENSITY && param < 0.0f) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	switch (pname) {
	case GL_FOG_MODE:
		if (param != GL_LINEAR && param != GL_EXP && param != GL_EXP2) {
			glSetError(GL_INVALID_ENUM);
			return;
		}
		gl_state.fog_mode = (GLenum)param;
		break;
	case GL_FOG_DENSITY:
		gl_state.fog_density = param;
		break;
	case GL_FOG_START:
		gl_state.fog_start = param;
		break;
	case GL_FOG_END:
		gl_state.fog_end = param;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		return;
	}
}

GL_API void GL_APIENTRY glFogfv(GLenum pname, const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	switch (pname) {
	case GL_FOG_COLOR:
		gl_state.fog_color[0] = params[0];
		gl_state.fog_color[1] = params[1];
		gl_state.fog_color[2] = params[2];
		gl_state.fog_color[3] = params[3];
		break;
	default:
		glFogf(pname, params[0]);
		break;
	}
}
GL_API void GL_APIENTRY glFrontFace(GLenum mode)
{
	gl_state.front_face = mode;
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
	switch (gl_state.matrix_mode) {
	case GL_PROJECTION:
		mat4_multiply(&result, &gl_state.projection_matrix, &frust);
		mat4_copy(&gl_state.projection_matrix, &result);
		break;
	case GL_MODELVIEW:
		mat4_multiply(&result, &gl_state.modelview_matrix, &frust);
		mat4_copy(&gl_state.modelview_matrix, &result);
		break;
	case GL_TEXTURE:
		mat4_multiply(&result, &gl_state.texture_matrix, &frust);
		mat4_copy(&gl_state.texture_matrix, &result);
		break;
	default:
		break;
	}
}
GL_API void GL_APIENTRY glGenBuffers(GLsizei n, GLuint *buffers)
{
	if (n < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (!buffers)
		return;
	for (GLsizei i = 0; i < n; ++i) {
		if (gl_state.buffer_count >= MAX_BUFFERS) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		BufferObject *obj =
			(BufferObject *)tracked_malloc(sizeof(BufferObject));
		if (!obj) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		obj->id = gl_state.next_buffer_id++;
		obj->size = 0;
		obj->usage = GL_STATIC_DRAW;
		obj->data = NULL;
		gl_state.buffers[gl_state.buffer_count++] = obj;
		buffers[i] = obj->id;
	}
}
GL_API void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
	if (n < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (!textures)
		return;
	for (GLsizei i = 0; i < n; ++i) {
		if (gl_state.texture_count >= MAX_TEXTURES) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		TextureOES *tex = CreateTextureOES(GL_TEXTURE_2D, GL_RGBA, 0, 0,
						   GL_FALSE);
		if (!tex) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		tex->id = gl_state.next_texture_id++;
		gl_state.textures[gl_state.texture_count++] = tex;
		textures[i] = tex->id;
	}
}
GL_API void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean *data)
{
	if (!data)
		return;
	switch (pname) {
	case GL_COLOR_WRITEMASK:
		data[0] = gl_state.color_mask[0];
		data[1] = gl_state.color_mask[1];
		data[2] = gl_state.color_mask[2];
		data[3] = gl_state.color_mask[3];
		break;
	case GL_DEPTH_WRITEMASK:
		*data = gl_state.depth_mask;
		break;
	case GL_LIGHT_MODEL_TWO_SIDE:
		*data = gl_state.light_model_two_side;
		break;
	case GL_SAMPLE_COVERAGE_INVERT:
		*data = gl_state.sample_coverage_invert;
		break;
	default:
		*data = glIsEnabled(pname);
		break;
	}
}
GL_API void GL_APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname,
					       GLint *params)
{
	if (!params)
		return;
	BufferObject *obj = NULL;
	if (target == GL_ARRAY_BUFFER) {
		obj = find_buffer(gl_state.array_buffer_binding);
	} else if (target == GL_ELEMENT_ARRAY_BUFFER) {
		obj = find_buffer(gl_state.element_array_buffer_binding);
	} else {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (!obj) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}
	switch (pname) {
	case GL_BUFFER_SIZE:
		*params = (GLint)obj->size;
		break;
	case GL_BUFFER_USAGE:
		*params = (GLint)obj->usage;
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
GLenum glGetError(void)
{
	return glGetErrorAndClear();
}
GL_API void GL_APIENTRY glGetFloatv(GLenum pname, GLfloat *data)
{
	if (!data)
		return;
	switch (pname) {
	case GL_COLOR_CLEAR_VALUE:
		data[0] = gl_state.clear_color[0];
		data[1] = gl_state.clear_color[1];
		data[2] = gl_state.clear_color[2];
		data[3] = gl_state.clear_color[3];
		break;
	case GL_DEPTH_CLEAR_VALUE:
		data[0] = gl_state.clear_depth;
		break;
	case GL_MODELVIEW_MATRIX:
		memcpy(data, gl_state.modelview_matrix.data,
		       sizeof(GLfloat) * 16);
		break;
	case GL_PROJECTION_MATRIX:
		memcpy(data, gl_state.projection_matrix.data,
		       sizeof(GLfloat) * 16);
		break;
	case GL_TEXTURE_MATRIX:
		memcpy(data, gl_state.texture_matrix.data,
		       sizeof(GLfloat) * 16);
		break;
	case GL_LIGHT_MODEL_AMBIENT:
		memcpy(data, gl_state.light_model_ambient, sizeof(GLfloat) * 4);
		break;
	case GL_DEPTH_RANGE:
		data[0] = gl_state.depth_range_near;
		data[1] = gl_state.depth_range_far;
		break;
	case GL_SAMPLE_COVERAGE_VALUE:
		data[0] = gl_state.sample_coverage_value;
		break;
	case GL_LINE_WIDTH:
		data[0] = gl_state.line_width;
		break;
	default:
		break;
	}
}
GL_API void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *data)
{
	if (!data)
		return;
	switch (pname) {
	case GL_VIEWPORT:
		data[0] = gl_state.viewport[0];
		data[1] = gl_state.viewport[1];
		data[2] = gl_state.viewport[2];
		data[3] = gl_state.viewport[3];
		break;
	case GL_ACTIVE_TEXTURE:
		*data = gl_state.active_texture;
		break;
	case GL_CLIENT_ACTIVE_TEXTURE:
		*data = gl_state.client_active_texture;
		break;
	case GL_ARRAY_BUFFER_BINDING:
		*data = gl_state.array_buffer_binding;
		break;
	case GL_ELEMENT_ARRAY_BUFFER_BINDING:
		*data = gl_state.element_array_buffer_binding;
		break;
	case GL_TEXTURE_BINDING_2D:
		*data = gl_state.bound_texture;
		break;
	case GL_TEXTURE_BINDING_EXTERNAL_OES:
		*data = gl_state.bound_texture_external;
		break;
	case GL_CULL_FACE_MODE:
		*data = gl_state.cull_face_mode;
		break;
	case GL_FRONT_FACE:
		*data = gl_state.front_face;
		break;
	case GL_MATRIX_MODE:
		*data = gl_state.matrix_mode;
		break;
	case GL_BLEND_SRC:
		*data = gl_state.blend_sfactor;
		break;
	case GL_BLEND_DST:
		*data = gl_state.blend_dfactor;
		break;
	case GL_DEPTH_FUNC:
		*data = gl_state.depth_func;
		break;
	case GL_DEPTH_RANGE:
		data[0] = (GLint)gl_state.depth_range_near;
		data[1] = (GLint)gl_state.depth_range_far;
		break;
	case GL_LOGIC_OP_MODE:
		*data = gl_state.logic_op_mode;
		break;
	case GL_STENCIL_FUNC:
		*data = gl_state.stencil_func;
		break;
	case GL_STENCIL_REF:
		*data = gl_state.stencil_ref;
		break;
	case GL_STENCIL_VALUE_MASK:
		*data = (GLint)gl_state.stencil_value_mask;
		break;
	case GL_STENCIL_WRITEMASK:
		*data = (GLint)gl_state.stencil_writemask;
		break;
	case GL_STENCIL_FAIL:
		*data = gl_state.stencil_fail;
		break;
	case GL_STENCIL_PASS_DEPTH_FAIL:
		*data = gl_state.stencil_zfail;
		break;
	case GL_STENCIL_PASS_DEPTH_PASS:
		*data = gl_state.stencil_zpass;
		break;
	case GL_STENCIL_CLEAR_VALUE:
		*data = gl_state.clear_stencil;
		break;
	case GL_LINE_WIDTH:
		*data = (GLint)gl_state.line_width;
		break;
	case GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES:
		for (int i = 0; i < 16; ++i)
			memcpy(&data[i], &gl_state.modelview_matrix.data[i],
			       sizeof(GLfloat));
		break;
	case GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES:
		for (int i = 0; i < 16; ++i)
			memcpy(&data[i], &gl_state.projection_matrix.data[i],
			       sizeof(GLfloat));
		break;
	case GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES:
		for (int i = 0; i < 16; ++i)
			memcpy(&data[i], &gl_state.texture_matrix.data[i],
			       sizeof(GLfloat));
		break;
	case GL_POINT_SIZE_ARRAY_TYPE_OES:
		*data = gl_state.point_size_array_type;
		break;
	case GL_POINT_SIZE_ARRAY_STRIDE_OES:
		*data = gl_state.point_size_array_stride;
		break;
	case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
		*data = gl_state.array_buffer_binding;
		break;
	case GL_MATRIX_INDEX_ARRAY_SIZE_OES:
		*data = gl_state.matrix_index_array_size;
		break;
	case GL_MATRIX_INDEX_ARRAY_TYPE_OES:
		*data = gl_state.matrix_index_array_type;
		break;
	case GL_MATRIX_INDEX_ARRAY_STRIDE_OES:
		*data = gl_state.matrix_index_array_stride;
		break;
	case GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES:
		*data = gl_state.array_buffer_binding;
		break;
	case GL_WEIGHT_ARRAY_SIZE_OES:
		*data = gl_state.weight_array_size;
		break;
	case GL_WEIGHT_ARRAY_TYPE_OES:
		*data = gl_state.weight_array_type;
		break;
	case GL_WEIGHT_ARRAY_STRIDE_OES:
		*data = gl_state.weight_array_stride;
		break;
	case GL_WEIGHT_ARRAY_BUFFER_BINDING_OES:
		*data = gl_state.array_buffer_binding;
		break;
	default:
		/* Unhandled pname */
		break;
	}
}
GL_API void GL_APIENTRY glGetLightfv(GLenum light, GLenum pname,
				     GLfloat *params)
{
	if (!params || !valid_light_enum(light)) {
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
	case GL_SPOT_DIRECTION:
		params[0] = lt->spot_direction[0];
		params[1] = lt->spot_direction[1];
		params[2] = lt->spot_direction[2];
		break;
	case GL_SPOT_EXPONENT:
		params[0] = lt->spot_exponent;
		break;
	case GL_SPOT_CUTOFF:
		params[0] = lt->spot_cutoff;
		break;
	case GL_CONSTANT_ATTENUATION:
		params[0] = lt->constant_attenuation;
		break;
	case GL_LINEAR_ATTENUATION:
		params[0] = lt->linear_attenuation;
		break;
	case GL_QUADRATIC_ATTENUATION:
		params[0] = lt->quadratic_attenuation;
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
	int count;
	Material *mat = select_material(face, &count);
	if (!mat)
		return;
	switch (pname) {
	case GL_AMBIENT:
		memcpy(params, mat[0].ambient, sizeof(GLfloat) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(params, mat[0].diffuse, sizeof(GLfloat) * 4);
		break;
	case GL_SPECULAR:
		memcpy(params, mat[0].specular, sizeof(GLfloat) * 4);
		break;
	case GL_EMISSION:
		memcpy(params, mat[0].emission, sizeof(GLfloat) * 4);
		break;
	case GL_SHININESS:
		params[0] = mat[0].shininess;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GL_API void GL_APIENTRY glGetPointerv(GLenum pname, void **params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	switch (pname) {
	case GL_VERTEX_ARRAY_POINTER:
		*params = (void *)gl_state.vertex_array_pointer;
		break;
	case GL_COLOR_ARRAY_POINTER:
		*params = (void *)gl_state.color_array_pointer;
		break;
	case GL_NORMAL_ARRAY_POINTER:
		*params = (void *)gl_state.normal_array_pointer;
		break;
	case GL_TEXTURE_COORD_ARRAY_POINTER:
		*params = (void *)gl_state.texcoord_array_pointer;
		break;
	case GL_POINT_SIZE_ARRAY_POINTER_OES: {
		GLenum type;
		GLsizei stride;
		*params = (void *)getPointSizePointerOES(&type, &stride);
		break;
	}
	case GL_MATRIX_INDEX_ARRAY_POINTER_OES:
		*params = (void *)gl_state.matrix_index_array_pointer;
		break;
	case GL_WEIGHT_ARRAY_POINTER_OES:
		*params = (void *)gl_state.weight_array_pointer;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
const GLubyte *glGetString(GLenum name)
{
	switch (name) {
	case GL_VENDOR:
		return VENDOR_STRING;
	case GL_RENDERER:
		return RENDERER_STRING;
	case GL_VERSION:
		return VERSION_STRING;
	case GL_EXTENSIONS:
		return renderer_get_extensions();
	default:
		glSetError(GL_INVALID_ENUM);
		return NULL;
	}
}
GL_API void GL_APIENTRY glGetTexEnvfv(GLenum target, GLenum pname,
				      GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (target != GL_TEXTURE_ENV) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	int unit = gl_state.active_texture - GL_TEXTURE0;
	switch (pname) {
	case GL_TEXTURE_ENV_MODE:
		params[0] = (GLfloat)gl_state.tex_env_mode[unit];
		break;
	case GL_TEXTURE_ENV_COLOR:
		memcpy(params, gl_state.tex_env_color[unit],
		       sizeof(GLfloat) * 4);
		break;
	case GL_COMBINE_RGB:
		params[0] = (GLfloat)gl_state.tex_env_combine_rgb[unit];
		break;
	case GL_COMBINE_ALPHA:
		params[0] = (GLfloat)gl_state.tex_env_combine_alpha[unit];
		break;
	case GL_SRC0_RGB:
		params[0] = (GLfloat)gl_state.tex_env_src_rgb[unit][0];
		break;
	case GL_SRC1_RGB:
		params[0] = (GLfloat)gl_state.tex_env_src_rgb[unit][1];
		break;
	case GL_SRC2_RGB:
		params[0] = (GLfloat)gl_state.tex_env_src_rgb[unit][2];
		break;
	case GL_SRC0_ALPHA:
		params[0] = (GLfloat)gl_state.tex_env_src_alpha[unit][0];
		break;
	case GL_SRC1_ALPHA:
		params[0] = (GLfloat)gl_state.tex_env_src_alpha[unit][1];
		break;
	case GL_SRC2_ALPHA:
		params[0] = (GLfloat)gl_state.tex_env_src_alpha[unit][2];
		break;
	case GL_OPERAND0_RGB:
		params[0] = (GLfloat)gl_state.tex_env_operand_rgb[unit][0];
		break;
	case GL_OPERAND1_RGB:
		params[0] = (GLfloat)gl_state.tex_env_operand_rgb[unit][1];
		break;
	case GL_OPERAND2_RGB:
		params[0] = (GLfloat)gl_state.tex_env_operand_rgb[unit][2];
		break;
	case GL_OPERAND0_ALPHA:
		params[0] = (GLfloat)gl_state.tex_env_operand_alpha[unit][0];
		break;
	case GL_OPERAND1_ALPHA:
		params[0] = (GLfloat)gl_state.tex_env_operand_alpha[unit][1];
		break;
	case GL_OPERAND2_ALPHA:
		params[0] = (GLfloat)gl_state.tex_env_operand_alpha[unit][2];
		break;
	case GL_RGB_SCALE:
		params[0] = gl_state.tex_env_rgb_scale[unit];
		break;
	case GL_ALPHA_SCALE:
		params[0] = gl_state.tex_env_alpha_scale[unit];
		break;
	case GL_COORD_REPLACE_OES:
		params[0] = gl_state.tex_env_coord_replace[unit];
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
	GLfloat tmp;
	glGetTexEnvfv(target, pname, &tmp);
	params[0] = (GLint)tmp;
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
	TextureOES *tex =
		(target == GL_TEXTURE_EXTERNAL_OES) ?
			find_texture(gl_state.bound_texture_external) :
			find_texture(gl_state.bound_texture);
	if (!tex) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}
	switch (pname) {
	case GL_TEXTURE_MIN_FILTER:
		params[0] = (GLfloat)tex->min_filter;
		break;
	case GL_TEXTURE_MAG_FILTER:
		params[0] = (GLfloat)tex->mag_filter;
		break;
	case GL_TEXTURE_WRAP_S:
		params[0] = (GLfloat)tex->wrap_s;
		break;
	case GL_TEXTURE_WRAP_T:
		params[0] = (GLfloat)tex->wrap_t;
		break;
	case GL_TEXTURE_CROP_RECT_OES:
		for (int i = 0; i < 4; ++i)
			params[i] = (GLfloat)tex->crop_rect[i];
		break;
	case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
		params[0] = (GLfloat)tex->required_units;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glGetTexParameteriv(GLenum target, GLenum pname,
					    GLint *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (pname == GL_TEXTURE_CROP_RECT_OES) {
		if (target != GL_TEXTURE_2D) {
			glSetError(GL_INVALID_ENUM);
			return;
		}
		TextureOES *tex = find_texture(gl_state.bound_texture);
		if (!tex) {
			glSetError(GL_INVALID_OPERATION);
			return;
		}
		params[0] = tex->crop_rect[0];
		params[1] = tex->crop_rect[1];
		params[2] = tex->crop_rect[2];
		params[3] = tex->crop_rect[3];
	} else {
		if (target != GL_TEXTURE_2D &&
		    target != GL_TEXTURE_EXTERNAL_OES) {
			glSetError(GL_INVALID_ENUM);
			return;
		}
		TextureOES *tex =
			(target == GL_TEXTURE_EXTERNAL_OES) ?
				find_texture(gl_state.bound_texture_external) :
				find_texture(gl_state.bound_texture);
		if (!tex) {
			glSetError(GL_INVALID_OPERATION);
			return;
		}
		switch (pname) {
		case GL_TEXTURE_MIN_FILTER:
			params[0] = tex->min_filter;
			break;
		case GL_TEXTURE_MAG_FILTER:
			params[0] = tex->mag_filter;
			break;
		case GL_TEXTURE_WRAP_S:
			params[0] = tex->wrap_s;
			break;
		case GL_TEXTURE_WRAP_T:
			params[0] = tex->wrap_t;
			break;
		case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
			params[0] = tex->required_units;
			break;
		default: {
			GLfloat tmp;
			glGetTexParameterfv(target, pname, &tmp);
			params[0] = (GLint)tmp;
			break;
		}
		}
	}
}
GL_API void GL_APIENTRY glHint(GLenum target, GLenum mode)
{
	if (mode != GL_FASTEST && mode != GL_NICEST && mode != GL_DONT_CARE) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	switch (target) {
	case GL_FOG_HINT:
		gl_state.fog_hint = mode;
		break;
	case GL_GENERATE_MIPMAP_HINT:
		gl_state.generate_mipmap_hint = mode;
		break;
	case GL_LINE_SMOOTH_HINT:
		gl_state.line_smooth_hint = mode;
		break;
	case GL_PERSPECTIVE_CORRECTION_HINT:
		gl_state.perspective_correction_hint = mode;
		break;
	case GL_POINT_SMOOTH_HINT:
		gl_state.point_smooth_hint = mode;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GLboolean glIsBuffer(GLuint buffer)
{
	return find_buffer(buffer) != NULL;
}
GLboolean glIsEnabled(GLenum cap)
{
	switch (cap) {
	case GL_ALPHA_TEST:
		return gl_state.alpha_test_enabled;
	case GL_BLEND:
		return gl_state.blend_enabled;
	case GL_COLOR_LOGIC_OP:
		return gl_state.color_logic_op_enabled;
	case GL_COLOR_MATERIAL:
		return gl_state.color_material_enabled;
	case GL_CULL_FACE:
		return gl_state.cull_face_enabled;
	case GL_DEPTH_TEST:
		return gl_state.depth_test_enabled;
	case GL_DITHER:
		return gl_state.dither_enabled;
	case GL_FOG:
		return gl_state.fog_enabled;
	case GL_LIGHTING:
		return gl_state.lighting_enabled;
	case GL_LINE_SMOOTH:
		return gl_state.line_smooth_enabled;
	case GL_MULTISAMPLE:
		return gl_state.multisample_enabled;
	case GL_NORMALIZE:
		return gl_state.normalize_enabled;
	case GL_POINT_SMOOTH:
		return gl_state.point_smooth_enabled;
	case GL_POINT_SPRITE_OES:
		return gl_state.point_sprite_enabled;
	case GL_POLYGON_OFFSET_FILL:
		return gl_state.polygon_offset_fill_enabled;
	case GL_RESCALE_NORMAL:
		return gl_state.rescale_normal_enabled;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		return gl_state.sample_alpha_to_coverage_enabled;
	case GL_SAMPLE_ALPHA_TO_ONE:
		return gl_state.sample_alpha_to_one_enabled;
	case GL_SAMPLE_COVERAGE:
		return gl_state.sample_coverage_enabled;
	case GL_SCISSOR_TEST:
		return gl_state.scissor_test_enabled;
	case GL_STENCIL_TEST:
		return gl_state.stencil_test_enabled;
	case GL_TEXTURE_2D:
		return gl_state.texture_2d_enabled;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:
		return gl_state.clip_plane_enabled[cap - GL_CLIP_PLANE0];
	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		return gl_state.light_enabled[cap - GL_LIGHT0];
	default:
		glSetError(GL_INVALID_ENUM);
		return GL_FALSE;
	}
}
GLboolean glIsTexture(GLuint texture)
{
	return find_texture(texture) != NULL;
}
GL_API void GL_APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
	if (!valid_light_enum(light)) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	Light *lt = &gl_state.lights[light - GL_LIGHT0];
	switch (pname) {
	case GL_SPOT_EXPONENT:
		if (param < 0.0f || param > 128.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->spot_exponent = param;
		break;
	case GL_SPOT_CUTOFF:
		if ((param < 0.0f || param > 90.0f) && param != 180.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->spot_cutoff = param;
		break;
	case GL_CONSTANT_ATTENUATION:
		if (param < 0.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->constant_attenuation = param;
		break;
	case GL_LINEAR_ATTENUATION:
		if (param < 0.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->linear_attenuation = param;
		break;
	case GL_QUADRATIC_ATTENUATION:
		if (param < 0.0f) {
			glSetError(GL_INVALID_VALUE);
			return;
		}
		lt->quadratic_attenuation = param;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glLightfv(GLenum light, GLenum pname,
				  const GLfloat *params)
{
	if (!valid_light_enum(light)) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	Light *lt = &gl_state.lights[light - GL_LIGHT0];
	switch (pname) {
	case GL_AMBIENT:
		memcpy(lt->ambient, params, sizeof(GLfloat) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(lt->diffuse, params, sizeof(GLfloat) * 4);
		break;
	case GL_SPECULAR:
		memcpy(lt->specular, params, sizeof(GLfloat) * 4);
		break;
	case GL_POSITION: {
		GLfloat tmp[4];
		mat4_transform_vec4(&gl_state.modelview_matrix, params, tmp);
		memcpy(lt->position, tmp, sizeof(GLfloat) * 4);
		break;
	}
	case GL_SPOT_DIRECTION: {
		GLfloat in[4] = { params[0], params[1], params[2], 0.0f };
		GLfloat tmp[4];
		mat4_transform_vec4(&gl_state.modelview_matrix, in, tmp);
		lt->spot_direction[0] = tmp[0];
		lt->spot_direction[1] = tmp[1];
		lt->spot_direction[2] = tmp[2];
		break;
	}
	case GL_SPOT_EXPONENT:
	case GL_SPOT_CUTOFF:
	case GL_CONSTANT_ATTENUATION:
	case GL_LINEAR_ATTENUATION:
	case GL_QUADRATIC_ATTENUATION:
		glLightf(light, pname, params[0]);
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GL_API void GL_APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
	if (pname == GL_LIGHT_MODEL_TWO_SIDE) {
		gl_state.light_model_two_side = param ? GL_TRUE : GL_FALSE;
	} else {
		glSetError(GL_INVALID_ENUM);
	}
}
GL_API void GL_APIENTRY glLightModelfv(GLenum pname, const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	switch (pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		memcpy(gl_state.light_model_ambient, params,
		       sizeof(GLfloat) * 4);
		break;
	case GL_LIGHT_MODEL_TWO_SIDE:
		gl_state.light_model_two_side = params[0] ? GL_TRUE : GL_FALSE;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GL_API void GL_APIENTRY glLineWidth(GLfloat width)
{
	if (width <= 0.0f) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	gl_state.line_width = width;
}
GL_API void GL_APIENTRY glLoadIdentity(void)
{
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
}
GL_API void GL_APIENTRY glLoadMatrixf(const GLfloat *m)
{
	if (!m)
		return;
	mat4 mat;
	memcpy(mat.data, m, sizeof(GLfloat) * 16);
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		mat4_copy(&gl_state.modelview_matrix, &mat);
		break;
	case GL_PROJECTION:
		mat4_copy(&gl_state.projection_matrix, &mat);
		break;
	case GL_TEXTURE:
		mat4_copy(&gl_state.texture_matrix, &mat);
		break;
	default:
		break;
	}
}
GL_API void GL_APIENTRY glLogicOp(GLenum opcode)
{
	switch (opcode) {
	case GL_CLEAR:
	case GL_AND:
	case GL_AND_REVERSE:
	case GL_COPY:
	case GL_AND_INVERTED:
	case GL_NOOP:
	case GL_XOR:
	case GL_OR:
	case GL_NOR:
	case GL_EQUIV:
	case GL_INVERT:
	case GL_OR_REVERSE:
	case GL_COPY_INVERTED:
	case GL_OR_INVERTED:
	case GL_NAND:
	case GL_SET:
		gl_state.logic_op_mode = opcode;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
static Material *select_material(GLenum face, int *count)
{
	switch (face) {
	case GL_FRONT:
		*count = 1;
		return &gl_state.material[0];
	case GL_BACK:
		*count = 1;
		return &gl_state.material[1];
	case GL_FRONT_AND_BACK:
		*count = 2;
		return gl_state.material;
	default:
		glSetError(GL_INVALID_ENUM);
		return NULL;
	}
}

GL_API void GL_APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	glMaterialfv(face, pname, &param);
}

GL_API void GL_APIENTRY glMaterialfv(GLenum face, GLenum pname,
				     const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	int count;
	Material *mat = select_material(face, &count);
	if (!mat)
		return;
	for (int i = 0; i < count; ++i) {
		switch (pname) {
		case GL_AMBIENT:
			memcpy(mat[i].ambient, params, sizeof(GLfloat) * 4);
			break;
		case GL_DIFFUSE:
			memcpy(mat[i].diffuse, params, sizeof(GLfloat) * 4);
			break;
		case GL_SPECULAR:
			memcpy(mat[i].specular, params, sizeof(GLfloat) * 4);
			break;
		case GL_EMISSION:
			memcpy(mat[i].emission, params, sizeof(GLfloat) * 4);
			break;
		case GL_SHININESS:
			if (params[0] < 0.0f || params[0] > 128.0f) {
				glSetError(GL_INVALID_VALUE);
				return;
			}
			mat[i].shininess = params[0];
			break;
		case GL_AMBIENT_AND_DIFFUSE:
			memcpy(mat[i].ambient, params, sizeof(GLfloat) * 4);
			memcpy(mat[i].diffuse, params, sizeof(GLfloat) * 4);
			break;
		default:
			glSetError(GL_INVALID_ENUM);
			return;
		}
	}
}
GL_API void GL_APIENTRY glMatrixMode(GLenum mode)
{
	switch (mode) {
	case GL_MODELVIEW:
	case GL_PROJECTION:
	case GL_TEXTURE:
		gl_state.matrix_mode = mode;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}
GL_API void GL_APIENTRY glMultMatrixf(const GLfloat *m)
{
	if (!m)
		return;
	mat4 mat, result;
	memcpy(mat.data, m, sizeof(GLfloat) * 16);
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		mat4_multiply(&result, &gl_state.modelview_matrix, &mat);
		mat4_copy(&gl_state.modelview_matrix, &result);
		break;
	case GL_PROJECTION:
		mat4_multiply(&result, &gl_state.projection_matrix, &mat);
		mat4_copy(&gl_state.projection_matrix, &result);
		break;
	case GL_TEXTURE:
		mat4_multiply(&result, &gl_state.texture_matrix, &mat);
		mat4_copy(&gl_state.texture_matrix, &result);
		break;
	default:
		break;
	}
}
GL_API void GL_APIENTRY glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t,
					  GLfloat r, GLfloat q)
{
	if (target < GL_TEXTURE0 || target >= GL_TEXTURE0 + 8) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	GLfloat *coord = gl_state.current_texcoord[target - GL_TEXTURE0];
	coord[0] = s;
	coord[1] = t;
	coord[2] = r;
	coord[3] = q;
}
GL_API void GL_APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	gl_state.current_normal[0] = nx;
	gl_state.current_normal[1] = ny;
	gl_state.current_normal[2] = nz;
}
GL_API void GL_APIENTRY glNormalPointer(GLenum type, GLsizei stride,
					const void *ptr)
{
	gl_state.normal_array_type = type;
	gl_state.normal_array_stride = stride;
	gl_state.normal_array_pointer = ptr;
	if (gl_state.bound_vao) {
		gl_state.bound_vao->normal_array_type = type;
		gl_state.bound_vao->normal_array_stride = stride;
		gl_state.bound_vao->normal_array_pointer = ptr;
	}
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
	switch (gl_state.matrix_mode) {
	case GL_PROJECTION:
		mat4_multiply(&result, &gl_state.projection_matrix, &ortho);
		mat4_copy(&gl_state.projection_matrix, &result);
		break;
	case GL_MODELVIEW:
		mat4_multiply(&result, &gl_state.modelview_matrix, &ortho);
		mat4_copy(&gl_state.modelview_matrix, &result);
		break;
	case GL_TEXTURE:
		mat4_multiply(&result, &gl_state.texture_matrix, &ortho);
		mat4_copy(&gl_state.texture_matrix, &result);
		break;
	default:
		break;
	}
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
GL_API void GL_APIENTRY glPointSize(GLfloat size)
{
	if (size <= 0.0f) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	gl_state.point_size = size;
}
GL_API void GL_APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
	gl_state.polygon_offset_factor = factor;
	gl_state.polygon_offset_units = units;
}
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

static mat4 *stack_for_mode(GLenum mode, GLint **depth, GLint *max_depth)
{
	switch (mode) {
	case GL_MODELVIEW:
		*depth = &gl_state.modelview_stack_depth;
		*max_depth = MODELVIEW_STACK_MAX;
		return gl_state.modelview_stack;
	case GL_PROJECTION:
		*depth = &gl_state.projection_stack_depth;
		*max_depth = PROJECTION_STACK_MAX;
		return gl_state.projection_stack;
	case GL_TEXTURE:
		*depth = &gl_state.texture_stack_depth;
		*max_depth = TEXTURE_STACK_MAX;
		return gl_state.texture_stack;
	default:
		return NULL;
	}
}

GL_API void GL_APIENTRY glPopMatrix(void)
{
	GLint *depth;
	GLint max_depth;
	mat4 *stack = stack_for_mode(gl_state.matrix_mode, &depth, &max_depth);
	if (!stack)
		return;
	if (*depth <= 1) {
		glSetError(GL_STACK_UNDERFLOW);
		return;
	}
	(*depth)--;
	mat4_copy(current_matrix_ptr(), &stack[*depth - 1]);
}

GL_API void GL_APIENTRY glPushMatrix(void)
{
	GLint *depth;
	GLint max_depth;
	mat4 *stack = stack_for_mode(gl_state.matrix_mode, &depth, &max_depth);
	if (!stack)
		return;
	if (*depth >= max_depth) {
		glSetError(GL_STACK_OVERFLOW);
		return;
	}
	mat4_copy(&stack[*depth], current_matrix_ptr());
	(*depth)++;
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
	memset(pixels, 0, (size_t)width * height * 4);
}
GL_API void GL_APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y,
				  GLfloat z)
{
	mat4 rot, result;
	mat4_identity(&rot);
	mat4_rotate_axis(&rot, angle, x, y, z);
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		mat4_multiply(&result, &gl_state.modelview_matrix, &rot);
		mat4_copy(&gl_state.modelview_matrix, &result);
		break;
	case GL_PROJECTION:
		mat4_multiply(&result, &gl_state.projection_matrix, &rot);
		mat4_copy(&gl_state.projection_matrix, &result);
		break;
	case GL_TEXTURE:
		mat4_multiply(&result, &gl_state.texture_matrix, &rot);
		mat4_copy(&gl_state.texture_matrix, &result);
		break;
	default:
		break;
	}
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
GL_API void GL_APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	mat4 scale, result;
	mat4_identity(&scale);
	mat4_scale(&scale, x, y, z);
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		mat4_multiply(&result, &gl_state.modelview_matrix, &scale);
		mat4_copy(&gl_state.modelview_matrix, &result);
		break;
	case GL_PROJECTION:
		mat4_multiply(&result, &gl_state.projection_matrix, &scale);
		mat4_copy(&gl_state.projection_matrix, &result);
		break;
	case GL_TEXTURE:
		mat4_multiply(&result, &gl_state.texture_matrix, &scale);
		mat4_copy(&gl_state.texture_matrix, &result);
		break;
	default:
		break;
	}
}
GL_API void GL_APIENTRY glScissor(GLint x, GLint y, GLsizei width,
				  GLsizei height)
{
	if (width < 0 || height < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	gl_state.scissor_box[0] = x;
	gl_state.scissor_box[1] = y;
	gl_state.scissor_box[2] = width;
	gl_state.scissor_box[3] = height;
}
GL_API void GL_APIENTRY glShadeModel(GLenum mode)
{
	if (mode != GL_FLAT && mode != GL_SMOOTH) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	gl_state.shade_model = mode;
}
GL_API void GL_APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	gl_state.stencil_func = func;
	gl_state.stencil_ref = ref;
	gl_state.stencil_value_mask = mask;
}
GL_API void GL_APIENTRY glStencilMask(GLuint mask)
{
	gl_state.stencil_writemask = mask;
}
GL_API void GL_APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	gl_state.stencil_fail = fail;
	gl_state.stencil_zfail = zfail;
	gl_state.stencil_zpass = zpass;
}
GL_API void GL_APIENTRY glTexCoordPointer(GLint size, GLenum type,
					  GLsizei stride, const void *ptr)
{
	gl_state.texcoord_array_size = size;
	gl_state.texcoord_array_type = type;
	gl_state.texcoord_array_stride = stride;
	gl_state.texcoord_array_pointer = ptr;
	if (gl_state.bound_vao) {
		gl_state.bound_vao->texcoord_array_size = size;
		gl_state.bound_vao->texcoord_array_type = type;
		gl_state.bound_vao->texcoord_array_stride = stride;
		gl_state.bound_vao->texcoord_array_pointer = ptr;
	}
}
GL_API void GL_APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
	if (target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	int unit = gl_state.active_texture - GL_TEXTURE0;
	switch (pname) {
	case GL_TEXTURE_ENV_MODE:
		switch ((GLenum)param) {
		case GL_MODULATE:
		case GL_DECAL:
		case GL_BLEND:
		case GL_ADD:
		case GL_REPLACE:
		case GL_COMBINE:
			gl_state.tex_env_mode[unit] = (GLenum)param;
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
		break;
	}
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
	int unit = gl_state.active_texture - GL_TEXTURE0;
	switch (pname) {
	case GL_TEXTURE_ENV_COLOR:
		memcpy(gl_state.tex_env_color[unit], params,
		       sizeof(GLfloat) * 4);
		break;
	case GL_TEXTURE_ENV_MODE:
	case GL_COMBINE_RGB:
	case GL_COMBINE_ALPHA:
	case GL_SRC0_RGB:
	case GL_SRC1_RGB:
	case GL_SRC2_RGB:
	case GL_SRC0_ALPHA:
	case GL_SRC1_ALPHA:
	case GL_SRC2_ALPHA:
	case GL_OPERAND0_RGB:
	case GL_OPERAND1_RGB:
	case GL_OPERAND2_RGB:
	case GL_OPERAND0_ALPHA:
	case GL_OPERAND1_ALPHA:
	case GL_OPERAND2_ALPHA:
	case GL_RGB_SCALE:
	case GL_ALPHA_SCALE:
	case GL_COORD_REPLACE_OES:
		glTexEnvf(target, pname, params[0]);
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
	glTexEnvf(target, pname, (GLfloat)param);
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
GL_API void GL_APIENTRY glTexImage2D(GLenum target, GLint level,
				     GLint internalformat, GLsizei width,
				     GLsizei height, GLint border,
				     GLenum format, GLenum type,
				     const void *pixels)
{
	if (target != GL_TEXTURE_2D) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (level < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (border != 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (format != GL_ALPHA && format != GL_RGB && format != GL_RGBA &&
	    format != GL_LUMINANCE && format != GL_LUMINANCE_ALPHA) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	GLenum base_format = 0;
	switch ((GLenum)internalformat) {
	case GL_ALPHA:
	case GL_ALPHA8_OES:
		base_format = GL_ALPHA;
		break;
	case GL_LUMINANCE:
	case GL_LUMINANCE8_OES:
		base_format = GL_LUMINANCE;
		break;
	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE4_ALPHA4_OES:
	case GL_LUMINANCE8_ALPHA8_OES:
		base_format = GL_LUMINANCE_ALPHA;
		break;
	case GL_RGB:
	case GL_RGB565_OES:
	case GL_RGB8_OES:
	case GL_RGB10_EXT:
		base_format = GL_RGB;
		break;
	case GL_RGBA:
	case GL_RGBA4_OES:
	case GL_RGB5_A1_OES:
	case GL_RGBA8_OES:
	case GL_RGB10_A2_EXT:
		base_format = GL_RGBA;
		break;
	case GL_DEPTH_COMPONENT:
	case GL_DEPTH_COMPONENT16_OES:
	case GL_DEPTH_COMPONENT24_OES:
	case GL_DEPTH_COMPONENT32_OES:
		base_format = GL_DEPTH_COMPONENT;
		break;
	case GL_DEPTH_STENCIL_OES:
	case GL_DEPTH24_STENCIL8_OES:
		base_format = GL_DEPTH_STENCIL_OES;
		break;
	default:
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (base_format != format) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}
	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT_5_6_5 &&
	    type != GL_UNSIGNED_SHORT_4_4_4_4 &&
	    type != GL_UNSIGNED_SHORT_5_5_5_1) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (type == GL_UNSIGNED_SHORT_5_6_5 && format != GL_RGB) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}
	if ((type == GL_UNSIGNED_SHORT_4_4_4_4 ||
	     type == GL_UNSIGNED_SHORT_5_5_5_1) &&
	    format != GL_RGBA) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}
	if (width < 0 || height < 0 || !is_power_of_two(width) ||
	    !is_power_of_two(height) || width > 1024 || height > 1024) {
		glSetError(GL_INVALID_VALUE);
		return;
	}

	GLuint id = gl_state.bound_texture;
	if (id == 0) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	TextureOES *tex = NULL;
	for (GLuint i = 0; i < gl_state.texture_count; ++i) {
		if (gl_state.textures[i] && gl_state.textures[i]->id == id) {
			tex = gl_state.textures[i];
			break;
		}
	}
	if (!tex) {
		if (gl_state.texture_count >= MAX_TEXTURES) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		tex = CreateTextureOES(target, internalformat, width, height,
				       GL_FALSE);
		if (!tex) {
			glSetError(GL_OUT_OF_MEMORY);
			return;
		}
		tex->id = id;
		gl_state.textures[gl_state.texture_count++] = tex;
	}
	TexImage2DOES(tex, level, internalformat, width, height, format, type,
		      pixels);
}
GL_API void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname,
					GLfloat param)
{
	glTexParameteri(target, pname, (GLint)param);
}

GL_API void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname,
					 const GLfloat *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (pname == GL_TEXTURE_CROP_RECT_OES) {
		if (target != GL_TEXTURE_2D &&
		    target != GL_TEXTURE_EXTERNAL_OES) {
			glSetError(GL_INVALID_ENUM);
			return;
		}
		TextureOES *tex =
			(target == GL_TEXTURE_EXTERNAL_OES) ?
				find_texture(gl_state.bound_texture_external) :
				find_texture(gl_state.bound_texture);
		if (!tex) {
			glSetError(GL_INVALID_OPERATION);
			return;
		}
		tex->crop_rect[0] = (GLint)params[0];
		tex->crop_rect[1] = (GLint)params[1];
		tex->crop_rect[2] = (GLint)params[2];
		tex->crop_rect[3] = (GLint)params[3];
	} else {
		glTexParameterf(target, pname, params[0]);
	}
}

GL_API void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname,
					GLint param)
{
	if (target != GL_TEXTURE_2D) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	TextureOES *tex = find_texture(gl_state.bound_texture);
	if (!tex) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}
	switch (pname) {
	case GL_TEXTURE_MIN_FILTER:
		tex->min_filter = param;
		break;
	case GL_TEXTURE_MAG_FILTER:
		tex->mag_filter = param;
		break;
	case GL_TEXTURE_WRAP_S:
		tex->wrap_s = param;
		break;
	case GL_TEXTURE_WRAP_T:
		tex->wrap_t = param;
		break;
	default:
		glSetError(GL_INVALID_ENUM);
		break;
	}
}

GL_API void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname,
					 const GLint *params)
{
	if (!params) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	if (pname == GL_TEXTURE_CROP_RECT_OES) {
		if (target != GL_TEXTURE_2D) {
			glSetError(GL_INVALID_ENUM);
			return;
		}
		TextureOES *tex = find_texture(gl_state.bound_texture);
		if (!tex) {
			glSetError(GL_INVALID_OPERATION);
			return;
		}
		tex->crop_rect[0] = params[0];
		tex->crop_rect[1] = params[1];
		tex->crop_rect[2] = params[2];
		tex->crop_rect[3] = params[3];
	} else {
		glTexParameteri(target, pname, params[0]);
	}
}
GL_API void GL_APIENTRY glTexSubImage2D(GLenum target, GLint level,
					GLint xoffset, GLint yoffset,
					GLsizei width, GLsizei height,
					GLenum format, GLenum type,
					const void *pixels)
{
	if (target != GL_TEXTURE_2D) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (level < 0 || xoffset < 0 || yoffset < 0 || width < 0 ||
	    height < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	TextureOES *tex = find_texture(gl_state.bound_texture);
	if (!tex) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}
	if (xoffset == 0 && yoffset == 0 && width == tex->width &&
	    height == tex->height) {
		TexImage2DOES(tex, level, tex->internalformat, width, height,
			      format, type, pixels);
	}
}
GL_API void GL_APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	mat4 trans, result;
	mat4_identity(&trans);
	mat4_translate(&trans, x, y, z);
	switch (gl_state.matrix_mode) {
	case GL_MODELVIEW:
		mat4_multiply(&result, &gl_state.modelview_matrix, &trans);
		mat4_copy(&gl_state.modelview_matrix, &result);
		break;
	case GL_PROJECTION:
		mat4_multiply(&result, &gl_state.projection_matrix, &trans);
		mat4_copy(&gl_state.projection_matrix, &result);
		break;
	case GL_TEXTURE:
		mat4_multiply(&result, &gl_state.texture_matrix, &trans);
		mat4_copy(&gl_state.texture_matrix, &result);
		break;
	default:
		break;
	}
}
GL_API void GL_APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride,
					const void *ptr)
{
	gl_state.vertex_array_size = size;
	gl_state.vertex_array_type = type;
	gl_state.vertex_array_stride = stride;
	gl_state.vertex_array_pointer = ptr;
	if (gl_state.bound_vao) {
		gl_state.bound_vao->vertex_array_size = size;
		gl_state.bound_vao->vertex_array_type = type;
		gl_state.bound_vao->vertex_array_stride = stride;
		gl_state.bound_vao->vertex_array_pointer = ptr;
	}
}
GL_API void GL_APIENTRY glViewport(GLint x, GLint y, GLsizei width,
				   GLsizei height)
{
	if (width < 0 || height < 0) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	gl_state.viewport[0] = x;
	gl_state.viewport[1] = y;
	gl_state.viewport[2] = width;
	gl_state.viewport[3] = height;
}

/* Fixed-point variants */
GL_API void GL_APIENTRY glAlphaFuncx(GLenum func, GLfixed ref)
{
	glAlphaFunc(func, FIXED_TO_FLOAT(ref));
}
GL_API void GL_APIENTRY glClearColorx(GLfixed red, GLfixed green, GLfixed blue,
				      GLfixed alpha)
{
	glClearColor(FIXED_TO_FLOAT(red), FIXED_TO_FLOAT(green),
		     FIXED_TO_FLOAT(blue), FIXED_TO_FLOAT(alpha));
}
GL_API void GL_APIENTRY glClearDepthx(GLfixed depth)
{
	glClearDepthf(FIXED_TO_FLOAT(depth));
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
GL_API void GL_APIENTRY glScalex(GLfixed x, GLfixed y, GLfixed z)
{
	glScalef(FIXED_TO_FLOAT(x), FIXED_TO_FLOAT(y), FIXED_TO_FLOAT(z));
}
GL_API void GL_APIENTRY glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
	glTranslatef(FIXED_TO_FLOAT(x), FIXED_TO_FLOAT(y), FIXED_TO_FLOAT(z));
}
