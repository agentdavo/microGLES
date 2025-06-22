#include "gl_errors.h"
#include "gl_ext_common.h"
#include "../gl_logger.h"
#include "../gl_utils.h"
#include <GLES/glext.h>

EXT_REGISTER("GL_OES_EGL_image")
__attribute__((used)) int ext_link_dummy_OES_egl_image = 0;

GL_API void GL_APIENTRY glEGLImageTargetTexture2DOES(GLenum target,
						     GLeglImageOES image)
{
	if (target != GL_TEXTURE_2D) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (!image) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	LOG_INFO(
		"glEGLImageTargetTexture2DOES called - EGLImage not supported");
	glSetError(GL_INVALID_OPERATION);
}

GL_API void GL_APIENTRY
glEGLImageTargetRenderbufferStorageOES(GLenum target, GLeglImageOES image)
{
	if (target != GL_RENDERBUFFER_OES) {
		glSetError(GL_INVALID_ENUM);
		return;
	}
	if (!image) {
		glSetError(GL_INVALID_VALUE);
		return;
	}
	LOG_INFO(
		"glEGLImageTargetRenderbufferStorageOES called - EGLImage not supported");
	glSetError(GL_INVALID_OPERATION);
}
