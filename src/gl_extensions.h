#ifndef GL_EXTENSIONS_H
#define GL_EXTENSIONS_H

#include <GLES/gl.h>
#include <GLES/glext.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Extension tokens for point sprite and point size arrays */
#ifndef GL_POINT_SPRITE_OES
#define GL_POINT_SPRITE_OES 0x8861
#endif
#ifndef GL_COORD_REPLACE_OES
#define GL_COORD_REPLACE_OES 0x8862
#endif
#ifndef GL_POINT_SIZE_ARRAY_OES
#define GL_POINT_SIZE_ARRAY_OES 0x8B9C
#define GL_POINT_SIZE_ARRAY_TYPE_OES 0x898A
#define GL_POINT_SIZE_ARRAY_STRIDE_OES 0x898B
#define GL_POINT_SIZE_ARRAY_POINTER_OES 0x898C
#define GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES 0x8B9F
#endif

/* Retrieve the extension string supported by the renderer */
const GLubyte *renderer_get_extensions(void);

/* Stub implementations for GL_OES_draw_texture */
void glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width,
                   GLshort height);
void glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height);
void glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width,
                   GLfixed height);
void glDrawTexsvOES(const GLshort *coords);
void glDrawTexivOES(const GLint *coords);
void glDrawTexxvOES(const GLfixed *coords);
void glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width,
                   GLfloat height);
void glDrawTexfvOES(const GLfloat *coords);

/* Blend and TexGen extension stubs */
void glBlendEquationOES(GLenum mode);
void glBlendFuncSeparateOES(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha,
                            GLenum dstAlpha);
void glBlendEquationSeparateOES(GLenum modeRGB, GLenum modeAlpha);
void glTexGenfOES(GLenum coord, GLenum pname, GLfloat param);
void glTexGenfvOES(GLenum coord, GLenum pname, const GLfloat *params);
void glTexGeniOES(GLenum coord, GLenum pname, GLint param);
void glTexGenivOES(GLenum coord, GLenum pname, const GLint *params);
void glGetTexGenfvOES(GLenum coord, GLenum pname, GLfloat *params);
void glGetTexGenivOES(GLenum coord, GLenum pname, GLint *params);
void glCurrentPaletteMatrixOES(GLuint matrixpaletteindex);
void glLoadPaletteFromModelViewMatrixOES(void);
void glMatrixIndexPointerOES(GLint size, GLenum type, GLsizei stride,
                             const void *pointer);
void glWeightPointerOES(GLint size, GLenum type, GLsizei stride,
                        const void *pointer);
void glPointSizePointerOES(GLenum type, GLsizei stride, const void *pointer);
const void *getPointSizePointerOES(GLenum *type, GLsizei *stride);

#ifdef __cplusplus
}
#endif

#endif /* GL_EXTENSIONS_H */
