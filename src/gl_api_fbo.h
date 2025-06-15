/* gl_api_fbo.h */

#ifndef GL_API_FBO_H
#define GL_API_FBO_H
/**
 * @file gl_api_fbo.h
 * @brief Framebuffer object helpers for the ES 1.1 renderer.
 */

#include "gl_types.h" /* For TextureOES */
#include "pipeline/gl_framebuffer.h" /* For Framebuffer */
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Attachment types */
typedef enum {
	ATTACHMENT_NONE,
	ATTACHMENT_RENDERBUFFER,
	ATTACHMENT_TEXTURE
} AttachmentType;

/* Renderbuffer structure */
typedef struct RenderbufferOES {
	GLuint id;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	union {
		struct {
			GLint red_size;
			GLint green_size;
			GLint blue_size;
			GLint alpha_size;
		};
		GLint color_size[4];
	};
	GLint depth_size;
	GLint stencil_size;
} RenderbufferOES;

/* Framebuffer attachment union */
typedef union {
	RenderbufferOES *renderbuffer;
	TextureOES *texture;
} FramebufferAttachment;

/* Framebuffer attachment structure */
typedef struct {
	AttachmentType type;
	FramebufferAttachment attachment;
} FramebufferAttachmentOES;

/* Framebuffer structure */
typedef struct FramebufferOES {
	GLuint id;
	FramebufferAttachmentOES color_attachment;
	FramebufferAttachmentOES depth_attachment;
	FramebufferAttachmentOES stencil_attachment;
	/*
         * Actual pixel storage for the rasterizer. When the framebuffer is
         * bound, all clears and draws operate on this object.
         */
	struct Framebuffer *fb;
} FramebufferOES;

/* Function prototypes */
GLboolean GL_APIENTRY glIsRenderbufferOES(GLuint renderbuffer);
void GL_APIENTRY glBindRenderbufferOES(GLenum target, GLuint renderbuffer);
void GL_APIENTRY glDeleteRenderbuffersOES(GLsizei n,
					  const GLuint *renderbuffers);
void GL_APIENTRY glGenRenderbuffersOES(GLsizei n, GLuint *renderbuffers);
void GL_APIENTRY glRenderbufferStorageOES(GLenum target, GLenum internalformat,
					  GLsizei width, GLsizei height);
void GL_APIENTRY glGetRenderbufferParameterivOES(GLenum target, GLenum pname,
						 GLint *params);
GLboolean GL_APIENTRY glIsFramebufferOES(GLuint framebuffer);
void GL_APIENTRY glBindFramebufferOES(GLenum target, GLuint framebuffer);
void GL_APIENTRY glDeleteFramebuffersOES(GLsizei n, const GLuint *framebuffers);
void GL_APIENTRY glGenFramebuffersOES(GLsizei n, GLuint *framebuffers);
GLenum GL_APIENTRY glCheckFramebufferStatusOES(GLenum target);
void GL_APIENTRY glFramebufferRenderbufferOES(GLenum target, GLenum attachment,
					      GLenum renderbuffertarget,
					      GLuint renderbuffer);
void GL_APIENTRY glFramebufferTexture2DOES(GLenum target, GLenum attachment,
					   GLenum textarget, GLuint texture,
					   GLint level);
void GL_APIENTRY glGetFramebufferAttachmentParameterivOES(GLenum target,
							  GLenum attachment,
							  GLenum pname,
							  GLint *params);
void GL_APIENTRY glGenerateMipmapOES(GLenum target);

#ifdef __cplusplus
}
#endif

#endif /* GL_API_FBO_H */
