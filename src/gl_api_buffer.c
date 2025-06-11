#include "gl_state.h"
#include "gl_memory_tracker.h"
#include "gl_errors.h"
#include "gl_utils.h"
#include <GLES/gl.h>
#include <string.h>

static BufferObject *find_buffer(GLuint id)
{
	for (GLint i = 0; i < gl_state.buffer_count; ++i) {
		if (gl_state.buffers[i]->id == id)
			return gl_state.buffers[i];
	}
	return NULL;
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

GL_API void GL_APIENTRY glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
	if (n < 0)
		return;
	if (!buffers)
		return;
	for (GLsizei i = 0; i < n; ++i) {
		BufferObject *obj = find_buffer(buffers[i]);
		if (obj) {
			if (obj->data)
				tracked_free(obj->data, obj->size);
			for (GLint j = 0; j < gl_state.buffer_count; ++j) {
				if (gl_state.buffers[j] == obj) {
					memmove(&gl_state.buffers[j],
						&gl_state.buffers[j + 1],
						sizeof(BufferObject *) *
							(gl_state.buffer_count -
							 j - 1));
					gl_state.buffer_count--;
					break;
				}
			}
			tracked_free(obj, sizeof(BufferObject));
		}
	}
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
