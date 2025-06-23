#include "plugin.h"
#include <stddef.h>

#define MAX_PLUGINS_PER_STAGE 4

static stage_plugin_fn g_plugins[STAGE_COUNT][MAX_PLUGINS_PER_STAGE];
static int g_plugin_count[STAGE_COUNT];

void plugin_register(stage_tag_t stage, stage_plugin_fn fn)
{
	if (!fn || stage < 0 || stage >= STAGE_COUNT)
		return;
	int count = g_plugin_count[stage];
	if (count < MAX_PLUGINS_PER_STAGE) {
		g_plugins[stage][count] = fn;
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
