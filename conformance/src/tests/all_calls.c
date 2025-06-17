#include "tests.h"
#include <string.h>

#define FIX(x) ((GLfixed)((x) * 65536))

int test_all_entrypoints(void)
{
	GLfloat eq[4] = { 1.f, 0.f, 0.f, 0.f };
	GLfloat color[4] = { 1.f, 1.f, 1.f, 1.f };
	GLfloat mat[16] = { 1.f, 0, 0,	 0, 0, 1.f, 0, 0,
			    0,	 0, 1.f, 0, 0, 0,   0, 1.f };
	GLfloat att[3] = { 1.f, 0.f, 0.f };
	GLfixed eqx[4] = { FIX(1.f), 0, 0, 0 };
	GLfixed matx[16] = {
		FIX(1.f), 0, 0,	       0, 0, FIX(1.f), 0, 0,
		0,	  0, FIX(1.f), 0, 0, 0,	       0, FIX(1.f)
	};
	GLuint buf = 0, tex = 0;
	GLint idata = 0;
	GLubyte ubuf[4] = { 0 };
	const GLubyte *ver = glGetString(GL_VERSION);

	glAlphaFunc(GL_ALWAYS, 0.5f);
	glBlendFunc(GL_ONE, GL_ZERO);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClearDepthf(1.f);
	glClipPlanef(GL_CLIP_PLANE0, eq);
	glClipPlanex(GL_CLIP_PLANE0, eqx);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glDepthRangef(0.f, 1.f);
	glFogf(GL_FOG_DENSITY, 1.f);
	glFogfv(GL_FOG_COLOR, color);
	glFogx(GL_FOG_DENSITY, FIX(1));
	glFogxv(GL_FOG_COLOR, eqx);
	glFrustumf(-1.f, 1.f, -1.f, 1.f, 1.f, 10.f);
	glGetClipPlanef(GL_CLIP_PLANE0, eq);
	glGetClipPlanex(GL_CLIP_PLANE0, eqx);
	glGetFloatv(GL_MODELVIEW_MATRIX, mat);
	glGetFixedv(GL_MODELVIEW_MATRIX, eqx);
	glGetLightfv(GL_LIGHT0, GL_AMBIENT, color);
	glGetLightxv(GL_LIGHT0, GL_AMBIENT, eqx);
	glGetMaterialfv(GL_FRONT, GL_AMBIENT, color);
	glGetMaterialxv(GL_FRONT, GL_AMBIENT, eqx);
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &idata);
	glGetTexEnvxv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, eqx);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &color[0]);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.f);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color);
	glLightModelx(GL_LIGHT_MODEL_TWO_SIDE, FIX(0));
	glLightModelxv(GL_LIGHT_MODEL_AMBIENT, eqx);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.f);
	glLightfv(GL_LIGHT0, GL_POSITION, color);
	glLightx(GL_LIGHT0, GL_CONSTANT_ATTENUATION, FIX(1));
	glLightxv(GL_LIGHT0, GL_POSITION, eqx);
	glLineWidth(1.f);
	glLoadMatrixf(mat);
	glLoadMatrixx(matx);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.f);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
	glMaterialx(GL_FRONT, GL_SHININESS, FIX(0));
	glMaterialxv(GL_FRONT, GL_DIFFUSE, eqx);
	glMultMatrixf(mat);
	glMultMatrixx(matx);
	glMultiTexCoord4f(GL_TEXTURE0, 0.f, 0.f, 0.f, 1.f);
	glMultiTexCoord4x(GL_TEXTURE0, FIX(0), FIX(0), FIX(0), FIX(1));
	glNormal3f(0.f, 0.f, 1.f);
	glNormal3x(FIX(0), FIX(0), FIX(1));
	glOrthof(-1.f, 1.f, -1.f, 1.f, 1.f, 10.f);
	glPointParameterf(GL_POINT_SIZE_MIN, 1.f);
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, att);
	glPointParameterx(GL_POINT_SIZE_MIN, FIX(1));
	glPointParameterxv(GL_POINT_DISTANCE_ATTENUATION, eqx);
	glPointSize(1.f);
	glPointSizex(FIX(1));
	glPointSizePointerOES(GL_FLOAT, 0, color);
	glPolygonOffset(0.5f, 0.5f);
	glPolygonOffsetx(FIX(1), FIX(1));
	glRotatef(45.f, 0.f, 1.f, 0.f);
	glScalef(1.f, 1.f, 1.f);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GLfloat wrap = (GLfloat)GL_CLAMP_TO_EDGE;
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrap);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, FIX(1));
	GLint ip = GL_NEAREST_MIPMAP_LINEAR;
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &ip);
	GLfixed xv[4] = { FIX(0.f), FIX(0.f), FIX(0.f), FIX(0.f) };
	glTexParameterxv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, xv);
	GLint out;
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &out);
	glGetTexParameterxv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, xv);
	glTranslatef(1.f, 2.f, 3.f);
	glActiveTexture(GL_TEXTURE0);
	glAlphaFuncx(GL_ALWAYS, FIX(0.5));
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(idata), &idata, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(idata), &idata);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColorx(FIX(0), FIX(0), FIX(0), FIX(1));
	glClearDepthx(FIX(1));
	glClearStencil(0);
	glClientActiveTexture(GL_TEXTURE0);
	glColor4ub(255, 255, 255, 255);
	glColor4x(FIX(1), FIX(1), FIX(1), FIX(1));
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glColorPointer(4, GL_FLOAT, 0, color);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, 0, NULL);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGB, 0,
				  NULL);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
	glCullFace(GL_BACK);
	glDeleteBuffers(1, &buf);
	glDeleteTextures(1, &tex);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glDepthRangex(FIX(0), FIX(1));
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_TRIANGLES, 0, 0);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, ubuf);
	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_COLOR_ARRAY);
	glFinish();
	glFlush();
	glFrontFace(GL_CCW);
	glFrustumx(FIX(-1), FIX(1), FIX(-1), FIX(1), FIX(1), FIX(10));
	GLboolean bval[4];
	glGetBooleanv(GL_COLOR_WRITEMASK, bval);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &idata);
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	glGetPointerv(GL_VERTEX_ARRAY_POINTER, (void **)&eqx);
	glHint(GL_FOG_HINT, GL_NICEST);
	glIsBuffer(buf);
	glIsEnabled(GL_CULL_FACE);
	glIsTexture(tex);
	glLineWidthx(FIX(1));
	glLoadIdentity();
	glLogicOp(GL_COPY);
	glMatrixMode(GL_MODELVIEW);
	glNormalPointer(GL_FLOAT, 0, color);
	glOrthox(FIX(-1), FIX(1), FIX(-1), FIX(1), FIX(1), FIX(10));
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glPopMatrix();
	glPushMatrix();
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, ubuf);
	glRotatex(FIX(90), FIX(1), 0, 0);
	glSampleCoverage(1.0f, GL_FALSE);
	glSampleCoveragex(FIX(1), GL_FALSE);
	glScalex(FIX(1), FIX(1), FIX(1));
	glScissor(0, 0, 1, 1);
	glShadeModel(GL_SMOOTH);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilMask(0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glTexCoordPointer(2, GL_FLOAT, 0, color);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &idata);
	glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvxv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, eqx);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
			ubuf);
	glTranslatex(FIX(1), FIX(2), FIX(3));
	glVertexPointer(3, GL_FLOAT, 0, color);
	glViewport(0, 0, 1, 1);

	while (glGetError() != GL_NO_ERROR)
		;

	return 1;
}

static const struct Test tests[] = {
	{ "all_entrypoints", test_all_entrypoints },
};

const struct Test *get_all_calls_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
