#include "gl_fragment.h"
#include "../gl_logger.h"
#include "../gl_context.h"
#include "texture_cache.h"
#include "../gl_thread.h"
#define PIPELINE_USE_GLSTATE 0
_Static_assert(PIPELINE_USE_GLSTATE == 0, "pipeline must not touch gl_state");
#include "../gl_memory_tracker.h"
#include "../pool.h"
#include "../plugin.h"
#include "../gl_types.h"
#include "gl_framebuffer.h"
#include "gl_raster.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "../c11_opt.h"

static uint32_t modulate(uint32_t a, uint32_t c)
{
	/* Colors are stored as AARRGGBB */
	uint8_t ar = (a >> 16) & 0xFF;
	uint8_t ag = (a >> 8) & 0xFF;
	uint8_t ab = a & 0xFF;
	uint8_t aa = (a >> 24) & 0xFF;

	uint8_t br = (c >> 16) & 0xFF;
	uint8_t bg = (c >> 8) & 0xFF;
	uint8_t bb = c & 0xFF;
	uint8_t ba = (c >> 24) & 0xFF;

	uint8_t r = (ar * br) / 255;
	uint8_t g = (ag * bg) / 255;
	uint8_t b = (ab * bb) / 255;
	uint8_t aout = (aa * ba) / 255;
	return ((uint32_t)aout << 24) | ((uint32_t)r << 16) |
	       ((uint32_t)g << 8) | (uint32_t)b;
}
static _Thread_local TextureState local_tex[2];
static _Thread_local unsigned local_tex_ver[2];
static _Thread_local BlendState local_blend;
static _Thread_local unsigned local_blend_ver;
static _Thread_local GLboolean local_blend_on;
static _Thread_local FogState local_fog;
static _Thread_local unsigned local_fog_ver;
static _Thread_local AlphaTestState local_alpha;
static _Thread_local unsigned local_alpha_ver;
static _Thread_local GLboolean tl_sprite_mode;
static _Thread_local GLfloat tl_sprite_cx;
static _Thread_local GLfloat tl_sprite_cy;
static _Thread_local GLfloat tl_sprite_size;

static float blend_factor(GLenum factor, float srcC, float dstC, float srcA,
			  float dstA)
{
	switch (factor) {
	case GL_ZERO:
		return 0.0f;
	case GL_ONE:
		return 1.0f;
	case GL_SRC_ALPHA:
		return srcA;
	case GL_ONE_MINUS_SRC_ALPHA:
		return 1.0f - srcA;
	case GL_DST_ALPHA:
		return dstA;
	case GL_ONE_MINUS_DST_ALPHA:
		return 1.0f - dstA;
	case GL_SRC_COLOR:
		return srcC;
	case GL_ONE_MINUS_SRC_COLOR:
		return 1.0f - srcC;
	case GL_DST_COLOR:
		return dstC;
	case GL_ONE_MINUS_DST_COLOR:
		return 1.0f - dstC;
	default:
		return 1.0f;
	}
}
static void update_state(void)
{
	RenderContext *ctx = GetCurrentContext();
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
	local_blend_on = ctx->blend_enabled;
	unsigned fv = atomic_load(&ctx->fog.version);
	if (fv != local_fog_ver) {
		memcpy(&local_fog, &ctx->fog, sizeof(FogState));
		local_fog_ver = fv;
	}
	unsigned av = atomic_load(&ctx->alpha_test.version);
	if (av != local_alpha_ver) {
		memcpy(&local_alpha, &ctx->alpha_test, sizeof(AlphaTestState));
		local_alpha_ver = av;
	}
}

void pipeline_shade_fragment(Fragment *frag, Framebuffer *fb)
{
	update_state();
	TextureOES *tex = context_find_texture(local_tex[0].bound_texture);
	texture_cache_t *cache = thread_get_texture_cache();
	if (tex && tex->levels[0]) {
		float u;
		float v;
		if (tl_sprite_mode) {
			float left = tl_sprite_cx - tl_sprite_size * 0.5f;
			float top = tl_sprite_cy - tl_sprite_size * 0.5f;
			u = (frag->x - left) / tl_sprite_size;
			v = (frag->y - top) / tl_sprite_size;
		} else {
			u = (float)frag->x / fb->width;
			v = (float)frag->y / fb->height;
		}
		if (tex->wrap_s == GL_REPEAT)
			u -= floorf(u);
		else
			u = GL_CLAMP(u, 0.0f, 1.0f);
		if (tex->wrap_t == GL_REPEAT)
			v -= floorf(v);
		else
			v = GL_CLAMP(v, 0.0f, 1.0f);
		unsigned level = 0;
		if (tex->min_filter >= GL_NEAREST_MIPMAP_NEAREST) {
			float ratio =
				GL_MAX((float)tex->mip_width[0] / fb->width,
				       (float)tex->mip_height[0] / fb->height);
			float lod = log2f(ratio);
			lod = GL_CLAMP(lod, 0.0f, tex->current_level);
			level = (unsigned)lod;
		}
		if (!tex->levels[level])
			level = 0;
		float x = u * (tex->mip_width[level] - 1);
		float y = v * (tex->mip_height[level] - 1);
		int ix = (int)roundf(x);
		int iy = (int)roundf(y);
		bool linear = tex->min_filter == GL_LINEAR ||
			      tex->mag_filter == GL_LINEAR ||
			      tex->min_filter == GL_LINEAR_MIPMAP_NEAREST ||
			      tex->min_filter == GL_LINEAR_MIPMAP_LINEAR;
		if (linear) {
			int ix1 = ix < tex->mip_width[level] - 1 ? ix + 1 : ix;
			int iy1 = iy < tex->mip_height[level] - 1 ? iy + 1 : iy;
			float fx = x - floorf(x);
			float fy = y - floorf(y);
			uint32_t c00 =
				texture_cache_fetch(cache, tex, level, ix, iy);
			uint32_t c10 =
				texture_cache_fetch(cache, tex, level, ix1, iy);
			uint32_t c01 =
				texture_cache_fetch(cache, tex, level, ix, iy1);
			uint32_t c11 = texture_cache_fetch(cache, tex, level,
							   ix1, iy1);
			float r = (((c00 >> 16) & 0xFF) * (1 - fx) * (1 - fy) +
				   ((c10 >> 16) & 0xFF) * fx * (1 - fy) +
				   ((c01 >> 16) & 0xFF) * (1 - fx) * fy +
				   ((c11 >> 16) & 0xFF) * fx * fy);
			float g = (((c00 >> 8) & 0xFF) * (1 - fx) * (1 - fy) +
				   ((c10 >> 8) & 0xFF) * fx * (1 - fy) +
				   ((c01 >> 8) & 0xFF) * (1 - fx) * fy +
				   ((c11 >> 8) & 0xFF) * fx * fy);
			float b = ((c00 & 0xFF) * (1 - fx) * (1 - fy) +
				   (c10 & 0xFF) * fx * (1 - fy) +
				   (c01 & 0xFF) * (1 - fx) * fy +
				   (c11 & 0xFF) * fx * fy);
			uint32_t color = ((uint32_t)r << 16) |
					 ((uint32_t)g << 8) | (uint32_t)b |
					 0xFF000000u;
			if (local_tex[0].env_mode == GL_MODULATE)
				frag->color = modulate(frag->color, color);
			else
				frag->color = color;
		} else {
			uint32_t color =
				texture_cache_fetch(cache, tex, level, ix, iy);
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
		factor = GL_CLAMP(factor, 0.0f, 1.0f);
		uint8_t r = (frag->color >> 16) & 0xFF;
		uint8_t g = (frag->color >> 8) & 0xFF;
		uint8_t b = frag->color & 0xFF;
		r = (uint8_t)(GL_LERP(local_fog.color[0] * 255.0f, r, factor));
		g = (uint8_t)(GL_LERP(local_fog.color[1] * 255.0f, g, factor));
		b = (uint8_t)(GL_LERP(local_fog.color[2] * 255.0f, b, factor));
		frag->color = (frag->color & 0xFF000000u) |
			      ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
	}
	RenderContext *ctx = GetCurrentContext();
	if (ctx->alpha_test.enabled) {
		float alpha = ((frag->color >> 24) & 0xFF) / 255.0f;
		float ref = local_alpha.ref;
		bool pass = false;
		switch (local_alpha.func) {
		case GL_NEVER:
			pass = false;
			break;
		case GL_LESS:
			pass = alpha < ref;
			break;
		case GL_LEQUAL:
			pass = alpha <= ref;
			break;
		case GL_GREATER:
			pass = alpha > ref;
			break;
		case GL_GEQUAL:
			pass = alpha >= ref;
			break;
		case GL_EQUAL:
			pass = alpha == ref;
			break;
		case GL_NOTEQUAL:
			pass = alpha != ref;
			break;
		case GL_ALWAYS:
		default:
			pass = true;
			break;
		}
		if (!pass)
			return;
	}
	if (local_blend_on) {
		uint32_t dst = framebuffer_get_pixel(fb, frag->x, frag->y);
		float srcA = ((frag->color >> 24) & 0xFF) / 255.0f;
		float dstA = ((dst >> 24) & 0xFF) / 255.0f;
		float src[3] = { ((frag->color >> 16) & 0xFF) / 255.0f,
				 ((frag->color >> 8) & 0xFF) / 255.0f,
				 (frag->color & 0xFF) / 255.0f };
		float dstc[3] = { ((dst >> 16) & 0xFF) / 255.0f,
				  ((dst >> 8) & 0xFF) / 255.0f,
				  (dst & 0xFF) / 255.0f };
		float out[3];
		for (int i = 0; i < 3; ++i) {
			float sf = blend_factor(local_blend.src_factor, src[i],
						dstc[i], srcA, dstA);
			float df = blend_factor(local_blend.dst_factor, src[i],
						dstc[i], srcA, dstA);
			out[i] = GL_CLAMP(src[i] * sf + dstc[i] * df, 0.0f,
					  1.0f);
		}
		float af = blend_factor(local_blend.src_factor, srcA, dstA,
					srcA, dstA);
		float bf = blend_factor(local_blend.dst_factor, srcA, dstA,
					srcA, dstA);
		float outA = srcA * af + dstA * bf;
		frag->color = ((uint32_t)(outA * 255.0f) << 24) |
			      ((uint32_t)(out[0] * 255.0f) << 16) |
			      ((uint32_t)(out[1] * 255.0f) << 8) |
			      (uint32_t)(out[2] * 255.0f);
	}
	framebuffer_set_pixel(fb, frag->x, frag->y, frag->color, frag->depth);
}

void process_fragment_job(void *task_data)
{
	FragmentJob *job = (FragmentJob *)task_data;
	plugin_invoke(STAGE_FRAGMENT, job);
	pipeline_shade_fragment(&job->frag, job->fb);
	MT_FREE(job, STAGE_FRAGMENT);
}

void process_fragment_tile_job(void *task_data)
{
	FragmentTileJob *job = (FragmentTileJob *)task_data;
	plugin_invoke(STAGE_FRAGMENT, job);
	LOG_DEBUG("Fragment tile (%u,%u)-(%u,%u) mode=%s", job->x0, job->y0,
		  job->x1, job->y1, job->sprite_mode ? "sprite" : "triangle");
	uint32_t w = job->x1 - job->x0 + 1;
	uint32_t h = job->y1 - job->y0 + 1;

	Framebuffer *fb = job->fb;
	uint32_t tile_x = job->x0 / fb->tile_size;
	uint32_t tile_y = job->y0 / fb->tile_size;
	FramebufferTile *tile = &fb->tiles[tile_y * fb->tiles_x + tile_x];
	while (atomic_flag_test_and_set(&tile->lock))
		thrd_yield();
	tile->x0 = job->x0;
	tile->y0 = job->y0;

	for (uint32_t row = 0; row < h; ++row) {
		size_t idx = (size_t)(job->y0 + row) * fb->width + job->x0;
		memcpy(&tile->color[row * fb->tile_size],
		       &fb->color_buffer[idx], w * sizeof(uint32_t));
		memcpy(&tile->depth[row * fb->tile_size],
		       &fb->depth_buffer[idx], w * sizeof(float));
		memcpy(&tile->stencil[row * fb->tile_size],
		       &fb->stencil_buffer[idx], w * sizeof(uint8_t));
	}

	framebuffer_enter_tile(tile);

	GLboolean prev_mode = tl_sprite_mode;
	GLfloat prev_cx = tl_sprite_cx;
	GLfloat prev_cy = tl_sprite_cy;
	GLfloat prev_size = tl_sprite_size;
	if (job->sprite_mode) {
		tl_sprite_mode = GL_TRUE;
		tl_sprite_cx = job->sprite_cx;
		tl_sprite_cy = job->sprite_cy;
		tl_sprite_size = job->sprite_size;
	}

	for (uint32_t row = 0; row < h; ++row) {
		for (uint32_t col = 0; col < w; ++col) {
			Fragment frag = {
				.x = job->x0 + col,
				.y = job->y0 + row,
				.color = job->color,
				.depth = job->depth,
			};
			pipeline_shade_fragment(&frag, fb);
		}
	}

	if (job->sprite_mode) {
		tl_sprite_mode = prev_mode;
		tl_sprite_cx = prev_cx;
		tl_sprite_cy = prev_cy;
		tl_sprite_size = prev_size;
	}

	framebuffer_leave_tile();
	for (uint32_t row = 0; row < h; ++row) {
		size_t idx = (size_t)(job->y0 + row) * fb->width + job->x0;
		memcpy(&fb->color_buffer[idx],
		       &tile->color[row * fb->tile_size], w * sizeof(uint32_t));
		memcpy(&fb->depth_buffer[idx],
		       &tile->depth[row * fb->tile_size], w * sizeof(float));
		memcpy(&fb->stencil_buffer[idx],
		       &tile->stencil[row * fb->tile_size],
		       w * sizeof(uint8_t));
	}
	atomic_flag_clear(&tile->lock);
	framebuffer_release(job->fb);
	tile_job_release(job);
}
