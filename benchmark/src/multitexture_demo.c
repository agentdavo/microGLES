#include "benchmark.h"
#include "gl_texture.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include <string.h>

void run_multitexture_demo(Framebuffer *fb, BenchmarkResult *result)
{
	const int size = 64;
	const int frames = 100;
	GLubyte *tex1 = tracked_malloc(size * size * 4);
	GLubyte *tex2 = tracked_malloc(size * size * 4);
	memset(tex1, 0xFF, size * size * 4);
	memset(tex2, 0x80, size * size * 4);

	TextureOES *t1 = CreateTextureOES(GL_TEXTURE_2D_OES, GL_RGBA8_OES, size,
					  size, GL_TRUE);
	TextureOES *t2 = CreateTextureOES(GL_TEXTURE_2D_OES, GL_RGBA8_OES, size,
					  size, GL_TRUE);
	if (t1)
		TexImage2DOES(t1, 0, GL_RGBA8_OES, size, size, GL_RGBA,
			      GL_UNSIGNED_BYTE, tex1);
	if (t2)
		TexImage2DOES(t2, 0, GL_RGBA8_OES, size, size, GL_RGBA,
			      GL_UNSIGNED_BYTE, tex2);

	GLfloat verts[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
	GLfloat uv[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

	framebuffer_clear(fb, 0x00000000u, 1.0f);
	clock_t start = clock();
	for (int f = 0; f < frames; ++f) {
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		if (t1)
			glBindTexture(GL_TEXTURE_2D, t1->id);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glClientActiveTexture(GL_TEXTURE0);
		glTexCoordPointer(2, GL_FLOAT, 0, uv);

		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		if (t2)
			glBindTexture(GL_TEXTURE_2D, t2->id);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glClientActiveTexture(GL_TEXTURE1);
		glTexCoordPointer(2, GL_FLOAT, 0, uv);

		glVertexPointer(2, GL_FLOAT, 0, verts);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	clock_t end = clock();

	if (t1)
		tracked_free(t1, sizeof(TextureOES));
	if (t2)
		tracked_free(t2, sizeof(TextureOES));
	tracked_free(tex1, size * size * 4);
	tracked_free(tex2, size * size * 4);

	compute_result(start, end, result);
	double secs = (double)(end - start) / CLOCKS_PER_SEC;
	result->pixels_per_second = (double)(size * size * frames) / secs;
	LOG_INFO("Multitexture Demo: %.2f MP/s",
		 result->pixels_per_second / 1e6);
}
