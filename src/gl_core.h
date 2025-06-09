#ifndef GL_CORE_H
#define GL_CORE_H

#include "gl_types.h"
#include "gl_errors.h"

void glAlphaFunc(GLenum func, GLfloat ref);
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glClearDepthf(GLfloat d);
void glClipPlanef(GLenum p, const GLfloat *eqn);
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glDepthRangef(GLfloat n, GLfloat f);
void glEnable(GLenum cap);
void glDisable(GLenum cap);
void glFinish(void);
void glFlush(void);
void glGetFloatv(GLenum pname, GLfloat *data);
void glGetIntegerv(GLenum pname, GLint *data);

#endif // GL_CORE_H
