#include "benchmark.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_thread.h"
#include "gl_context.h"
#include <string.h>

void run_miletostone1(Framebuffer *fb, BenchmarkResult *result)
{
	GLfloat verts[] = { -0.5f, -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f,
			    0.5f,  0.5f,  0.0f, -0.5f, -0.5f, 0.0f,
			    0.5f,  0.5f,  0.0f, -0.5f, 0.5f,  0.0f };
	GLfloat texcoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	GLubyte pixels[64 * 64 * 4];
	memset(pixels, 255, sizeof(pixels));
	GLubyte pixels2[32 * 32 * 4];
	memset(pixels2, 128, sizeof(pixels2));
	GLuint tex[2];
	glGenTextures(2, tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, pixels);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, pixels2);

	framebuffer_clear_async(fb, 0x00000000u, 1.0f, 0);
	thread_pool_wait();

	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (GLfloat[]){ 1.f, 1.f, 1.f, 1.f });
	glEnable(GL_FOG);
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, (GLfloat[]){ 0.5f, 0.5f, 0.5f, 1.f });
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

	clock_t start = clock();
	for (int i = 0; i < 10; ++i) {
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	clock_t end = clock();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDeleteTextures(2, tex);

	compute_result(start, end, result);
	LOG_INFO("miletostone1: %.2f FPS, %.2f ms/frame", result->fps,
		 result->cpu_time_ms);
}
