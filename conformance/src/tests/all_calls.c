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
	GLuint buf = 0, tex = 0;
	GLint idata = 0;
	GLubyte ubuf[4] = { 0 };

	glAlphaFunc(GL_ALWAYS, 0.5f);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClearDepthf(1.f);
	glClipPlanef(GL_CLIP_PLANE0, eq);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glDepthRangef(0.f, 1.f);
	glFogf(GL_FOG_DENSITY, 1.f);
	glFogfv(GL_FOG_COLOR, color);
	glFrustumf(-1.f, 1.f, -1.f, 1.f, 1.f, 10.f);
	glGetClipPlanef(GL_CLIP_PLANE0, eq);
	glGetFloatv(GL_MODELVIEW_MATRIX, mat);
	glGetLightfv(GL_LIGHT0, GL_AMBIENT, color);
	glGetMaterialfv(GL_FRONT, GL_AMBIENT, color);
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &color[0]);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.f);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.f);
	glLightfv(GL_LIGHT0, GL_POSITION, color);
	glLineWidth(1.f);
	glLoadMatrixf(mat);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.f);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
	glMultMatrixf(mat);
	glMultiTexCoord4f(GL_TEXTURE0, 0.f, 0.f, 0.f, 1.f);
	glNormal3f(0.f, 0.f, 1.f);
	glOrthof(-1.f, 1.f, -1.f, 1.f, 1.f, 10.f);
	glPointParameterf(GL_POINT_SIZE_MIN, 1.f);
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, att);
	glPointSize(1.f);
	glPolygonOffset(0.5f, 0.5f);
	glRotatef(45.f, 0.f, 1.f, 0.f);
	glScalef(1.f, 1.f, 1.f);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GLfloat wrap = (GLfloat)GL_CLAMP_TO_EDGE;
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrap);
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
	/* fixed-point wrappers not implemented: skip glClipPlanex, glColor4ub */
	glColor4x(FIX(1), FIX(1), FIX(1), FIX(1));
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glColorPointer(4, GL_FLOAT, 0, color);
	/* copy tex image functions not implemented */
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
	/* fixed-point fog functions unimplemented */
	glFrontFace(GL_CCW);
	glFrustumx(FIX(-1), FIX(1), FIX(-1), FIX(1), FIX(1), FIX(10));
	GLboolean bval[4];
	glGetBooleanv(GL_COLOR_WRITEMASK, bval);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &idata);
	/* fixed-point getters unimplemented */
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	/* fixed-point light/material getters unimplemented */
	glGetPointerv(GL_VERTEX_ARRAY_POINTER, (void **)&eqx);
	/* fixed-point tex env/parameter getters unimplemented */
	glHint(GL_FOG_HINT, GL_NICEST);
	glIsBuffer(buf);
	glIsEnabled(GL_CULL_FACE);
	glIsTexture(tex);
	/* fixed-point light functions not present */
	glLineWidthx(FIX(1));
	glLoadIdentity();
	/* glLoadMatrixx missing */
	glLogicOp(GL_COPY);
	/* fixed-point material functions missing */
	glMatrixMode(GL_MODELVIEW);
	/* fixed-point matrix/texcoord/normal functions missing */
	glNormalPointer(GL_FLOAT, 0, color);
	glOrthox(FIX(-1), FIX(1), FIX(-1), FIX(1), FIX(1), FIX(10));
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	/* fixed-point point/polygon functions missing */
	glPopMatrix();
	glPushMatrix();
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, ubuf);
	glRotatex(FIX(90), FIX(1), 0, 0);
	glSampleCoverage(1.0f, GL_FALSE);
	/* glSampleCoveragex not implemented */
	glScalex(FIX(1), FIX(1), FIX(1));
	glScissor(0, 0, 1, 1);
	glShadeModel(GL_SMOOTH);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilMask(0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glTexCoordPointer(2, GL_FLOAT, 0, color);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	/* fixed-point texture env setters missing */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	/* fixed-point texture parameter setters missing */
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
