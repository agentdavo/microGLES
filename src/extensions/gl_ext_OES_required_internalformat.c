#include "gl_ext_common.h"
#include <GLES/glext.h>

EXT_REGISTER("GL_OES_required_internalformat")
__attribute__((used)) int ext_link_dummy_OES_required_internalformat = 0;

static const GLenum required_formats[] = { GL_ALPHA8_OES,
					   GL_LUMINANCE4_ALPHA4_OES,
					   GL_LUMINANCE8_ALPHA8_OES,
					   GL_LUMINANCE8_OES,
					   GL_RGB8_OES,
					   GL_RGBA8_OES,
					   GL_RGB10_EXT,
					   GL_RGB10_A2_EXT };

GLboolean is_required_internalformat(GLenum fmt)
{
	for (size_t i = 0;
	     i < sizeof(required_formats) / sizeof(required_formats[0]); ++i) {
		if (required_formats[i] == fmt)
			return GL_TRUE;
	}
	return GL_FALSE;
}
