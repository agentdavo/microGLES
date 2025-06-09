#ifndef GL_FUNCTIONS_H
#define GL_FUNCTIONS_H

#include <GLES/gl.h>

// Alpha and Blending Functions
void glAlphaFunc(GLenum func, GLfloat ref);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glClearDepthf(GLfloat d);

// Transformation Functions
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glScalef(GLfloat x, GLfloat y, GLfloat z);
void glLoadMatrixf(const GLfloat *m);
void glMultMatrixf(const GLfloat *m);
void glLoadIdentity(void);
void glFrustumf(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
void glOrthof(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);

// Lighting and Materials
void glLightf(GLenum light, GLenum pname, GLfloat param);
void glLightfv(GLenum light, GLenum pname, const GLfloat *params);
void glMaterialf(GLenum face, GLenum pname, GLfloat param);
void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
void glLightModelf(GLenum pname, GLfloat param);
void glLightModelfv(GLenum pname, const GLfloat *params);

// Fog Functions
void glFogf(GLenum pname, GLfloat param);
void glFogfv(GLenum pname, const GLfloat *params);

// Clipping and Depth Functions
void glClipPlanef(GLenum plane, const GLfloat *equation);
void glGetClipPlanef(GLenum plane, GLfloat *equation);
void glDepthRangef(GLfloat n, GLfloat f);
void glDepthFunc(GLenum func);
void glDepthMask(GLboolean flag);

// Texture Functions
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void glTexEnvf(GLenum target, GLenum pname, GLfloat param);
void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params);
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);

// Buffer Functions
void glBindBuffer(GLenum target, GLuint buffer);
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
void glDeleteBuffers(GLsizei n, const GLuint *buffers);
void glGenBuffers(GLsizei n, GLuint *buffers);
void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params);

// Vertex and Color Pointer Functions
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
void glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
void glNormalPointer(GLenum type, GLsizei stride, const void *pointer);
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);

// State Query Functions
void glGetFloatv(GLenum pname, GLfloat *data);
void glGetIntegerv(GLenum pname, GLint *data);
void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params);
void glGetError(void);
const GLubyte *glGetString(GLenum name);

// Drawing Functions
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
void glClear(GLbitfield mask);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

// Miscellaneous Functions
void glEnable(GLenum cap);
void glDisable(GLenum cap);
void glEnableClientState(GLenum array);
void glDisableClientState(GLenum array);
void glFinish(void);
void glFlush(void);
void glShadeModel(GLenum mode);
void glCullFace(GLenum mode);
void glPolygonOffset(GLfloat factor, GLfloat units);
void glHint(GLenum target, GLenum mode);
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void glStencilFunc(GLenum func, GLint ref, GLuint mask);
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
void glStencilMask(GLuint mask);

// Fixed-Point Functions
void glAlphaFuncx(GLenum func, GLfixed ref);
void glClearColorx(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
void glClearDepthx(GLfixed depth);
void glDepthRangex(GLfixed n, GLfixed f);
void glFrustumx(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
void glOrthox(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
void glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
void glScalex(GLfixed x, GLfixed y, GLfixed z);
void glTranslatex(GLfixed x, GLfixed y, GLfixed z);

#endif // GL_FUNCTIONS_H
