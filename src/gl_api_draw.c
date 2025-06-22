#include "gl_state.h"
#include "gl_context.h"
#include "gl_memory_tracker.h"
#include "gl_init.h"
#include "gl_errors.h"
#include "command_buffer.h"
#include "pool.h"
#include "pipeline/gl_vertex.h"
#include "pipeline/gl_raster.h"
#include "matrix_utils.h"
#include "gl_thread.h"
#include "function_profile.h"
#include <string.h>
#include <GLES/gl.h>
#include <stdatomic.h>

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
	PROFILE_START("glDrawArrays");
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
		PROFILE_END("glDrawArrays");
		return;
	}

	if (first < 0 || count < 0) {
		glSetError(GL_INVALID_VALUE);
		PROFILE_END("glDrawArrays");
		return;
	}
	RenderContext *ctx = GetCurrentContext();
	unsigned bver = atomic_load(&ctx->blend.version);
	unsigned dver = atomic_load(&ctx->version_depth);
	unsigned fver = atomic_load(&ctx->fog.version);
	unsigned cver = atomic_load(&ctx->version_cull);
	if (bver != ctx->validated_blend_version ||
	    dver != ctx->validated_depth_version ||
	    fver != ctx->validated_fog_version ||
	    cver != ctx->validated_cull_version) {
		ctx->validated_blend_version = bver;
		ctx->validated_depth_version = dver;
		ctx->validated_fog_version = fver;
		ctx->validated_cull_version = cver;
	}
	if (!ctx->vertex_array.enabled) {
		glSetError(GL_INVALID_OPERATION);
		PROFILE_END("glDrawArrays");
		return;
	}

	Framebuffer *fb = NULL;
	if (gl_state.bound_framebuffer)
		fb = gl_state.bound_framebuffer->fb;
	if (!fb)
		fb = GL_get_default_framebuffer();
	if (!fb) {
		PROFILE_END("glDrawArrays");
		return;
	}

	GLsizei vstride =
		ctx->vertex_array.stride ?
			ctx->vertex_array.stride :
			(GLsizei)(ctx->vertex_array.size * sizeof(GLfloat));
	GLsizei nstride = ctx->normal_array.stride ?
				  ctx->normal_array.stride :
				  (GLsizei)(3 * sizeof(GLfloat));
	GLsizei cstride = ctx->color_array.stride ?
				  ctx->color_array.stride :
				  (GLsizei)(ctx->color_array.size *
					    (ctx->color_array.type == GL_FLOAT ?
						     sizeof(GLfloat) :
						     sizeof(GLubyte)));
	GLsizei tstride =
		ctx->texcoord_array.stride ?
			ctx->texcoord_array.stride :
			(GLsizei)(ctx->texcoord_array.size * sizeof(GLfloat));

	const uint8_t *vptr = (const uint8_t *)ctx->vertex_array.pointer;
	const uint8_t *nptr = (const uint8_t *)ctx->normal_array.pointer;
	const uint8_t *cptr = (const uint8_t *)ctx->color_array.pointer;
	const uint8_t *tptr = (const uint8_t *)ctx->texcoord_array.pointer;

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

	if (mode == GL_POINTS) {
		mat4 mvp;
		mat4_multiply(&mvp, &ctx->projection_matrix,
			      &ctx->modelview_matrix);
		for (GLint i = 0; i < count; ++i) {
			GLint idx = first + i;
			const GLfloat *vp =
				(const GLfloat *)(vptr + (size_t)idx * vstride);
			Vertex src = { 0 };
			src.x = vp[0];
			src.y = ctx->vertex_array.size > 1 ? vp[1] : 0.0f;
			src.z = ctx->vertex_array.size > 2 ? vp[2] : 0.0f;
			src.w = ctx->vertex_array.size > 3 ? vp[3] : 1.0f;
			for (int k = 0; k < 3; ++k)
				src.normal[k] = ctx->current_normal[k];
			for (int k = 0; k < 4; ++k)
				src.color[k] = ctx->current_color[k];
			for (int k = 0; k < 4; ++k)
				src.texcoord[k] = ctx->current_texcoord[0][k];
			src.point_size = gl_state.point_size;
			if (gl_state.point_size_array_pointer) {
				const uint8_t *pptr =
					(const uint8_t *)gl_state
						.point_size_array_pointer;
				if (gl_state.point_size_array_stride)
					pptr += (size_t)idx *
						gl_state.point_size_array_stride;
				if (gl_state.point_size_array_type == GL_FLOAT)
					src.point_size = *(const GLfloat *)pptr;
			}
			Vertex dst;
			pipeline_transform_vertex(&dst, &src, &mvp, NULL,
						  gl_state.viewport);
			pipeline_rasterize_point(&dst, src.point_size,
						 gl_state.viewport, fb);
		}
		return;
	}

	for (GLint i = 0; i + 2 < count; i += 3) {
		VertexJob *job = vertex_job_acquire();
		if (!job)
			return;
		memcpy(job->viewport, gl_state.viewport, sizeof(job->viewport));
		for (int j = 0; j < 3; ++j) {
			GLint idx = first + i + j;
			const GLfloat *vp =
				(const GLfloat *)(vptr + (size_t)idx * vstride);
			Vertex *dst = &job->in[j];
			dst->x = vp[0];
			dst->y = ctx->vertex_array.size > 1 ? vp[1] : 0.0f;
			dst->z = ctx->vertex_array.size > 2 ? vp[2] : 0.0f;
			dst->w = ctx->vertex_array.size > 3 ? vp[3] : 1.0f;
			if (ctx->normal_array.enabled) {
				const GLfloat *np =
					(const GLfloat *)(nptr +
							  (size_t)idx *
								  nstride);
				dst->normal[0] = np[0];
				dst->normal[1] = np[1];
				dst->normal[2] = np[2];
			} else {
				dst->normal[0] = ctx->current_normal[0];
				dst->normal[1] = ctx->current_normal[1];
				dst->normal[2] = ctx->current_normal[2];
			}
			if (ctx->color_array.enabled) {
				if (ctx->color_array.type == GL_FLOAT) {
					const GLfloat *cp =
						(const GLfloat
							 *)(cptr +
							    (size_t)idx *
								    cstride);
					for (int k = 0;
					     k < ctx->color_array.size; ++k)
						dst->color[k] = cp[k];
				} else {
					const GLubyte *cp =
						cptr + (size_t)idx * cstride;
					for (int k = 0;
					     k < ctx->color_array.size; ++k)
						dst->color[k] = cp[k] / 255.0f;
				}
				if (ctx->color_array.size == 3)
					dst->color[3] = 1.0f;
			} else {
				for (int k = 0; k < 4; ++k)
					dst->color[k] = ctx->current_color[k];
			}
			if (ctx->texcoord_array.enabled) {
				const GLfloat *tp =
					(const GLfloat *)(tptr +
							  (size_t)idx *
								  tstride);
				for (int k = 0; k < ctx->texcoord_array.size;
				     ++k)
					dst->texcoord[k] = tp[k];
				for (int k = ctx->texcoord_array.size; k < 4;
				     ++k)
					dst->texcoord[k] = k == 3 ? 1.0f : 0.0f;
			} else {
				for (int k = 0; k < 4; ++k)
					dst->texcoord[k] =
						ctx->current_texcoord[0][k];
			}
		}
		job->fb = fb;
		command_buffer_record_task(process_vertex_job, job,
					   STAGE_VERTEX);
	}
}

GL_API void GL_APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type,
				       const void *indices)
{
	PROFILE_START("glDrawElements");
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
		PROFILE_END("glDrawElements");
		return;
	}

	if (count < 0) {
		glSetError(GL_INVALID_VALUE);
		PROFILE_END("glDrawElements");
		return;
	}

	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT) {
		glSetError(GL_INVALID_ENUM);
		PROFILE_END("glDrawElements");
		return;
	}

	if (!indices && gl_state.element_array_buffer_binding == 0) {
		glSetError(GL_INVALID_VALUE);
		PROFILE_END("glDrawElements");
		return;
	}

	const GLubyte *u8_indices = NULL;
	const GLushort *u16_indices = NULL;
	if (gl_state.element_array_buffer_binding != 0) {
		BufferObject *obj =
			find_buffer(gl_state.element_array_buffer_binding);
		if (!obj || !obj->data) {
			glSetError(GL_INVALID_OPERATION);
			PROFILE_END("glDrawElements");
			return;
		}
		size_t offset = (size_t)indices;
		size_t elem_size = type == GL_UNSIGNED_BYTE ? sizeof(GLubyte) :
							      sizeof(GLushort);
		if (offset + elem_size * (size_t)count > (size_t)obj->size) {
			glSetError(GL_INVALID_OPERATION);
			PROFILE_END("glDrawElements");
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

	RenderContext *ctx = GetCurrentContext();
	if (!ctx->vertex_array.enabled) {
		glSetError(GL_INVALID_OPERATION);
		PROFILE_END("glDrawElements");
		return;
	}

	GLsizei vstride =
		ctx->vertex_array.stride ?
			ctx->vertex_array.stride :
			(GLsizei)(ctx->vertex_array.size * sizeof(GLfloat));
	GLsizei nstride = ctx->normal_array.stride ?
				  ctx->normal_array.stride :
				  (GLsizei)(3 * sizeof(GLfloat));
	GLsizei cstride = ctx->color_array.stride ?
				  ctx->color_array.stride :
				  (GLsizei)(ctx->color_array.size *
					    (ctx->color_array.type == GL_FLOAT ?
						     sizeof(GLfloat) :
						     sizeof(GLubyte)));
	GLsizei tstride =
		ctx->texcoord_array.stride ?
			ctx->texcoord_array.stride :
			(GLsizei)(ctx->texcoord_array.size * sizeof(GLfloat));

	const uint8_t *vptr = (const uint8_t *)ctx->vertex_array.pointer;
	const uint8_t *nptr = (const uint8_t *)ctx->normal_array.pointer;
	const uint8_t *cptr = (const uint8_t *)ctx->color_array.pointer;
	const uint8_t *tptr = (const uint8_t *)ctx->texcoord_array.pointer;

	BufferObject *array_obj = NULL;
	if (gl_state.array_buffer_binding) {
		array_obj = find_buffer(gl_state.array_buffer_binding);
		if (!array_obj || !array_obj->data) {
			glSetError(GL_INVALID_OPERATION);
			PROFILE_END("glDrawElements");
			return;
		}
		GLuint max_idx = 0;
		for (GLsizei i = 0; i < count; ++i) {
			GLuint idx = type == GL_UNSIGNED_BYTE ?
					     (GLuint)u8_indices[i] :
					     (GLuint)u16_indices[i];
			if (idx > max_idx)
				max_idx = idx;
		}
		size_t needed = (size_t)ctx->vertex_array.pointer +
				(size_t)(max_idx + 1) * vstride;
		if (needed > (size_t)array_obj->size) {
			glSetError(GL_INVALID_OPERATION);
			PROFILE_END("glDrawElements");
			return;
		}

		vptr = (const uint8_t *)array_obj->data + (size_t)vptr;
		nptr = (const uint8_t *)array_obj->data + (size_t)nptr;
		cptr = (const uint8_t *)array_obj->data + (size_t)cptr;
		tptr = (const uint8_t *)array_obj->data + (size_t)tptr;
	}

	Framebuffer *fb = NULL;
	if (gl_state.bound_framebuffer)
		fb = gl_state.bound_framebuffer->fb;
	if (!fb)
		fb = GL_get_default_framebuffer();
	if (!fb) {
		PROFILE_END("glDrawElements");
		return;
	}

	if (mode == GL_POINTS) {
		mat4 mvp;
		mat4_multiply(&mvp, &ctx->projection_matrix,
			      &ctx->modelview_matrix);
		for (GLint i = 0; i < count; ++i) {
			GLuint idx = type == GL_UNSIGNED_BYTE ?
					     (GLuint)u8_indices[i] :
					     (GLuint)u16_indices[i];
			const GLfloat *vp =
				(const GLfloat *)(vptr + (size_t)idx * vstride);
			Vertex src = { 0 };
			src.x = vp[0];
			src.y = ctx->vertex_array.size > 1 ? vp[1] : 0.0f;
			src.z = ctx->vertex_array.size > 2 ? vp[2] : 0.0f;
			src.w = ctx->vertex_array.size > 3 ? vp[3] : 1.0f;
			for (int k = 0; k < 3; ++k)
				src.normal[k] = ctx->current_normal[k];
			for (int k = 0; k < 4; ++k)
				src.color[k] = ctx->current_color[k];
			for (int k = 0; k < 4; ++k)
				src.texcoord[k] = ctx->current_texcoord[0][k];
			src.point_size = gl_state.point_size;
			if (gl_state.point_size_array_pointer) {
				const uint8_t *pptr =
					(const uint8_t *)gl_state
						.point_size_array_pointer;
				if (gl_state.point_size_array_stride)
					pptr += (size_t)idx *
						gl_state.point_size_array_stride;
				if (gl_state.point_size_array_type == GL_FLOAT)
					src.point_size = *(const GLfloat *)pptr;
			}
			Vertex dst;
			pipeline_transform_vertex(&dst, &src, &mvp, NULL,
						  gl_state.viewport);
			pipeline_rasterize_point(&dst, src.point_size,
						 gl_state.viewport, fb);
		}
		PROFILE_END("glDrawElements");
		return;
	}

	for (GLsizei i = 0; i + 2 < count; i += 3) {
		VertexJob *job = vertex_job_acquire();
		if (!job) {
			PROFILE_END("glDrawElements");
			return;
		}
		memcpy(job->viewport, gl_state.viewport, sizeof(job->viewport));
		for (int j = 0; j < 3; ++j) {
			GLuint idx = type == GL_UNSIGNED_BYTE ?
					     (GLuint)u8_indices[i + j] :
					     (GLuint)u16_indices[i + j];
			const GLfloat *vp =
				(const GLfloat *)(vptr + (size_t)idx * vstride);
			Vertex *dst = &job->in[j];
			dst->x = vp[0];
			dst->y = ctx->vertex_array.size > 1 ? vp[1] : 0.0f;
			dst->z = ctx->vertex_array.size > 2 ? vp[2] : 0.0f;
			dst->w = ctx->vertex_array.size > 3 ? vp[3] : 1.0f;
			if (ctx->normal_array.enabled) {
				const GLfloat *np =
					(const GLfloat *)(nptr +
							  (size_t)idx *
								  nstride);
				dst->normal[0] = np[0];
				dst->normal[1] = np[1];
				dst->normal[2] = np[2];
			} else {
				dst->normal[0] = ctx->current_normal[0];
				dst->normal[1] = ctx->current_normal[1];
				dst->normal[2] = ctx->current_normal[2];
			}
			if (ctx->color_array.enabled) {
				if (ctx->color_array.type == GL_FLOAT) {
					const GLfloat *cp =
						(const GLfloat
							 *)(cptr +
							    (size_t)idx *
								    cstride);
					for (int k = 0;
					     k < ctx->color_array.size; ++k)
						dst->color[k] = cp[k];
				} else {
					const GLubyte *cp =
						cptr + (size_t)idx * cstride;
					for (int k = 0;
					     k < ctx->color_array.size; ++k)
						dst->color[k] = cp[k] / 255.0f;
				}
				if (ctx->color_array.size == 3)
					dst->color[3] = 1.0f;
			} else {
				for (int k = 0; k < 4; ++k)
					dst->color[k] = ctx->current_color[k];
			}
			if (ctx->texcoord_array.enabled) {
				const GLfloat *tp =
					(const GLfloat *)(tptr +
							  (size_t)idx *
								  tstride);
				for (int k = 0; k < ctx->texcoord_array.size;
				     ++k)
					dst->texcoord[k] = tp[k];
				for (int k = ctx->texcoord_array.size; k < 4;
				     ++k)
					dst->texcoord[k] = k == 3 ? 1.0f : 0.0f;
			} else {
				for (int k = 0; k < 4; ++k)
					dst->texcoord[k] =
						ctx->current_texcoord[0][k];
			}
		}
		job->fb = fb;
		command_buffer_record_task(process_vertex_job, job,
					   STAGE_VERTEX);
	}
	PROFILE_END("glDrawElements");
}
