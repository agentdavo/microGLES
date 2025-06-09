#include "gl_extensions.h"
#include "logger.h"

static const char *EXT_STRING =
    "GL_OES_draw_texture GL_OES_matrix_get GL_OES_point_size_array "
    "GL_OES_point_sprite GL_OES_framebuffer_object GL_OES_EGL_image "
    "GL_OES_EGL_image_external GL_OES_required_internalformat "
    "GL_OES_fixed_point GL_OES_texture_env_crossbar "
    "GL_OES_texture_mirrored_repeat GL_OES_texture_cube_map "
    "GL_OES_blend_subtract GL_OES_blend_func_separate "
    "GL_OES_blend_equation_separate GL_OES_stencil_wrap "
    "GL_OES_extended_matrix_palette";

const GLubyte *renderer_get_extensions(void) {
  return (const GLubyte *)EXT_STRING;
}

void glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width,
                   GLshort height) {
  (void)x;
  (void)y;
  (void)z;
  (void)width;
  (void)height;
  LOG_INFO("glDrawTexsOES called - no software implementation available.");
}

void glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height) {
  (void)x;
  (void)y;
  (void)z;
  (void)width;
  (void)height;
  LOG_INFO("glDrawTexiOES called - no software implementation available.");
}

void glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width,
                   GLfixed height) {
  (void)x;
  (void)y;
  (void)z;
  (void)width;
  (void)height;
  LOG_INFO("glDrawTexxOES called - no software implementation available.");
}

void glDrawTexsvOES(const GLshort *coords) {
  (void)coords;
  LOG_INFO("glDrawTexsvOES called - no software implementation available.");
}

void glDrawTexivOES(const GLint *coords) {
  (void)coords;
  LOG_INFO("glDrawTexivOES called - no software implementation available.");
}

void glDrawTexxvOES(const GLfixed *coords) {
  (void)coords;
  LOG_INFO("glDrawTexxvOES called - no software implementation available.");
}

void glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width,
                   GLfloat height) {
  (void)x;
  (void)y;
  (void)z;
  (void)width;
  (void)height;
  LOG_INFO("glDrawTexfOES called - no software implementation available.");
}

void glDrawTexfvOES(const GLfloat *coords) {
  (void)coords;
  LOG_INFO("glDrawTexfvOES called - no software implementation available.");
}

/* Blend and TexGen extension stubs */
void glBlendEquationOES(GLenum mode) {
  (void)mode;
  LOG_INFO("glBlendEquationOES called - not yet supported.");
}

void glBlendFuncSeparateOES(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha,
                            GLenum dstAlpha) {
  (void)srcRGB;
  (void)dstRGB;
  (void)srcAlpha;
  (void)dstAlpha;
  LOG_INFO("glBlendFuncSeparateOES called - not yet supported.");
}

void glBlendEquationSeparateOES(GLenum modeRGB, GLenum modeAlpha) {
  (void)modeRGB;
  (void)modeAlpha;
  LOG_INFO("glBlendEquationSeparateOES called - not yet supported.");
}

void glTexGenfOES(GLenum coord, GLenum pname, GLfloat param) {
  (void)coord;
  (void)pname;
  (void)param;
  LOG_INFO("glTexGenfOES called - not yet supported.");
}

void glTexGenfvOES(GLenum coord, GLenum pname, const GLfloat *params) {
  (void)coord;
  (void)pname;
  (void)params;
  LOG_INFO("glTexGenfvOES called - not yet supported.");
}

void glTexGeniOES(GLenum coord, GLenum pname, GLint param) {
  (void)coord;
  (void)pname;
  (void)param;
  LOG_INFO("glTexGeniOES called - not yet supported.");
}

void glTexGenivOES(GLenum coord, GLenum pname, const GLint *params) {
  (void)coord;
  (void)pname;
  (void)params;
  LOG_INFO("glTexGenivOES called - not yet supported.");
}

void glGetTexGenfvOES(GLenum coord, GLenum pname, GLfloat *params) {
  (void)coord;
  (void)pname;
  (void)params;
  LOG_INFO("glGetTexGenfvOES called - not yet supported.");
}

void glGetTexGenivOES(GLenum coord, GLenum pname, GLint *params) {
  (void)coord;
  (void)pname;
  (void)params;
  LOG_INFO("glGetTexGenivOES called - not yet supported.");
}

void glCurrentPaletteMatrixOES(GLuint matrixpaletteindex) {
  (void)matrixpaletteindex;
  LOG_INFO("glCurrentPaletteMatrixOES called - not yet supported.");
}

void glLoadPaletteFromModelViewMatrixOES(void) {
  LOG_INFO("glLoadPaletteFromModelViewMatrixOES called - not yet supported.");
}

void glMatrixIndexPointerOES(GLint size, GLenum type, GLsizei stride,
                             const void *pointer) {
  (void)size;
  (void)type;
  (void)stride;
  (void)pointer;
  LOG_INFO("glMatrixIndexPointerOES called - not yet supported.");
}

void glWeightPointerOES(GLint size, GLenum type, GLsizei stride,
                        const void *pointer) {
  (void)size;
  (void)type;
  (void)stride;
  (void)pointer;
  LOG_INFO("glWeightPointerOES called - not yet supported.");
}

static GLenum g_point_size_type = GL_FLOAT;
static GLsizei g_point_size_stride = 0;
static const void *g_point_size_pointer = NULL;

void glPointSizePointerOES(GLenum type, GLsizei stride, const void *pointer) {
  g_point_size_type = type;
  g_point_size_stride = stride;
  g_point_size_pointer = pointer;
  LOG_INFO("glPointSizePointerOES set pointer=%p type=0x%X stride=%d.", pointer,
           type, stride);
}

/* Helper to query current point size array pointer, used for testing */
const void *getPointSizePointerOES(GLenum *type, GLsizei *stride) {
  if (type)
    *type = g_point_size_type;
  if (stride)
    *stride = g_point_size_stride;
  return g_point_size_pointer;
}
