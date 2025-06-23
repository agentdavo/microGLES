#include "tests.h"
#include "plugin.h"
#include "gl_context.h"

static int cb_calls;

static void add_one(void *job)
{
	int *val = (int *)job;
	if (val)
		*val += 1;
	cb_calls++;
}

static void add_two(void *job)
{
	int *val = (int *)job;
	if (val)
		*val += 2;
	cb_calls++;
}

int test_plugin_callbacks(void)
{
	int value = 0;
	cb_calls = 0;
	plugin_register(STAGE_STEAL, add_one, "add_one");
	plugin_register(STAGE_STEAL, add_two, "add_two");
	plugin_invoke(STAGE_STEAL, &value);
	return cb_calls == 2 && value == 3;
}

int test_texture_decode_plugin(void)
{
	GLuint tex = texture_decode("gold/red.ktx");
	if (!tex)
		return 0;
	TextureOES *obj = context_find_texture(tex);
	int ok = obj && obj->mip_width[0] == 4 && obj->mip_height[0] == 4;
	glDeleteTextures(1, &tex);
	return ok;
}

static const struct Test tests[] = {
	{ "plugin_callbacks", test_plugin_callbacks },
	{ "texture_decode_plugin", test_texture_decode_plugin },
};

const struct Test *get_plugin_tests(size_t *count)
{
	*count = sizeof(tests) / sizeof(tests[0]);
	return tests;
}
