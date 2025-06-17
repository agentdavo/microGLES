#include "benchmark.h"
#include "gl_logger.h"
#include "gl_thread.h"
#define GL_GLEXT_PROTOTYPES
#include "gl_context.h"
#include "pipeline/gl_framebuffer.h"
#include "gl_api_fbo.h"
#include <GLES/glext.h>
#include <string.h>

#define CHECK_ERROR()                                                          \
	do {                                                                   \
		GLenum err = glGetError();                                     \
		if (err != GL_NO_ERROR)                                        \
			LogMessage(LOG_LEVEL_ERROR,                            \
				   "GL Error 0x%X at line %d", err, __LINE__); \
	} while (0)

static const GLfloat vertices[] = { -0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
				    0.0f,  0.0f,  0.5f, 0.0f };
static const GLfloat normals[] = { 0, 0, 1, 0, 0, 1, 0, 0, 1 };
static const GLfloat texcoords[] = { 0, 0, 1, 0, 0.5f, 1 };
static const GLubyte colors[] = {
	255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255
};
static const GLfloat point_sizes[] = { 5.f, 5.f, 5.f };
static const GLushort indices[] = { 0, 1, 2 };
static uint32_t texture_data[32 * 32];
static uint32_t texture_update[16 * 16];
static uint32_t fbo_texture_data[32 * 32];

void run_milestone2(Framebuffer *fb, BenchmarkResult *result)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustumf(-1, 1, -1, 1, 1, 10);
	glOrthof(-1, 1, -1, 1, 1, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -5);
	glRotatef(45, 1, 0, 0);
	glScalef(1.5f, 1.5f, 1.5f);
	/* These matrix operations trigger GL errors on some platforms and are
         * not required for the benchmark. */
	CHECK_ERROR();

	glViewport(0, 0, 320, 240);
	glDepthRangef(0, 1);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	CHECK_ERROR();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, (GLfloat[]){ 0.2f, 0.2f, 0.2f, 1 });
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (GLfloat[]){ 0.8f, 0.8f, 0.8f, 1 });
	glLightfv(GL_LIGHT0, GL_SPECULAR, (GLfloat[]){ 1, 1, 1, 1 });
	glLightfv(GL_LIGHT0, GL_POSITION, (GLfloat[]){ 0, 0, 1, 0 });
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,
		     (GLfloat[]){ 0.3f, 0.3f, 0.3f, 1 });
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
		     (GLfloat[]){ 0.7f, 0.7f, 0.7f, 1 });
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat[]){ 1, 1, 1, 1 });
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50);
	glShadeModel(GL_SMOOTH);
	CHECK_ERROR();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, normals);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	glColor4f(1, 1, 1, 1);
	glNormal3f(0, 0, 1);
	glClientActiveTexture(GL_TEXTURE0);
	CHECK_ERROR();

	GLuint tex[2];
	glGenTextures(2, tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	memset(texture_data, 0xFF0000FF, sizeof(texture_data));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, texture_data);
	memset(texture_update, 0x00FF00FF, sizeof(texture_update));
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGBA,
			GL_UNSIGNED_BYTE, texture_update);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
		   (GLfloat[]){ 0.5f, 0.5f, 0.5f, 1 });
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, texture_data);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	glEnable(GL_TEXTURE_2D);
	CHECK_ERROR();

	glEnable(GL_FOG);
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_DENSITY, 0.5f);
	glFogf(GL_FOG_START, 1);
	glFogf(GL_FOG_END, 5);
	glFogfv(GL_FOG_COLOR, (GLfloat[]){ 0.5f, 0.5f, 0.5f, 1 });
	glHint(GL_FOG_HINT, GL_NICEST);
	CHECK_ERROR();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	CHECK_ERROR();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	CHECK_ERROR();

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF);
	CHECK_ERROR();

	glClearColor(0, 0, 0, 1);
	glClearDepthf(1);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT);
	glEnable(GL_SCISSOR_TEST);
	glScissor(50, 50, 100, 100);
	CHECK_ERROR();

	glLineWidth(2);
	glPointSize(5);
	CHECK_ERROR();

	glEnable(GL_POINT_SPRITE_OES);
	glTexEnvf(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
	glPointSizePointerOES(GL_FLOAT, 0, point_sizes);
	CHECK_ERROR();

	GLuint fbo, fbo_tex;
	glGenFramebuffersOES(1, &fbo);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);
	glGenTextures(1, &fbo_tex);
	glBindTexture(GL_TEXTURE_2D, fbo_tex);
	memset(fbo_texture_data, 0x0000FFFF, sizeof(fbo_texture_data));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, fbo_texture_data);
	glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,
				  GL_TEXTURE_2D, fbo_tex, 0);
	GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
	if (status != GL_FRAMEBUFFER_COMPLETE_OES)
		LogMessage(LOG_LEVEL_ERROR, "FBO incomplete: 0x%X", status);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
	CHECK_ERROR();

	glDrawTexiOES(100, 100, 0, 32, 32);
	glDrawTexfvOES((GLfloat[]){ 100, 100, 0, 32, 32 });
	CHECK_ERROR();

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, indices);
	glFinish();
	glFlush();
	CHECK_ERROR();

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	CHECK_ERROR();

	glDeleteTextures(2, tex);
	glDeleteTextures(1, &fbo_tex);
	glDeleteFramebuffersOES(1, &fbo);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_POINT_SPRITE_OES);
	CHECK_ERROR();

	uint32_t pixel = framebuffer_get_pixel(fb, 160, 160);
	LogMessage(LOG_LEVEL_INFO, "Center pixel: 0x%08X", pixel);

	(void)fb;
	(void)result;
}
