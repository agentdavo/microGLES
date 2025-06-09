#ifndef GL_FIXED_POINT_H
#define GL_FIXED_POINT_H

#include <GLES/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

GL_API void GL_APIENTRY glAlphaFuncxOES(GLenum func, GLfixed ref);
GL_API void GL_APIENTRY glClearColorxOES(GLfixed red, GLfixed green,
                                         GLfixed blue, GLfixed alpha);
GL_API void GL_APIENTRY glClearDepthxOES(GLfixed depth);
GL_API void GL_APIENTRY glClipPlanexOES(GLenum plane, const GLfixed *equation);
GL_API void GL_APIENTRY glColor4xOES(GLfixed red, GLfixed green, GLfixed blue,
                                     GLfixed alpha);
GL_API void GL_APIENTRY glDepthRangexOES(GLfixed n, GLfixed f);
GL_API void GL_APIENTRY glFogxOES(GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glFogxvOES(GLenum pname, const GLfixed *param);
GL_API void GL_APIENTRY glFrustumxOES(GLfixed l, GLfixed r, GLfixed b,
                                      GLfixed t, GLfixed n, GLfixed f);
GL_API void GL_APIENTRY glGetClipPlanexOES(GLenum plane, GLfixed *equation);
GL_API void GL_APIENTRY glGetFixedvOES(GLenum pname, GLfixed *params);
GL_API void GL_APIENTRY glGetLightxvOES(GLenum light, GLenum pname,
                                        GLfixed *params);
GL_API void GL_APIENTRY glGetMaterialxvOES(GLenum face, GLenum pname,
                                           GLfixed *params);
GL_API void GL_APIENTRY glGetTexEnvxvOES(GLenum target, GLenum pname,
                                         GLfixed *params);
GL_API void GL_APIENTRY glGetTexParameterxvOES(GLenum target, GLenum pname,
                                               GLfixed *params);
GL_API void GL_APIENTRY glLightModelxOES(GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glLightModelxvOES(GLenum pname, const GLfixed *param);
GL_API void GL_APIENTRY glLightxOES(GLenum light, GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glLightxvOES(GLenum light, GLenum pname,
                                     const GLfixed *params);
GL_API void GL_APIENTRY glLineWidthxOES(GLfixed width);
GL_API void GL_APIENTRY glLoadMatrixxOES(const GLfixed *m);
GL_API void GL_APIENTRY glMaterialxOES(GLenum face, GLenum pname,
                                       GLfixed param);
GL_API void GL_APIENTRY glMaterialxvOES(GLenum face, GLenum pname,
                                        const GLfixed *param);
GL_API void GL_APIENTRY glMultMatrixxOES(const GLfixed *m);
GL_API void GL_APIENTRY glMultiTexCoord4xOES(GLenum texture, GLfixed s,
                                             GLfixed t, GLfixed r, GLfixed q);
GL_API void GL_APIENTRY glNormal3xOES(GLfixed nx, GLfixed ny, GLfixed nz);
GL_API void GL_APIENTRY glOrthoxOES(GLfixed l, GLfixed r, GLfixed b, GLfixed t,
                                    GLfixed n, GLfixed f);
GL_API void GL_APIENTRY glPointParameterxOES(GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glPointParameterxvOES(GLenum pname,
                                              const GLfixed *params);
GL_API void GL_APIENTRY glPointSizexOES(GLfixed size);
GL_API void GL_APIENTRY glPolygonOffsetxOES(GLfixed factor, GLfixed units);
GL_API void GL_APIENTRY glRotatexOES(GLfixed angle, GLfixed x, GLfixed y,
                                     GLfixed z);
GL_API void GL_APIENTRY glSampleCoveragexOES(GLclampf value, GLboolean invert);
GL_API void GL_APIENTRY glScalexOES(GLfixed x, GLfixed y, GLfixed z);
GL_API void GL_APIENTRY glTexEnvxOES(GLenum target, GLenum pname,
                                     GLfixed param);
GL_API void GL_APIENTRY glTexEnvxvOES(GLenum target, GLenum pname,
                                      const GLfixed *params);
GL_API void GL_APIENTRY glTexParameterxOES(GLenum target, GLenum pname,
                                           GLfixed param);
GL_API void GL_APIENTRY glTexParameterxvOES(GLenum target, GLenum pname,
                                            const GLfixed *params);
GL_API void GL_APIENTRY glTranslatexOES(GLfixed x, GLfixed y, GLfixed z);

#ifdef __cplusplus
}
#endif

#endif /* GL_FIXED_POINT_H */
