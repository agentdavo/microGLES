#include "benchmark.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_thread.h"
#include "gl_memory_tracker.h"
#include "matrix_utils.h"
#include <GLES/gl.h>
#include <stdlib.h>
#include <time.h>

static const GLfloat cube_vertices[] = {
	-0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,	 0.5f,	-0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f,
	-0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,	 -0.5f, 0.5f,  -0.5f,
	0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,	 0.5f,	-0.5f, -0.5f,
	0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f,
	0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,	-0.5f, 0.5f,
	0.5f,  -0.5f, 0.5f,  -0.5f, 0.5f,  -0.5f, 0.5f,	 0.5f,	0.5f,  0.5f,
	-0.5f, 0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,	 -0.5f, -0.5f
};
static const GLfloat cube_normals[] = {
	0, 0,  -1, 0, 0,  -1, 0,  0, -1, 0,  0, -1, 0,	0,  1, 0,  0,  1,
	0, 0,  1,  0, 0,  1,  -1, 0, 0,	 -1, 0, 0,  -1, 0,  0, -1, 0,  0,
	1, 0,  0,  1, 0,  0,  1,  0, 0,	 1,  0, 0,  0,	-1, 0, 0,  -1, 0,
	0, -1, 0,  0, -1, 0,  0,  1, 0,	 0,  1, 0,  0,	1,  0, 0,  1,  0
};
static const GLfloat cube_tex[] = { 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0,
				    1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1,
				    0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0,
				    1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1 };
static const GLubyte cube_indices[] = {
	0,  1,	2,  0,	2,  3, /* Front */
	4,  5,	6,  4,	6,  7, /* Back */
	8,  9,	10, 8,	10, 11, /* Left */
	12, 13, 14, 12, 14, 15, /* Right */
	16, 17, 18, 16, 18, 19, /* Top */
	20, 21, 22, 20, 22, 23 /* Bottom */
};

void run_stress_test(Framebuffer *fb, BenchmarkResult *result, bool stream_fb,
		     int frames)
{
	const int cube_count = 1000000;
	const int face_pixels = 64 * 64; /* approximate pixels per face */
	GLuint tex[2];
	GLubyte *pix = tracked_malloc(face_pixels * 4);
	for (int i = 0; i < face_pixels * 4; ++i)
		pix[i] = (GLubyte)(rand() & 0xFF);
	glGenTextures(2, tex);
	for (int i = 0; i < 2; ++i) {
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA,
			     GL_UNSIGNED_BYTE, pix);
	}
	tracked_free(pix, face_pixels * 4);

	/* Setup vertex arrays */
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, cube_vertices);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, cube_normals);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, cube_tex);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,
		       (GLfloat[]){ 0.1f, 0.1f, 0.1f, 1 });
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (GLfloat[]){ 0.8f, 0.2f, 0.2f, 1 });
	glLightfv(GL_LIGHT0, GL_POSITION, (GLfloat[]){ 1, 1, 2, 0 });
	glLightfv(GL_LIGHT1, GL_DIFFUSE, (GLfloat[]){ 0.2f, 0.8f, 0.2f, 1 });
	glLightfv(GL_LIGHT1, GL_POSITION, (GLfloat[]){ -1, -1, 2, 0 });

	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	mat4 model;
	for (int f = 0; f < frames; ++f) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < cube_count; ++i) {
			float x = (float)(i % 1000) / 500.0f - 1.0f;
			float y = (float)((i / 1000) % 1000) / 500.0f - 1.0f;
			float z = -2.0f - (float)i / cube_count * 10.0f;
			mat4_identity(&model);
			mat4_translate(&model, x, y, z);
			mat4_rotate_x(&model, (float)(i & 0xFF));
			mat4_rotate_y(&model, (float)((i >> 8) & 0xFF));
			glLoadMatrixf(model.data);
			glBindTexture(GL_TEXTURE_2D, tex[i & 1]);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE,
				       cube_indices);
		}
		if (stream_fb) {
			framebuffer_stream_rgba(fb, stdout);
			fflush(stdout);
		}
	}
	clock_t end = clock();
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_LIGHTING);
	glDeleteTextures(2, tex);

	compute_result(start, end, result);
	double secs = (double)(end - start) / CLOCKS_PER_SEC;
	result->pixels_per_second =
		(double)cube_count * 6.0 * face_pixels / secs;
	LOG_INFO("Stress Test: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
	LOG_INFO("Stress Fill Rate: %.2f MP/s",
		 result->pixels_per_second / 1e6);
}
