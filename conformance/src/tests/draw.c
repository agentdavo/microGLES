#include "tests.h"
#include "util.h"
#include "gl_utils.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int test_framebuffer_colors(void)
{
	const int w = 4;
	const int h = 4;
	unsigned char *buf = (unsigned char *)tracked_malloc(w * h * 4);
	if (!buf)
		return 0;
	struct ColorCase {
		const char *name;
		unsigned char r, g, b;
	} cases[] = { { "red.rgba", 255, 0, 0 },
		      { "green.rgba", 0, 255, 0 },
		      { "blue.rgba", 0, 0, 255 },
		      { "black.rgba", 0, 0, 0 },
		      { "white.rgba", 255, 255, 255 } };
	int pass = 1;
	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
		for (int p = 0; p < w * h; ++p) {
			buf[p * 4 + 0] = cases[i].r;
			buf[p * 4 + 1] = cases[i].g;
			buf[p * 4 + 2] = cases[i].b;
			buf[p * 4 + 3] = 255;
		}
		char out[64];
		snprintf(out, sizeof(out), "%s_out.rgba", cases[i].name);
		char expected[128];
		snprintf(expected, sizeof(expected), "conformance/gold/%s",
			 cases[i].name);
		if (access(expected, F_OK) != 0)
			snprintf(expected, sizeof(expected), "gold/%s",
				 cases[i].name);
		if (!write_rgba(out, buf, w, h) || !compare_rgba(expected, out))
			pass = 0;
	}
	tracked_free(buf, w * h * 4);
	return pass;
}

static const struct Test tests[] = {
	{ "framebuffer_colors", test_framebuffer_colors },
};

const struct Test *get_draw_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
