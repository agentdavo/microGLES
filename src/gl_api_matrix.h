#ifndef GL_API_MATRIX_H
#define GL_API_MATRIX_H
/**
 * @file gl_api_matrix.h
 * @brief Matrix manipulation API declarations.
 */
#include <GLES/gl.h>

GL_API void GL_APIENTRY glMatrixMode(GLenum mode);
GL_API void GL_APIENTRY glPushMatrix(void);
GL_API void GL_APIENTRY glPopMatrix(void);
GL_API void GL_APIENTRY glLoadIdentity(void);
GL_API void GL_APIENTRY glLoadMatrixf(const GLfloat *m);
GL_API void GL_APIENTRY glMultMatrixf(const GLfloat *m);
GL_API void GL_APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z);
GL_API void GL_APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y,
				  GLfloat z);
GL_API void GL_APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z);
GL_API void GL_APIENTRY glFrustumf(GLfloat l, GLfloat r, GLfloat b, GLfloat t,
				   GLfloat n, GLfloat f);
GL_API void GL_APIENTRY glOrthof(GLfloat l, GLfloat r, GLfloat b, GLfloat t,
				 GLfloat n, GLfloat f);

#endif /* GL_API_MATRIX_H */
