/* gl_framebuffer_object.c */

#include "gl_api_fbo.h"
#include "gl_state.h"
#include "gl_types.h"
#include "gl_errors.h"
#include "gl_utils.h"
#include "gl_context.h"
#include "gl_ext_common.h"
EXT_REGISTER("GL_OES_framebuffer_object")
__attribute__((used)) int ext_link_dummy_OES_framebuffer_object = 0;
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include <GLES/gl.h> // Core OpenGL ES 1.1
#include <GLES/glext.h> // For GL_OES_framebuffer_object extension
#include <stddef.h> // For size_t
#include <stdlib.h> // For malloc and free
#include <string.h> // For memset

/* Assume GLState is a global or accessible structure */
extern GLState gl_state;

/* Helper function to calculate maximum mipmap level */
static GLint level_max(GLsizei width, GLsizei height);

/* Helper function to create a new renderbuffer */
static RenderbufferOES *create_renderbuffer(GLenum internalformat,
					    GLsizei width, GLsizei height)
{
	RenderbufferOES *rb =
		(RenderbufferOES *)tracked_malloc(sizeof(RenderbufferOES));
	if (!rb) {
		LOG_ERROR("Failed to allocate memory for RenderbufferOES.");
		glSetError(GL_OUT_OF_MEMORY);
		return NULL;
	}
	rb->id = gl_state.next_renderbuffer_id++;
	rb->internalformat = internalformat;
	rb->width = width;
	rb->height = height;
	/* Initialize size components based on internalformat */
	switch (internalformat) {
	case GL_RGBA4_OES:
		rb->red_size = 4;
		rb->green_size = 4;
		rb->blue_size = 4;
		rb->alpha_size = 4;
		rb->depth_size = 0;
		rb->stencil_size = 0;
		break;
	case GL_RGB5_A1_OES:
		rb->red_size = 5;
		rb->green_size = 5;
		rb->blue_size = 5;
		rb->alpha_size = 1;
		rb->depth_size = 0;
		rb->stencil_size = 0;
		break;
	case GL_RGB565_OES:
		rb->red_size = 5;
		rb->green_size = 6;
		rb->blue_size = 5;
		rb->alpha_size = 0;
		rb->depth_size = 0;
		rb->stencil_size = 0;
		break;
	case GL_RGBA8_OES:
		rb->red_size = 8;
		rb->green_size = 8;
		rb->blue_size = 8;
		rb->alpha_size = 8;
		rb->depth_size = 0;
		rb->stencil_size = 0;
		break;
	case GL_DEPTH_COMPONENT24_OES:
		rb->red_size = 0;
		rb->green_size = 0;
		rb->blue_size = 0;
		rb->alpha_size = 0;
		rb->depth_size = 24;
		rb->stencil_size = 0;
		break;
	case GL_DEPTH_COMPONENT32_OES:
		rb->red_size = 0;
		rb->green_size = 0;
		rb->blue_size = 0;
		rb->alpha_size = 0;
		rb->depth_size = 32;
		rb->stencil_size = 0;
		break;
	case GL_DEPTH24_STENCIL8_OES:
		rb->red_size = 0;
		rb->green_size = 0;
		rb->blue_size = 0;
		rb->alpha_size = 0;
		rb->depth_size = 24;
		rb->stencil_size = 8;
		break;
	case GL_DEPTH_COMPONENT16_OES:
		rb->red_size = 0;
		rb->green_size = 0;
		rb->blue_size = 0;
		rb->alpha_size = 0;
		rb->depth_size = 16;
		rb->stencil_size = 0;
		break;
	default:
		LOG_WARN("Unsupported internalformat 0x%X for RenderbufferOES.",
			 internalformat);
		rb->red_size = rb->green_size = rb->blue_size = rb->alpha_size =
			rb->depth_size = rb->stencil_size = 0;
		break;
	}
	return rb;
}

/* Helper function to create a new framebuffer */
static FramebufferOES *create_framebuffer(void)
{
	FramebufferOES *fb =
		(FramebufferOES *)tracked_malloc(sizeof(FramebufferOES));
	if (!fb) {
		LOG_ERROR("Failed to allocate memory for FramebufferOES.");
		glSetError(GL_OUT_OF_MEMORY);
		return NULL;
	}
	fb->id = gl_state.next_framebuffer_id++;
	/* Initialize attachments */
	fb->color_attachment.type = ATTACHMENT_NONE;
	fb->color_attachment.attachment.renderbuffer = NULL;
	fb->color_attachment.attachment.texture = NULL;

	fb->depth_attachment.type = ATTACHMENT_NONE;
	fb->depth_attachment.attachment.renderbuffer = NULL;
	fb->depth_attachment.attachment.texture = NULL;

	fb->stencil_attachment.type = ATTACHMENT_NONE;
	fb->stencil_attachment.attachment.renderbuffer = NULL;
	fb->stencil_attachment.attachment.texture = NULL;

	fb->fb = NULL;

	return fb;
}

/* Implementation of glIsRenderbufferOES */
GLboolean GL_APIENTRY glIsRenderbufferOES(GLuint renderbuffer)
{
	for (GLint i = 0; i < gl_state.renderbuffer_count; ++i) {
		if (gl_state.renderbuffers[i]->id == renderbuffer) {
			LOG_DEBUG(
				"glIsRenderbufferOES: Renderbuffer ID %u exists.",
				renderbuffer);
			return GL_TRUE;
		}
	}
	LOG_DEBUG("glIsRenderbufferOES: Renderbuffer ID %u does not exist.",
		  renderbuffer);
	return GL_FALSE;
}

/* Implementation of glBindRenderbufferOES */
GL_API void GL_APIENTRY glBindRenderbufferOES(GLenum target,
					      GLuint renderbuffer)
{
	if (target != GL_RENDERBUFFER_OES) {
		LOG_ERROR("glBindRenderbufferOES: Invalid target 0x%X.",
			  target);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (renderbuffer == 0) {
		gl_state.bound_renderbuffer = NULL;
		LOG_DEBUG("glBindRenderbufferOES: Unbound renderbuffer.");
		return;
	}

	for (GLint i = 0; i < gl_state.renderbuffer_count; ++i) {
		if (gl_state.renderbuffers[i]->id == renderbuffer) {
			gl_state.bound_renderbuffer = gl_state.renderbuffers[i];
			LOG_DEBUG(
				"glBindRenderbufferOES: Bound renderbuffer ID %u.",
				renderbuffer);
			return;
		}
	}

	LOG_WARN("glBindRenderbufferOES: Renderbuffer ID %u not found.",
		 renderbuffer);
	glSetError(GL_INVALID_VALUE);
}

/* Implementation of glDeleteRenderbuffersOES */
GL_API void GL_APIENTRY glDeleteRenderbuffersOES(GLsizei n,
						 const GLuint *renderbuffers)
{
	for (GLsizei i = 0; i < n; ++i) {
		GLuint rb_id = renderbuffers[i];
		int index = -1;
		for (GLint j = 0; j < gl_state.renderbuffer_count; ++j) {
			if (gl_state.renderbuffers[j]->id == rb_id) {
				index = j;
				break;
			}
		}

		if (index != -1) {
			RenderbufferOES *rb = gl_state.renderbuffers[index];
			/* If the renderbuffer is bound, unbind it */
			if (gl_state.bound_renderbuffer == rb) {
				gl_state.bound_renderbuffer = NULL;
			}
			/* Detach from any framebuffer */
			for (GLint fb = 0; fb < gl_state.framebuffer_count;
			     ++fb) {
				FramebufferOES *framebuffer =
					gl_state.framebuffers[fb];
				if (framebuffer->color_attachment.type ==
					    ATTACHMENT_RENDERBUFFER &&
				    framebuffer->color_attachment.attachment
						    .renderbuffer == rb) {
					framebuffer->color_attachment.type =
						ATTACHMENT_NONE;
					framebuffer->color_attachment.attachment
						.renderbuffer = NULL;
				}
				if (framebuffer->depth_attachment.type ==
					    ATTACHMENT_RENDERBUFFER &&
				    framebuffer->depth_attachment.attachment
						    .renderbuffer == rb) {
					framebuffer->depth_attachment.type =
						ATTACHMENT_NONE;
					framebuffer->depth_attachment.attachment
						.renderbuffer = NULL;
				}
				if (framebuffer->stencil_attachment.type ==
					    ATTACHMENT_RENDERBUFFER &&
				    framebuffer->stencil_attachment.attachment
						    .renderbuffer == rb) {
					framebuffer->stencil_attachment.type =
						ATTACHMENT_NONE;
					framebuffer->stencil_attachment
						.attachment.renderbuffer = NULL;
				}
			}
			tracked_free(rb, sizeof(RenderbufferOES));
			/* Remove from the array */
			for (GLint j = index;
			     j < gl_state.renderbuffer_count - 1; ++j) {
				gl_state.renderbuffers[j] =
					gl_state.renderbuffers[j + 1];
			}
			gl_state.renderbuffer_count--;
			LOG_INFO(
				"glDeleteRenderbuffersOES: Deleted renderbuffer ID %u.",
				rb_id);
		} else {
			LOG_WARN(
				"glDeleteRenderbuffersOES: Renderbuffer ID %u does not exist.",
				rb_id);
		}
	}
}

/* Implementation of glGenRenderbuffersOES */
GL_API void GL_APIENTRY glGenRenderbuffersOES(GLsizei n, GLuint *renderers)
{
	for (GLsizei i = 0; i < n; ++i) {
		if (gl_state.renderbuffer_count >= MAX_RENDERBUFFERS) {
			LOG_WARN(
				"glGenRenderbuffersOES: Maximum renderbuffer limit reached.");
			renderers[i] = 0;
			glSetError(GL_OUT_OF_MEMORY);
			continue;
		}

		/* Create a renderbuffer with default parameters */
		RenderbufferOES *rb = create_renderbuffer(
			GL_RGBA4_OES, 0, 0); // Default format and size
		if (rb) {
			gl_state.renderbuffers[gl_state.renderbuffer_count++] =
				rb;
			renderers[i] = rb->id;
			LOG_DEBUG(
				"glGenRenderbuffersOES: Generated renderbuffer ID %u.",
				rb->id);
		} else {
			renderers[i] = 0;
			LOG_ERROR(
				"glGenRenderbuffersOES: Failed to generate renderbuffer.");
			glSetError(GL_OUT_OF_MEMORY);
		}
	}
}

/* Implementation of glRenderbufferStorageOES */
GL_API void GL_APIENTRY glRenderbufferStorageOES(GLenum target,
						 GLenum internalformat,
						 GLsizei width, GLsizei height)
{
	if (target != GL_RENDERBUFFER_OES) {
		LOG_ERROR("glRenderbufferStorageOES: Invalid target 0x%X.",
			  target);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (!gl_state.bound_renderbuffer) {
		LOG_ERROR(
			"glRenderbufferStorageOES: No renderbuffer is currently bound.");
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	/* Update the internal format and size */
	gl_state.bound_renderbuffer->internalformat = internalformat;
	gl_state.bound_renderbuffer->width = width;
	gl_state.bound_renderbuffer->height = height;

	/* Reset size components based on internalformat */
	switch (internalformat) {
	case GL_RGBA4_OES:
		gl_state.bound_renderbuffer->red_size = 4;
		gl_state.bound_renderbuffer->green_size = 4;
		gl_state.bound_renderbuffer->blue_size = 4;
		gl_state.bound_renderbuffer->alpha_size = 4;
		gl_state.bound_renderbuffer->depth_size = 0;
		gl_state.bound_renderbuffer->stencil_size = 0;
		break;
	case GL_RGB5_A1_OES:
		gl_state.bound_renderbuffer->red_size = 5;
		gl_state.bound_renderbuffer->green_size = 5;
		gl_state.bound_renderbuffer->blue_size = 5;
		gl_state.bound_renderbuffer->alpha_size = 1;
		gl_state.bound_renderbuffer->depth_size = 0;
		gl_state.bound_renderbuffer->stencil_size = 0;
		break;
	case GL_RGB565_OES:
		gl_state.bound_renderbuffer->red_size = 5;
		gl_state.bound_renderbuffer->green_size = 6;
		gl_state.bound_renderbuffer->blue_size = 5;
		gl_state.bound_renderbuffer->alpha_size = 0;
		gl_state.bound_renderbuffer->depth_size = 0;
		gl_state.bound_renderbuffer->stencil_size = 0;
		break;
	case GL_RGBA8_OES:
		gl_state.bound_renderbuffer->red_size = 8;
		gl_state.bound_renderbuffer->green_size = 8;
		gl_state.bound_renderbuffer->blue_size = 8;
		gl_state.bound_renderbuffer->alpha_size = 8;
		gl_state.bound_renderbuffer->depth_size = 0;
		gl_state.bound_renderbuffer->stencil_size = 0;
		break;
	case GL_DEPTH_COMPONENT24_OES:
		gl_state.bound_renderbuffer->red_size = 0;
		gl_state.bound_renderbuffer->green_size = 0;
		gl_state.bound_renderbuffer->blue_size = 0;
		gl_state.bound_renderbuffer->alpha_size = 0;
		gl_state.bound_renderbuffer->depth_size = 24;
		gl_state.bound_renderbuffer->stencil_size = 0;
		break;
	case GL_DEPTH_COMPONENT32_OES:
		gl_state.bound_renderbuffer->red_size = 0;
		gl_state.bound_renderbuffer->green_size = 0;
		gl_state.bound_renderbuffer->blue_size = 0;
		gl_state.bound_renderbuffer->alpha_size = 0;
		gl_state.bound_renderbuffer->depth_size = 32;
		gl_state.bound_renderbuffer->stencil_size = 0;
		break;
	case GL_DEPTH24_STENCIL8_OES:
		gl_state.bound_renderbuffer->red_size = 0;
		gl_state.bound_renderbuffer->green_size = 0;
		gl_state.bound_renderbuffer->blue_size = 0;
		gl_state.bound_renderbuffer->alpha_size = 0;
		gl_state.bound_renderbuffer->depth_size = 24;
		gl_state.bound_renderbuffer->stencil_size = 8;
		break;
	case GL_DEPTH_COMPONENT16_OES:
		gl_state.bound_renderbuffer->red_size = 0;
		gl_state.bound_renderbuffer->green_size = 0;
		gl_state.bound_renderbuffer->blue_size = 0;
		gl_state.bound_renderbuffer->alpha_size = 0;
		gl_state.bound_renderbuffer->depth_size = 16;
		gl_state.bound_renderbuffer->stencil_size = 0;
		break;
	default:
		LOG_WARN(
			"glRenderbufferStorageOES: Unsupported internalformat 0x%X.",
			internalformat);
		glSetError(GL_INVALID_ENUM);
		break;
	}

	LOG_DEBUG(
		"glRenderbufferStorageOES: Set storage for renderbuffer ID %u to "
		"internalformat=0x%X, width=%d, height=%d.",
		gl_state.bound_renderbuffer->id, internalformat, width, height);
}

/* Implementation of glGetRenderbufferParameterivOES */
GL_API void GL_APIENTRY glGetRenderbufferParameterivOES(GLenum target,
							GLenum pname,
							GLint *params)
{
	if (target != GL_RENDERBUFFER_OES) {
		LOG_ERROR(
			"glGetRenderbufferParameterivOES: Invalid target 0x%X.",
			target);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (!gl_state.bound_renderbuffer) {
		LOG_ERROR(
			"glGetRenderbufferParameterivOES: No renderbuffer is currently bound.");
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	switch (pname) {
	case GL_RENDERBUFFER_WIDTH_OES:
		params[0] = gl_state.bound_renderbuffer->width;
		break;
	case GL_RENDERBUFFER_HEIGHT_OES:
		params[0] = gl_state.bound_renderbuffer->height;
		break;
	case GL_RENDERBUFFER_INTERNAL_FORMAT_OES:
		params[0] = gl_state.bound_renderbuffer->internalformat;
		break;
	case GL_RENDERBUFFER_RED_SIZE_OES:
		params[0] = gl_state.bound_renderbuffer->red_size;
		break;
	case GL_RENDERBUFFER_GREEN_SIZE_OES:
		params[0] = gl_state.bound_renderbuffer->green_size;
		break;
	case GL_RENDERBUFFER_BLUE_SIZE_OES:
		params[0] = gl_state.bound_renderbuffer->blue_size;
		break;
	case GL_RENDERBUFFER_ALPHA_SIZE_OES:
		params[0] = gl_state.bound_renderbuffer->alpha_size;
		break;
	case GL_RENDERBUFFER_DEPTH_SIZE_OES:
		params[0] = gl_state.bound_renderbuffer->depth_size;
		break;
	case GL_RENDERBUFFER_STENCIL_SIZE_OES:
		params[0] = gl_state.bound_renderbuffer->stencil_size;
		break;
	default:
		LOG_ERROR(
			"glGetRenderbufferParameterivOES: Invalid pname 0x%X.",
			pname);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	LOG_DEBUG("glGetRenderbufferParameterivOES: Retrieved pname=0x%X for "
		  "renderbuffer ID %u.",
		  pname, gl_state.bound_renderbuffer->id);
}

/* Implementation of glIsFramebufferOES */
GLboolean GL_APIENTRY glIsFramebufferOES(GLuint framebuffer)
{
	for (GLint i = 0; i < gl_state.framebuffer_count; ++i) {
		if (gl_state.framebuffers[i]->id == framebuffer) {
			LOG_DEBUG(
				"glIsFramebufferOES: Framebuffer ID %u exists.",
				framebuffer);
			return GL_TRUE;
		}
	}
	LOG_DEBUG("glIsFramebufferOES: Framebuffer ID %u does not exist.",
		  framebuffer);
	return GL_FALSE;
}

/* Implementation of glBindFramebufferOES */
GL_API void GL_APIENTRY glBindFramebufferOES(GLenum target, GLuint framebuffer)
{
	if (target != GL_FRAMEBUFFER_OES) {
		LOG_ERROR("glBindFramebufferOES: Invalid target 0x%X.", target);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (framebuffer == 0) {
		gl_state.bound_framebuffer = &gl_state.default_framebuffer;
		LOG_DEBUG(
			"glBindFramebufferOES: Bound to default framebuffer.");
		return;
	}

	for (GLint i = 0; i < gl_state.framebuffer_count; ++i) {
		if (gl_state.framebuffers[i]->id == framebuffer) {
			gl_state.bound_framebuffer = gl_state.framebuffers[i];
			LOG_DEBUG(
				"glBindFramebufferOES: Bound framebuffer ID %u.",
				framebuffer);
			return;
		}
	}

	LOG_WARN("glBindFramebufferOES: Framebuffer ID %u not found.",
		 framebuffer);
	glSetError(GL_INVALID_VALUE);
}

/* Implementation of glDeleteFramebuffersOES */
GL_API void GL_APIENTRY glDeleteFramebuffersOES(GLsizei n,
						const GLuint *framebuffers)
{
	for (GLsizei i = 0; i < n; ++i) {
		GLuint fb_id = framebuffers[i];
		int index = -1;
		for (GLint j = 0; j < gl_state.framebuffer_count; ++j) {
			if (gl_state.framebuffers[j]->id == fb_id) {
				index = j;
				break;
			}
		}

		if (index != -1) {
			FramebufferOES *fb = gl_state.framebuffers[index];
			/* If the framebuffer is bound, unbind it */
			if (gl_state.bound_framebuffer == fb) {
				gl_state.bound_framebuffer =
					&gl_state.default_framebuffer;
			}
			/* Clean up attachments */
			if (fb->color_attachment.type ==
			    ATTACHMENT_RENDERBUFFER) {
				fb->color_attachment.attachment.renderbuffer =
					NULL;
			}
			if (fb->depth_attachment.type ==
			    ATTACHMENT_RENDERBUFFER) {
				fb->depth_attachment.attachment.renderbuffer =
					NULL;
			}
			if (fb->stencil_attachment.type ==
			    ATTACHMENT_RENDERBUFFER) {
				fb->stencil_attachment.attachment.renderbuffer =
					NULL;
			}
			if (fb->color_attachment.type == ATTACHMENT_TEXTURE) {
				fb->color_attachment.attachment.texture = NULL;
			}
			if (fb->depth_attachment.type == ATTACHMENT_TEXTURE) {
				fb->depth_attachment.attachment.texture = NULL;
			}
			if (fb->stencil_attachment.type == ATTACHMENT_TEXTURE) {
				fb->stencil_attachment.attachment.texture =
					NULL;
			}
			tracked_free(fb, sizeof(FramebufferOES));
			/* Remove from the array */
			for (GLint j = index;
			     j < gl_state.framebuffer_count - 1; ++j) {
				gl_state.framebuffers[j] =
					gl_state.framebuffers[j + 1];
			}
			gl_state.framebuffer_count--;
			LOG_DEBUG(
				"glDeleteFramebuffersOES: Deleted framebuffer ID %u.",
				fb_id);
		} else {
			LOG_WARN(
				"glDeleteFramebuffersOES: Framebuffer ID %u does not exist.",
				fb_id);
			glSetError(GL_INVALID_VALUE);
		}
	}
}

/* Implementation of glGenFramebuffersOES */
GL_API void GL_APIENTRY glGenFramebuffersOES(GLsizei n, GLuint *framebuffers)
{
	for (GLsizei i = 0; i < n; ++i) {
		if (gl_state.framebuffer_count >= MAX_FRAMEBUFFERS) {
			LOG_WARN(
				"glGenFramebuffersOES: Maximum framebuffer limit reached.");
			framebuffers[i] = 0;
			glSetError(GL_OUT_OF_MEMORY);
			continue;
		}

		/* Create a new framebuffer */
		FramebufferOES *fb = create_framebuffer();
		if (fb) {
			gl_state.framebuffers[gl_state.framebuffer_count++] =
				fb;
			framebuffers[i] = fb->id;
			LOG_DEBUG(
				"glGenFramebuffersOES: Generated framebuffer ID %u.",
				fb->id);
		} else {
			framebuffers[i] = 0;
			LOG_ERROR(
				"glGenFramebuffersOES: Failed to generate framebuffer.");
			glSetError(GL_OUT_OF_MEMORY);
		}
	}
}

/* Implementation of glCheckFramebufferStatusOES */
GLenum GL_APIENTRY glCheckFramebufferStatusOES(GLenum target)
{
	if (target != GL_FRAMEBUFFER_OES) {
		LOG_ERROR("glCheckFramebufferStatusOES: Invalid target 0x%X.",
			  target);
		glSetError(GL_INVALID_ENUM);
		return GL_FRAMEBUFFER_UNSUPPORTED_OES;
	}

	FramebufferOES *fb = gl_state.bound_framebuffer;
	if (fb == NULL) {
		LOG_ERROR(
			"glCheckFramebufferStatusOES: No framebuffer is currently bound.");
		glSetError(GL_INVALID_OPERATION);
		return GL_FRAMEBUFFER_UNSUPPORTED_OES;
	}

	/* Check if at least one color attachment is present */
	if (fb->color_attachment.type == ATTACHMENT_NONE &&
	    fb->depth_attachment.type == ATTACHMENT_NONE &&
	    fb->stencil_attachment.type == ATTACHMENT_NONE) {
		LOG_DEBUG(
			"glCheckFramebufferStatusOES: Framebuffer is incomplete - no "
			"attachments.");
		return GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES;
	}

	/* Check for dimension consistency */
	GLsizei width = 0, height = 0;
	AttachmentType types_to_check[3] = { fb->color_attachment.type,
					     fb->depth_attachment.type,
					     fb->stencil_attachment.type };
	RenderbufferOES *renderbuffers_to_check[3] = {
		fb->color_attachment.attachment.renderbuffer,
		fb->depth_attachment.attachment.renderbuffer,
		fb->stencil_attachment.attachment.renderbuffer
	};
	TextureOES *textures_to_check[3] = {
		fb->color_attachment.attachment.texture,
		fb->depth_attachment.attachment.texture,
		fb->stencil_attachment.attachment.texture
	};
	for (int i = 0; i < 3; ++i) {
		if (types_to_check[i] == ATTACHMENT_RENDERBUFFER &&
		    renderbuffers_to_check[i]) {
			if (i == 0) { // Color attachment
				width = renderbuffers_to_check[i]->width;
				height = renderbuffers_to_check[i]->height;
			} else {
				/* Ensure dimensions match */
				if (renderbuffers_to_check[i]->width != width ||
				    renderbuffers_to_check[i]->height !=
					    height) {
					LOG_DEBUG(
						"glCheckFramebufferStatusOES: Framebuffer is incomplete - "
						"attachment dimensions do not match.");
					return GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES;
				}
			}
		} else if (types_to_check[i] == ATTACHMENT_TEXTURE &&
			   textures_to_check[i]) {
			if (i == 0) { // Color attachment
				width = textures_to_check[i]->width;
				height = textures_to_check[i]->height;
			} else {
				if (textures_to_check[i]->width != width ||
				    textures_to_check[i]->height != height) {
					LOG_DEBUG(
						"glCheckFramebufferStatusOES: Framebuffer is incomplete - "
						"attachment dimensions do not match.");
					return GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES;
				}
			}
		}
	}

	/* Additional checks can be added here (e.g., format compatibility) */

	LOG_DEBUG("glCheckFramebufferStatusOES: Framebuffer is complete.");
	return GL_FRAMEBUFFER_COMPLETE_OES;
}

/* Implementation of glFramebufferRenderbufferOES */
GL_API void GL_APIENTRY glFramebufferRenderbufferOES(GLenum target,
						     GLenum attachment,
						     GLenum renderbuffertarget,
						     GLuint renderbuffer)
{
	if (target != GL_FRAMEBUFFER_OES) {
		LOG_ERROR("glFramebufferRenderbufferOES: Invalid target 0x%X.",
			  target);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	FramebufferOES *fb = gl_state.bound_framebuffer;
	if (fb == NULL) {
		LOG_ERROR(
			"glFramebufferRenderbufferOES: No framebuffer is currently bound.");
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	if (renderbuffertarget != GL_RENDERBUFFER_OES) {
		LOG_ERROR(
			"glFramebufferRenderbufferOES: Invalid renderbuffertarget 0x%X.",
			renderbuffertarget);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	RenderbufferOES *rb = NULL;
	if (renderbuffer != 0) {
		for (GLint i = 0; i < gl_state.renderbuffer_count; ++i) {
			if (gl_state.renderbuffers[i]->id == renderbuffer) {
				rb = gl_state.renderbuffers[i];
				break;
			}
		}
		if (rb == NULL) {
			LOG_WARN(
				"glFramebufferRenderbufferOES: Renderbuffer ID %u does not exist.",
				renderbuffer);
			glSetError(GL_INVALID_VALUE);
			return;
		}
	}

	switch (attachment) {
	case GL_COLOR_ATTACHMENT0_OES:
		if (rb) {
			fb->color_attachment.type = ATTACHMENT_RENDERBUFFER;
			fb->color_attachment.attachment.renderbuffer = rb;
			fb->color_attachment.attachment.texture = NULL;
		} else {
			fb->color_attachment.type = ATTACHMENT_NONE;
			fb->color_attachment.attachment.renderbuffer = NULL;
			fb->color_attachment.attachment.texture = NULL;
		}
		break;
	case GL_DEPTH_ATTACHMENT_OES:
		if (rb) {
			fb->depth_attachment.type = ATTACHMENT_RENDERBUFFER;
			fb->depth_attachment.attachment.renderbuffer = rb;
			fb->depth_attachment.attachment.texture = NULL;
		} else {
			fb->depth_attachment.type = ATTACHMENT_NONE;
			fb->depth_attachment.attachment.renderbuffer = NULL;
			fb->depth_attachment.attachment.texture = NULL;
		}
		break;
	case GL_STENCIL_ATTACHMENT_OES:
		if (rb) {
			fb->stencil_attachment.type = ATTACHMENT_RENDERBUFFER;
			fb->stencil_attachment.attachment.renderbuffer = rb;
			fb->stencil_attachment.attachment.texture = NULL;
		} else {
			fb->stencil_attachment.type = ATTACHMENT_NONE;
			fb->stencil_attachment.attachment.renderbuffer = NULL;
			fb->stencil_attachment.attachment.texture = NULL;
		}
		break;
	default:
		LOG_ERROR(
			"glFramebufferRenderbufferOES: Unsupported attachment 0x%X.",
			attachment);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	LOG_DEBUG(
		"glFramebufferRenderbufferOES: Attached renderbuffer ID %u to "
		"attachment 0x%X on framebuffer ID %u.",
		renderbuffer, attachment, fb->id);
}

/* Implementation of glFramebufferTexture2DOES */
GL_API void GL_APIENTRY glFramebufferTexture2DOES(GLenum target,
						  GLenum attachment,
						  GLenum textarget,
						  GLuint texture, GLint level)
{
	if (target != GL_FRAMEBUFFER_OES) {
		LOG_ERROR("glFramebufferTexture2DOES: Invalid target 0x%X.",
			  target);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	FramebufferOES *fb = gl_state.bound_framebuffer;
	if (fb == NULL) {
		LOG_ERROR(
			"glFramebufferTexture2DOES: No framebuffer is currently bound.");
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	/* Validate textarget */
	if (textarget != GL_TEXTURE_2D) {
		LOG_ERROR(
			"glFramebufferTexture2DOES: Unsupported textarget 0x%X.",
			textarget);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	TextureOES *tex = NULL;
	if (texture != 0) {
		RenderContext *ctx = GetCurrentContext();
		for (GLuint i = 0; i < ctx->texture_count; ++i) {
			if (ctx->textures[i] &&
			    ctx->textures[i]->id == texture) {
				tex = ctx->textures[i];
				break;
			}
		}
		if (tex == NULL) {
			LOG_WARN(
				"glFramebufferTexture2DOES: Texture ID %u does not exist.",
				texture);
			glSetError(GL_INVALID_VALUE);
			return;
		}
	}

	switch (attachment) {
	case GL_COLOR_ATTACHMENT0_OES:
		if (tex) {
			fb->color_attachment.type = ATTACHMENT_TEXTURE;
			fb->color_attachment.attachment.texture = tex;
			fb->color_attachment.attachment.renderbuffer = NULL;
		} else {
			fb->color_attachment.type = ATTACHMENT_NONE;
			fb->color_attachment.attachment.texture = NULL;
			fb->color_attachment.attachment.renderbuffer = NULL;
		}
		break;
	case GL_DEPTH_ATTACHMENT_OES:
		if (tex) {
			fb->depth_attachment.type = ATTACHMENT_TEXTURE;
			fb->depth_attachment.attachment.texture = tex;
			fb->depth_attachment.attachment.renderbuffer = NULL;
		} else {
			fb->depth_attachment.type = ATTACHMENT_NONE;
			fb->depth_attachment.attachment.texture = NULL;
			fb->depth_attachment.attachment.renderbuffer = NULL;
		}
		break;
	case GL_STENCIL_ATTACHMENT_OES:
		if (tex) {
			fb->stencil_attachment.type = ATTACHMENT_TEXTURE;
			fb->stencil_attachment.attachment.texture = tex;
			fb->stencil_attachment.attachment.renderbuffer = NULL;
		} else {
			fb->stencil_attachment.type = ATTACHMENT_NONE;
			fb->stencil_attachment.attachment.texture = NULL;
			fb->stencil_attachment.attachment.renderbuffer = NULL;
		}
		break;
	default:
		LOG_ERROR(
			"glFramebufferTexture2DOES: Unsupported attachment 0x%X.",
			attachment);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	LOG_DEBUG(
		"glFramebufferTexture2DOES: Attached texture ID %u to attachment "
		"0x%X on framebuffer ID %u at level %d.",
		texture, attachment, fb->id, level);
}

/* Implementation of glGetFramebufferAttachmentParameterivOES */
GL_API void GL_APIENTRY glGetFramebufferAttachmentParameterivOES(
	GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
	if (target != GL_FRAMEBUFFER_OES) {
		LOG_ERROR(
			"glGetFramebufferAttachmentParameterivOES: Invalid target 0x%X.",
			target);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	FramebufferOES *fb = gl_state.bound_framebuffer;
	if (fb == NULL) {
		LOG_ERROR(
			"glGetFramebufferAttachmentParameterivOES: No framebuffer is "
			"currently bound.");
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	/* Determine which attachment to query */
	AttachmentType type;
	union {
		RenderbufferOES *rb;
		TextureOES *tex;
	} attachment_ptr;

	switch (attachment) {
	case GL_COLOR_ATTACHMENT0_OES:
		type = fb->color_attachment.type;
		attachment_ptr.rb =
			fb->color_attachment.attachment.renderbuffer;
		attachment_ptr.tex = fb->color_attachment.attachment.texture;
		break;
	case GL_DEPTH_ATTACHMENT_OES:
		type = fb->depth_attachment.type;
		attachment_ptr.rb =
			fb->depth_attachment.attachment.renderbuffer;
		attachment_ptr.tex = fb->depth_attachment.attachment.texture;
		break;
	case GL_STENCIL_ATTACHMENT_OES:
		type = fb->stencil_attachment.type;
		attachment_ptr.rb =
			fb->stencil_attachment.attachment.renderbuffer;
		attachment_ptr.tex = fb->stencil_attachment.attachment.texture;
		break;
	default:
		LOG_ERROR(
			"glGetFramebufferAttachmentParameterivOES: Unsupported "
			"attachment 0x%X.",
			attachment);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	if (type == ATTACHMENT_NONE) {
		if (pname == GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES) {
			params[0] = GL_NONE_OES;
		} else {
			LOG_ERROR(
				"glGetFramebufferAttachmentParameterivOES: No object attached "
				"to attachment 0x%X.",
				attachment);
			glSetError(GL_INVALID_OPERATION);
		}
		return;
	}

	/* Handle parameters based on attachment type */
	switch (pname) {
	case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES:
		params[0] = (type == ATTACHMENT_RENDERBUFFER) ?
				    GL_RENDERBUFFER_OES :
				    GL_TEXTURE_2D;
		break;
	case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES:
		if (type == ATTACHMENT_RENDERBUFFER) {
			if (attachment_ptr.rb) {
				params[0] = attachment_ptr.rb->id;
			} else {
				params[0] = 0;
			}
		} else if (type == ATTACHMENT_TEXTURE) {
			if (attachment_ptr.tex) {
				params[0] = attachment_ptr.tex->id;
			} else {
				params[0] = 0;
			}
		}
		break;
	case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_OES:
		params[0] = 0; // Assuming level 0 for simplicity
		break;
	case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_OES:
		params[0] = 0; // Cube maps not supported
		break;
	default:
		LOG_ERROR(
			"glGetFramebufferAttachmentParameterivOES: Unsupported pname 0x%X.",
			pname);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	LOG_DEBUG(
		"glGetFramebufferAttachmentParameterivOES: Retrieved pname=0x%X "
		"for attachment 0x%X on framebuffer ID %u.",
		pname, attachment, fb->id);
}

/* Implementation of glGenerateMipmapOES */
GL_API void GL_APIENTRY glGenerateMipmapOES(GLenum target)
{
	if (target != GL_TEXTURE_2D) {
		LOG_ERROR(
			"glGenerateMipmapOES: Invalid target 0x%X. Only GL_TEXTURE_2D is "
			"supported.",
			target);
		glSetError(GL_INVALID_ENUM);
		return;
	}

	/* The spec operates on the texture currently bound to the active unit. */
	RenderContext *ctx = GetCurrentContext();
	GLuint tex_id = ctx->texture_env[ctx->active_texture - GL_TEXTURE0]
				.bound_texture;
	TextureOES *tex = context_find_texture(tex_id);
	if (!tex) {
		LOG_ERROR(
			"glGenerateMipmapOES: No texture bound to active unit %d.",
			ctx->active_texture - GL_TEXTURE0);
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	/* Check if the texture supports mipmaps */
	if (!tex->mipmap_supported) {
		LOG_ERROR(
			"glGenerateMipmapOES: Texture ID %u does not support mipmaps.",
			tex->id);
		glSetError(GL_INVALID_OPERATION);
		return;
	}

	/* Simulated mipmap generation */
	tex->current_level = level_max(tex->width, tex->height);
	LOG_DEBUG(
		"glGenerateMipmapOES: Generated mipmaps for texture ID %u up to "
		"level %d.",
		tex->id, tex->current_level);
}

/* Helper function to calculate maximum mipmap level */
static GLint level_max(GLsizei width, GLsizei height)
{
	GLint levels = 1;
	while ((width >>= 1) > 1 || (height >>= 1) > 1) {
		levels++;
	}
	return levels;
}
