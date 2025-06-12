#include "tests.h"
#include "util.h"
#include <GLES/glext.h>

int test_point_sprite(void)
{
	GLfloat verts[8] = { 8, 8, 24, 8, 8, 24, 24, 24 };
	GLfloat sizes[4] = { 4, 8, 12, 16 };
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);
	glPointSizePointerOES(GL_FLOAT, 0, sizes);
	glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
	glEnable(GL_POINT_SPRITE_OES);
	glColor4f(1, 0, 0, 1);
	glDrawArrays(GL_POINTS, 0, 4);
	glGetError();
	unsigned char buf[64 * 64 * 4];
	glReadPixels(0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	write_rgba("point_sprite_out.rgba", buf, 64, 64);
	return compare_rgba("gold/point_sprite.rgba", "point_sprite_out.rgba");
}

static const struct Test tests[] = { { "point_sprite", test_point_sprite } };

const struct Test *get_point_sprite_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
