#include "gl_ext_common.h"
#include <GLES/glext.h>

EXT_REGISTER("GL_OES_EGL_image_external")
int ext_link_dummy_OES_egl_image_external;

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif
#ifndef GL_TEXTURE_BINDING_EXTERNAL_OES
#define GL_TEXTURE_BINDING_EXTERNAL_OES 0x8D67
#endif
