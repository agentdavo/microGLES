#include "benchmark.h"
#include "gl_types.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_thread.h"
#include "matrix_utils.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

void run_spinning_cubes(Framebuffer *fb, BenchmarkResult *result)
{
	const int cube_count = 12;
	const int frames = 100;
	const int face_pixels = 64 * 64; // approximate pixels per face
	GLubyte *tex = tracked_malloc(face_pixels * 4);
	memset(tex, 0xAA, face_pixels * 4);
	GLuint t;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, tex);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, cube_vertices);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, cube_normals);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, cube_tex);

	glEnable(GL_FOG);
	glFogf(GL_FOG_DENSITY, 0.5f);

	mat4 model;
	mat4_identity(&model);

	double pixels_drawn = 0.0;
	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();
	clock_t start = clock();
	for (int f = 0; f < frames; ++f) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int c = 0; c < cube_count; ++c) {
			mat4_rotate_x(&model, 2.0f);
			mat4_rotate_y(&model, 3.0f);
			mat4_rotate_z(&model, 1.0f);
			glLoadMatrixf(model.data);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE,
				       cube_indices);
			pixels_drawn += 6 * face_pixels; // 6 faces per cube
		}
	}
	clock_t end = clock();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDeleteTextures(1, &t);
	tracked_free(tex, face_pixels * 4);

	compute_result(start, end, result);
	double secs = (double)(end - start) / CLOCKS_PER_SEC;
	result->pixels_per_second = pixels_drawn / secs;
	LOG_INFO("Spinning Cubes Fill Rate: %.2f MP/s",
		 result->pixels_per_second / 1e6);
}
