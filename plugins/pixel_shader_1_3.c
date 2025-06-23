#include "plugin.h"
#include "pipeline/gl_fragment.h"
#include "gl_thread.h"
#include <stdint.h>

static void noop_task(void *data)
{
	(void)data;
}

static void pixel_shader_1_3(void *job)
{
	if (!job)
		return;

	FragmentJob *fj = (FragmentJob *)job;
	if (!fj->fb)
		return; /* likely a tile job */

	Fragment *f = &fj->frag;
	uint8_t r = (f->color >> 16) & 0xFF;
	uint8_t g = (f->color >> 8) & 0xFF;
	uint8_t b = f->color & 0xFF;
	uint8_t a = (f->color >> 24) & 0xFF;
	uint8_t gray = (uint8_t)((r + g + b) / 3);
	f->color = ((uint32_t)a << 24) | ((uint32_t)gray << 16) |
		   ((uint32_t)gray << 8) | gray;

	if (f->x == 0 && f->y == 0)
		thread_pool_submit(noop_task, NULL, STAGE_FRAGMENT);
}

PLUGIN_REGISTER(STAGE_FRAGMENT, pixel_shader_1_3)

int pixel_shader_1_3_link;
