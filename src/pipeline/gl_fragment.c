#include "gl_fragment.h"
#include "../gl_logger.h"
#include "../gl_context.h"
#include "../gl_memory_tracker.h"
#include "gl_framebuffer.h"
#include <string.h>

static _Thread_local TextureState local_tex[2];
static _Thread_local unsigned local_tex_ver[2];
static _Thread_local BlendState local_blend;
static _Thread_local unsigned local_blend_ver;

static void update_state(void)
{
	RenderContext *ctx = context_get();
	for (int i = 0; i < 2; ++i) {
		unsigned v = atomic_load(&ctx->texture_env[i].version);
		if (v != local_tex_ver[i]) {
			memcpy(&local_tex[i], &ctx->texture_env[i],
			       sizeof(TextureState));
			local_tex_ver[i] = v;
		}
	}
	unsigned bv = atomic_load(&ctx->blend.version);
	if (bv != local_blend_ver) {
		memcpy(&local_blend, &ctx->blend, sizeof(BlendState));
		local_blend_ver = bv;
	}
}

void pipeline_shade_fragment(Fragment *frag, Framebuffer *fb)
{
	update_state();
	framebuffer_set_pixel(fb, frag->x, frag->y, frag->color, frag->depth);
}

void process_fragment_job(void *task_data)
{
	FragmentJob *job = (FragmentJob *)task_data;
	pipeline_shade_fragment(&job->frag, job->fb);
	MT_FREE(job, STAGE_FRAGMENT);
}
