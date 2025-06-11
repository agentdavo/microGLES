#include "gl_fragment.h"
#include "../gl_logger.h"
#include "../gl_context.h"
#include "../gl_memory_tracker.h"
#include "gl_framebuffer.h"
#include <string.h>
#include <math.h>

static _Thread_local TextureState local_tex[2];
static _Thread_local unsigned local_tex_ver[2];
static _Thread_local BlendState local_blend;
static _Thread_local unsigned local_blend_ver;
static _Thread_local FogState local_fog;
static _Thread_local unsigned local_fog_ver;

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
	unsigned fv = atomic_load(&ctx->fog.version);
	if (fv != local_fog_ver) {
		memcpy(&local_fog, &ctx->fog, sizeof(FogState));
		local_fog_ver = fv;
	}
}

void pipeline_shade_fragment(Fragment *frag, Framebuffer *fb)
{
	update_state();
	if (local_fog.enabled) {
		float factor = 1.0f;
		switch (local_fog.mode) {
		case GL_LINEAR:
			factor = (local_fog.end - frag->depth) /
				 (local_fog.end - local_fog.start);
			break;
		case GL_EXP:
			factor = expf(-local_fog.density * frag->depth);
			break;
		case GL_EXP2:
			factor = expf(-local_fog.density * local_fog.density *
				      frag->depth * frag->depth);
			break;
		}
		if (factor < 0.0f)
			factor = 0.0f;
		if (factor > 1.0f)
			factor = 1.0f;
		for (int i = 0; i < 3; ++i)
			((float *)&frag->color)[i] =
				((float *)&frag->color)[i] * factor +
				local_fog.color[i] * (1.0f - factor);
	}
	framebuffer_set_pixel(fb, frag->x, frag->y, frag->color, frag->depth);
}

void process_fragment_job(void *task_data)
{
	FragmentJob *job = (FragmentJob *)task_data;
	pipeline_shade_fragment(&job->frag, job->fb);
	MT_FREE(job, STAGE_FRAGMENT);
}
