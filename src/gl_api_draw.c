#include "gl_state.h"
#include "gl_context.h"
#include "gl_memory_tracker.h"
#include "gl_init.h"
#include "pipeline/gl_vertex.h"
#include <GLES/gl.h>

/* Helper from gl_functions.c */
static BufferObject *find_buffer(GLuint id)
{
	for (GLint i = 0; i < gl_state.buffer_count; ++i) {
		if (gl_state.buffers[i]->id == id)
			return gl_state.buffers[i];
	}
	return NULL;
}

/* Draw commands separated from gl_functions for clarity. */

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
	if (!gl_state.vertex_array_enabled) {
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	Framebuffer *fb = NULL;
	if (gl_state.bound_framebuffer)
		fb = gl_state.bound_framebuffer->fb;
	if (!fb)
		fb = GL_get_default_framebuffer();
	if (!fb)
		return;

	GLsizei vstride = gl_state.vertex_array_stride ?
				  gl_state.vertex_array_stride :
				  gl_state.vertex_array_size * sizeof(GLfloat);
	GLsizei nstride = gl_state.normal_array_stride ?
				  gl_state.normal_array_stride :
				  3 * sizeof(GLfloat);
	GLsizei cstride =
		gl_state.color_array_stride ?
			gl_state.color_array_stride :
			gl_state.color_array_size *
				(gl_state.color_array_type == GL_FLOAT ?
					 sizeof(GLfloat) :
					 sizeof(GLubyte));
	GLsizei tstride =
		gl_state.texcoord_array_stride ?
			gl_state.texcoord_array_stride :
			gl_state.texcoord_array_size * sizeof(GLfloat);

	const uint8_t *vptr = (const uint8_t *)gl_state.vertex_array_pointer;
	const uint8_t *nptr = (const uint8_t *)gl_state.normal_array_pointer;
	const uint8_t *cptr = (const uint8_t *)gl_state.color_array_pointer;
	const uint8_t *tptr = (const uint8_t *)gl_state.texcoord_array_pointer;

	if (gl_state.array_buffer_binding) {
		BufferObject *obj = find_buffer(gl_state.array_buffer_binding);
		if (!obj || !obj->data) {
			glSetError(GL_INVALID_OPERATION);
			return;
		}
		vptr = (const uint8_t *)obj->data + (size_t)vptr;
		nptr = (const uint8_t *)obj->data + (size_t)nptr;
		cptr = (const uint8_t *)obj->data + (size_t)cptr;
		tptr = (const uint8_t *)obj->data + (size_t)tptr;
	}

	for (GLint i = 0; i + 2 < count; i += 3) {
		VertexJob *job = MT_ALLOC(sizeof(VertexJob), STAGE_VERTEX);
		if (!job)
			return;
		for (int j = 0; j < 3; ++j) {
			GLint idx = first + i + j;
			const GLfloat *vp =
				(const GLfloat *)(vptr + (size_t)idx * vstride);
			Vertex *dst = &job->in[j];
			dst->x = vp[0];
			dst->y = gl_state.vertex_array_size > 1 ? vp[1] : 0.0f;
			dst->z = gl_state.vertex_array_size > 2 ? vp[2] : 0.0f;
			dst->w = gl_state.vertex_array_size > 3 ? vp[3] : 1.0f;
			if (gl_state.normal_array_enabled) {
				const GLfloat *np =
					(const GLfloat *)(nptr +
							  (size_t)idx *
								  nstride);
				dst->normal[0] = np[0];
				dst->normal[1] = np[1];
				dst->normal[2] = np[2];
			} else {
				dst->normal[0] = gl_state.current_normal[0];
				dst->normal[1] = gl_state.current_normal[1];
				dst->normal[2] = gl_state.current_normal[2];
			}
			if (gl_state.color_array_enabled) {
				if (gl_state.color_array_type == GL_FLOAT) {
					const GLfloat *cp =
						(const GLfloat
							 *)(cptr +
							    (size_t)idx *
								    cstride);
					for (int k = 0;
					     k < gl_state.color_array_size; ++k)
						dst->color[k] = cp[k];
				} else {
					const GLubyte *cp =
						cptr + (size_t)idx * cstride;
					for (int k = 0;
					     k < gl_state.color_array_size; ++k)
						dst->color[k] = cp[k] / 255.0f;
				}
				if (gl_state.color_array_size == 3)
					dst->color[3] = 1.0f;
			} else {
				for (int k = 0; k < 4; ++k)
					dst->color[k] =
						gl_state.current_color[k];
			}
			if (gl_state.texcoord_array_enabled) {
				const GLfloat *tp =
					(const GLfloat *)(tptr +
							  (size_t)idx *
								  tstride);
				for (int k = 0;
				     k < gl_state.texcoord_array_size; ++k)
					dst->texcoord[k] = tp[k];
				for (int k = gl_state.texcoord_array_size;
				     k < 4; ++k)
					dst->texcoord[k] = k == 3 ? 1.0f : 0.0f;
			} else {
				for (int k = 0; k < 4; ++k)
					dst->texcoord[k] =
						gl_state.current_texcoord[0][k];
			}
		}
		job->fb = fb;
		thread_pool_submit(process_vertex_job, job, STAGE_VERTEX);
	}
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
