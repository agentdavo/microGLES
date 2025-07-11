#include "plugin.h"
#include <stddef.h>
#include <string.h>

#define MAX_PLUGINS_PER_STAGE 4

static stage_plugin_fn g_plugins[STAGE_COUNT][MAX_PLUGINS_PER_STAGE];
static const char *g_plugin_names[STAGE_COUNT][MAX_PLUGINS_PER_STAGE];
static int g_plugin_count[STAGE_COUNT];

#define MAX_TEXTURE_DECODERS 4
static texture_decoder_fn g_decoders[MAX_TEXTURE_DECODERS];
static int g_dec_count;

extern int ktx_decoder_link;
static void *volatile force_link_ktx __attribute__((used)) = &ktx_decoder_link;
extern int vertex_shader_1_1_link;
static void *volatile force_link_vs
	__attribute__((used)) = &vertex_shader_1_1_link;
extern int pixel_shader_1_3_link;
static void *volatile force_link_ps
	__attribute__((used)) = &pixel_shader_1_3_link;

void plugin_register(stage_tag_t stage, stage_plugin_fn fn, const char *name)
{
	if (!fn || !name || stage < 0 || stage >= STAGE_COUNT)
		return;
	int count = g_plugin_count[stage];
	if (count < MAX_PLUGINS_PER_STAGE) {
		g_plugins[stage][count] = fn;
		g_plugin_names[stage][count] = name;
		g_plugin_count[stage] = count + 1;
	}
}

void plugin_invoke(stage_tag_t stage, void *job)
{
	if (stage < 0 || stage >= STAGE_COUNT)
		return;
	for (int i = 0; i < g_plugin_count[stage]; ++i) {
		stage_plugin_fn fn = g_plugins[stage][i];
		if (fn)
			fn(job);
	}
}

void plugin_submit(task_function_t fn, void *data, stage_tag_t stage)
{
	thread_pool_submit(fn, data, stage);
}

void texture_decoder_register(texture_decoder_fn fn)
{
	if (!fn)
		return;
	if (g_dec_count < MAX_TEXTURE_DECODERS)
		g_decoders[g_dec_count++] = fn;
}

GLuint texture_decode(const char *file)
{
	GLuint tex = 0;
	for (int i = 0; i < g_dec_count; ++i) {
		texture_decoder_fn fn = g_decoders[i];
		if (fn && fn(file, &tex))
			return tex;
	}
	return 0;
}

const char *plugin_list(void)
{
	static char list[256];
	size_t pos = 0;
	list[0] = '\0';
	for (int s = 0; s < STAGE_COUNT; ++s) {
		for (int i = 0; i < g_plugin_count[s]; ++i) {
			const char *name = g_plugin_names[s][i];
			if (!name)
				continue;
			size_t len = strlen(name);
			if (pos != 0 && pos + 1 < sizeof(list))
				list[pos++] = ' ';
			if (pos + len >= sizeof(list))
				return list; /* truncate */
			memcpy(&list[pos], name, len);
			pos += len;
		}
	}
	if (pos < sizeof(list))
		list[pos] = '\0';
	else
		list[sizeof(list) - 1] = '\0';
	return list;
}
