#include "gl_extensions.h"
#include "gl_errors.h"
#include "gl_state.h"
#include "logger.h"
#include <GLES/gl.h>

static const char *EXT_STRING =
    "GL_OES_draw_texture GL_OES_matrix_get GL_OES_point_size_array "
    "GL_OES_point_sprite GL_OES_framebuffer_object GL_OES_EGL_image "
    "GL_OES_EGL_image_external GL_OES_required_internalformat "
    "GL_OES_fixed_point GL_OES_texture_env_crossbar "
    "GL_OES_texture_mirrored_repeat GL_OES_texture_cube_map "
    "GL_OES_blend_subtract GL_OES_blend_func_separate "
    "GL_OES_blend_equation_separate GL_OES_stencil_wrap "
    "GL_OES_extended_matrix_palette";

extern GLState gl_state;
#define FIXED_TO_FLOAT(x) ((GLfloat)(x) / 65536.0f)

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
void glAlphaFuncxOES(GLenum func, GLfixed ref) {
  (void)func;
  (void)ref;
  LOG_INFO("glAlphaFuncxOES called - not yet supported.");
}

void glClearColorxOES(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
  (void)red;
  (void)green;
  (void)blue;
  (void)alpha;
  LOG_INFO("glClearColorxOES called - not yet supported.");
}

void glClearDepthxOES(GLfixed depth) {
  (void)depth;
  LOG_INFO("glClearDepthxOES called - not yet supported.");
}

void glClipPlanexOES(GLenum plane, const GLfixed *equation) {
  (void)plane;
  (void)equation;
  LOG_INFO("glClipPlanexOES called - not yet supported.");
}

void glColor4xOES(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
  (void)red;
  (void)green;
  (void)blue;
  (void)alpha;
  LOG_INFO("glColor4xOES called - not yet supported.");
}

void glDepthRangexOES(GLfixed n, GLfixed f) {
  (void)n;
  (void)f;
  LOG_INFO("glDepthRangexOES called - not yet supported.");
}

void glFogxOES(GLenum pname, GLfixed param) {
  glFogf(pname, FIXED_TO_FLOAT(param));
}

void glFogxvOES(GLenum pname, const GLfixed *param) {
  if (!param) {
    glSetError(GL_INVALID_VALUE);
    return;
  }
  GLfloat vals[4];
  vals[0] = FIXED_TO_FLOAT(param[0]);
  vals[1] = FIXED_TO_FLOAT(param[1]);
  vals[2] = FIXED_TO_FLOAT(param[2]);
  vals[3] = FIXED_TO_FLOAT(param[3]);
  glFogfv(pname, vals);
}

void glFrustumxOES(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n,
                   GLfixed f) {
  glFrustumf(FIXED_TO_FLOAT(l), FIXED_TO_FLOAT(r), FIXED_TO_FLOAT(b),
             FIXED_TO_FLOAT(t), FIXED_TO_FLOAT(n), FIXED_TO_FLOAT(f));
}

void glGetClipPlanexOES(GLenum plane, GLfixed *equation) {
  (void)plane;
  (void)equation;
  LOG_INFO("glGetClipPlanexOES called - not yet supported.");
}

void glGetFixedvOES(GLenum pname, GLfixed *params) {
  (void)pname;
  (void)params;
  LOG_INFO("glGetFixedvOES called - not yet supported.");
}

void glGetLightxvOES(GLenum light, GLenum pname, GLfixed *params) {
  (void)light;
  (void)pname;
  (void)params;
  LOG_INFO("glGetLightxvOES called - not yet supported.");
}

void glGetMaterialxvOES(GLenum face, GLenum pname, GLfixed *params) {
  (void)face;
  (void)pname;
  (void)params;
  LOG_INFO("glGetMaterialxvOES called - not yet supported.");
}

void glGetTexEnvxvOES(GLenum target, GLenum pname, GLfixed *params) {
  (void)target;
  (void)pname;
  (void)params;
  LOG_INFO("glGetTexEnvxvOES called - not yet supported.");
}

void glGetTexParameterxvOES(GLenum target, GLenum pname, GLfixed *params) {
  (void)target;
  (void)pname;
  (void)params;
  LOG_INFO("glGetTexParameterxvOES called - not yet supported.");
}

void glLightModelxOES(GLenum pname, GLfixed param) {
  (void)pname;
  (void)param;
  LOG_INFO("glLightModelxOES called - not yet supported.");
}

void glLightModelxvOES(GLenum pname, const GLfixed *param) {
  (void)pname;
  (void)param;
  LOG_INFO("glLightModelxvOES called - not yet supported.");
}

void glLightxOES(GLenum light, GLenum pname, GLfixed param) {
  glLightf(light, pname, FIXED_TO_FLOAT(param));
}

void glLightxvOES(GLenum light, GLenum pname, const GLfixed *params) {
  if (!params) {
    glSetError(GL_INVALID_VALUE);
    return;
  }
  GLfloat vals[4];
  vals[0] = FIXED_TO_FLOAT(params[0]);
  vals[1] = FIXED_TO_FLOAT(params[1]);
  vals[2] = FIXED_TO_FLOAT(params[2]);
  vals[3] = FIXED_TO_FLOAT(params[3]);
  glLightfv(light, pname, vals);
}

void glLineWidthxOES(GLfixed width) {
  (void)width;
  LOG_INFO("glLineWidthxOES called - not yet supported.");
}

void glLoadMatrixxOES(const GLfixed *m) {
  (void)m;
  LOG_INFO("glLoadMatrixxOES called - not yet supported.");
}

void glMaterialxOES(GLenum face, GLenum pname, GLfixed param) {
  (void)face;
  (void)pname;
  (void)param;
  LOG_INFO("glMaterialxOES called - not yet supported.");
}

void glMaterialxvOES(GLenum face, GLenum pname, const GLfixed *param) {
  (void)face;
  (void)pname;
  (void)param;
  LOG_INFO("glMaterialxvOES called - not yet supported.");
}

void glMultMatrixxOES(const GLfixed *m) {
  (void)m;
  LOG_INFO("glMultMatrixxOES called - not yet supported.");
}

void glMultiTexCoord4xOES(GLenum texture, GLfixed s, GLfixed t, GLfixed r,
                          GLfixed q) {
  (void)texture;
  (void)s;
  (void)t;
  (void)r;
  (void)q;
  LOG_INFO("glMultiTexCoord4xOES called - not yet supported.");
}

void glNormal3xOES(GLfixed nx, GLfixed ny, GLfixed nz) {
  (void)nx;
  (void)ny;
  (void)nz;
  LOG_INFO("glNormal3xOES called - not yet supported.");
}

void glOrthoxOES(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n,
                 GLfixed f) {
  (void)l;
  (void)r;
  (void)b;
  (void)t;
  (void)n;
  (void)f;
  LOG_INFO("glOrthoxOES called - not yet supported.");
}

void glPointParameterxOES(GLenum pname, GLfixed param) {
  (void)pname;
  (void)param;
  LOG_INFO("glPointParameterxOES called - not yet supported.");
}

void glPointParameterxvOES(GLenum pname, const GLfixed *params) {
  (void)pname;
  (void)params;
  LOG_INFO("glPointParameterxvOES called - not yet supported.");
}

void glPointSizexOES(GLfixed size) {
  (void)size;
  LOG_INFO("glPointSizexOES called - not yet supported.");
}

void glPolygonOffsetxOES(GLfixed factor, GLfixed units) {
  (void)factor;
  (void)units;
  LOG_INFO("glPolygonOffsetxOES called - not yet supported.");
}

void glRotatexOES(GLfixed angle, GLfixed x, GLfixed y, GLfixed z) {
  (void)angle;
  (void)x;
  (void)y;
  (void)z;
  LOG_INFO("glRotatexOES called - not yet supported.");
}

void glSampleCoveragexOES(GLclampx value, GLboolean invert) {
  (void)value;
  (void)invert;
  LOG_INFO("glSampleCoveragexOES called - not yet supported.");
}

void glScalexOES(GLfixed x, GLfixed y, GLfixed z) {
  (void)x;
  (void)y;
  (void)z;
  LOG_INFO("glScalexOES called - not yet supported.");
}

void glTexEnvxOES(GLenum target, GLenum pname, GLfixed param) {
  (void)target;
  (void)pname;
  (void)param;
  LOG_INFO("glTexEnvxOES called - not yet supported.");
}

void glTexEnvxvOES(GLenum target, GLenum pname, const GLfixed *params) {
  (void)target;
  (void)pname;
  (void)params;
  LOG_INFO("glTexEnvxvOES called - not yet supported.");
}

void glTexParameterxOES(GLenum target, GLenum pname, GLfixed param) {
  (void)target;
  (void)pname;
  (void)param;
  LOG_INFO("glTexParameterxOES called - not yet supported.");
}

void glTexParameterxvOES(GLenum target, GLenum pname, const GLfixed *params) {
  (void)target;
  (void)pname;
  (void)params;
  LOG_INFO("glTexParameterxvOES called - not yet supported.");
}

void glTranslatexOES(GLfixed x, GLfixed y, GLfixed z) {
  (void)x;
  (void)y;
  (void)z;
  LOG_INFO("glTranslatexOES called - not yet supported.");
}
