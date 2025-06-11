#include "gl_fragment.h"
#include "../gl_logger.h"
#include "../gl_context.h"
#include "../gl_memory_tracker.h"
#include "../gl_types.h"
#include "gl_framebuffer.h"
#include <string.h>
#include <math.h>

static uint32_t modulate(uint32_t a, uint32_t c)
{
	uint8_t ar = a & 0xFF;
	uint8_t ag = (a >> 8) & 0xFF;
	uint8_t ab = (a >> 16) & 0xFF;
	uint8_t aa = (a >> 24) & 0xFF;
	uint8_t br = c & 0xFF;
	uint8_t bg = (c >> 8) & 0xFF;
	uint8_t bb = (c >> 16) & 0xFF;
	uint8_t ba = (c >> 24) & 0xFF;
	uint8_t r = (ar * br) / 255;
	uint8_t g = (ag * bg) / 255;
	uint8_t b = (ab * bb) / 255;
	uint8_t aout = (aa * ba) / 255;
	return r | (g << 8) | (b << 16) | ((uint32_t)aout << 24);
}
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
		if (local_tex[i].bound_texture == 0)
			local_tex[i].env_mode = GL_REPLACE;
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
	TextureOES *tex = context_find_texture(local_tex[0].bound_texture);
	if (tex && tex->data) {
		float u = (float)frag->x / fb->width;
		float v = (float)frag->y / fb->height;
		if (tex->wrap_s == GL_REPEAT)
			u -= floorf(u);
		else
			u = fminf(fmaxf(u, 0.0f), 1.0f);
		if (tex->wrap_t == GL_REPEAT)
			v -= floorf(v);
		else
			v = fminf(fmaxf(v, 0.0f), 1.0f);
		float x = u * (tex->width - 1);
		float y = v * (tex->height - 1);
		int ix = (int)roundf(x);
		int iy = (int)roundf(y);
		if (tex->min_filter == GL_LINEAR ||
		    tex->mag_filter == GL_LINEAR) {
			int ix1 = ix < tex->width - 1 ? ix + 1 : ix;
			int iy1 = iy < tex->height - 1 ? iy + 1 : iy;
			float fx = x - floorf(x);
			float fy = y - floorf(y);
			uint32_t c00 = tex->data[iy * tex->width + ix];
			uint32_t c10 = tex->data[iy * tex->width + ix1];
			uint32_t c01 = tex->data[iy1 * tex->width + ix];
			uint32_t c11 = tex->data[iy1 * tex->width + ix1];
			float r = ((c00 & 0xFF) * (1 - fx) * (1 - fy) +
				   (c10 & 0xFF) * fx * (1 - fy) +
				   (c01 & 0xFF) * (1 - fx) * fy +
				   (c11 & 0xFF) * fx * fy);
			float g = (((c00 >> 8) & 0xFF) * (1 - fx) * (1 - fy) +
				   ((c10 >> 8) & 0xFF) * fx * (1 - fy) +
				   ((c01 >> 8) & 0xFF) * (1 - fx) * fy +
				   ((c11 >> 8) & 0xFF) * fx * fy);
			float b = (((c00 >> 16) & 0xFF) * (1 - fx) * (1 - fy) +
				   ((c10 >> 16) & 0xFF) * fx * (1 - fy) +
				   ((c01 >> 16) & 0xFF) * (1 - fx) * fy +
				   ((c11 >> 16) & 0xFF) * fx * fy);
			uint32_t color = ((uint32_t)r & 0xFF) |
					 (((uint32_t)g & 0xFF) << 8) |
					 (((uint32_t)b & 0xFF) << 16) |
					 0xFF000000u;
			if (local_tex[0].env_mode == GL_MODULATE)
				frag->color = modulate(frag->color, color);
			else
				frag->color = color;
		} else {
			uint32_t color = tex->data[iy * tex->width + ix];
			if (local_tex[0].env_mode == GL_MODULATE)
				frag->color = modulate(frag->color, color);
			else
				frag->color = color;
		}
	}
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
