#include "gl_raster.h"
#include "gl_fragment.h"
#include "../gl_logger.h"
#include "../gl_context.h"
#define PIPELINE_USE_GLSTATE 0
_Static_assert(PIPELINE_USE_GLSTATE == 0, "pipeline must not touch gl_state");
#include "../gl_memory_tracker.h"
#include "../gl_thread.h"
#include "../pool.h"

static uint32_t pack_color(const GLfloat c[4])
{
	uint8_t r = (uint8_t)(c[0] * 255.0f + 0.5f);
	uint8_t g = (uint8_t)(c[1] * 255.0f + 0.5f);
	uint8_t b = (uint8_t)(c[2] * 255.0f + 0.5f);
	uint8_t a = (uint8_t)(c[3] * 255.0f + 0.5f);
	return (a << 24) | (b << 16) | (g << 8) | r;
}

void pipeline_rasterize_triangle(const Triangle *restrict tri,
				 const GLint *restrict viewport,
				 Framebuffer *restrict fb)
{
	float minx = tri->v0.x;
	float maxx = tri->v0.x;
	float miny = tri->v0.y;
	float maxy = tri->v0.y;
	for (int i = 1; i < 3; ++i) {
		const Vertex *v = i == 1 ? &tri->v1 : &tri->v2;
		if (v->x < minx)
			minx = v->x;
		if (v->x > maxx)
			maxx = v->x;
		if (v->y < miny)
			miny = v->y;
		if (v->y > maxy)
			maxy = v->y;
	}
	int vp_x0 = viewport[0];
	int vp_y0 = viewport[1];
	int vp_x1 = viewport[0] + viewport[2] - 1;
	int vp_y1 = viewport[1] + viewport[3] - 1;
	int iminx = (int)minx;
	if (iminx < vp_x0)
		iminx = vp_x0;
	int iminy = (int)miny;
	if (iminy < vp_y0)
		iminy = vp_y0;
	int imaxx = (int)maxx;
	if (imaxx > vp_x1)
		imaxx = vp_x1;
	int imaxy = (int)maxy;
	if (imaxy > vp_y1)
		imaxy = vp_y1;
	RenderContext *ctx = GetCurrentContext();
	if (ctx->scissor_test_enabled) {
		int sx = ctx->scissor_box[0];
		int sy = ctx->scissor_box[1];
		int sx1 = sx + ctx->scissor_box[2] - 1;
		int sy1 = sy + ctx->scissor_box[3] - 1;
		if (sx < 0)
			sx = 0;
		if (sy < 0)
			sy = 0;
		if (sx1 >= (int)fb->width)
			sx1 = fb->width - 1;
		if (sy1 >= (int)fb->height)
			sy1 = fb->height - 1;
		if (iminx < sx)
			iminx = sx;
		if (iminy < sy)
			iminy = sy;
		if (imaxx > sx1)
			imaxx = sx1;
		if (imaxy > sy1)
			imaxy = sy1;
	}
	if (iminx > imaxx || iminy > imaxy)
		return;
	uint32_t color = pack_color(tri->v0.color);
	for (int ty = iminy; ty <= imaxy; ty += TILE_SIZE) {
		for (int tx = iminx; tx <= imaxx; tx += TILE_SIZE) {
			int ex = tx + TILE_SIZE - 1;
			if (ex > imaxx)
				ex = imaxx;
			int ey = ty + TILE_SIZE - 1;
			if (ey > imaxy)
				ey = imaxy;
			FragmentTileJob *jobt = tile_job_acquire();
			if (!jobt)
				continue;
			jobt->x0 = tx;
			jobt->y0 = ty;
			jobt->x1 = ex;
			jobt->y1 = ey;
			jobt->color = color;
			jobt->depth = tri->v0.z;
			jobt->fb = fb;
			jobt->sprite_mode = GL_FALSE;
			thread_pool_submit(process_fragment_tile_job, jobt,
					   STAGE_FRAGMENT);
		}
	}
}

void pipeline_rasterize_point(const Vertex *restrict v, GLfloat size,
			      const GLint *restrict viewport,
			      Framebuffer *restrict fb)
{
	int half = (int)(size * 0.5f);
	int x0 = (int)(v->x - half);
	int y0 = (int)(v->y - half);
	int x1 = (int)(v->x + half);
	int y1 = (int)(v->y + half);
	int vp_x0 = viewport[0];
	int vp_y0 = viewport[1];
	int vp_x1 = viewport[0] + viewport[2] - 1;
	int vp_y1 = viewport[1] + viewport[3] - 1;
	if (x0 < vp_x0)
		x0 = vp_x0;
	if (y0 < vp_y0)
		y0 = vp_y0;
	if (x1 > vp_x1)
		x1 = vp_x1;
	if (y1 > vp_y1)
		y1 = vp_y1;
	RenderContext *ctx = GetCurrentContext();
	if (ctx->scissor_test_enabled) {
		int sx = ctx->scissor_box[0];
		int sy = ctx->scissor_box[1];
		int sx1 = sx + ctx->scissor_box[2] - 1;
		int sy1 = sy + ctx->scissor_box[3] - 1;
		if (sx < 0)
			sx = 0;
		if (sy < 0)
			sy = 0;
		if (sx1 >= (int)fb->width)
			sx1 = fb->width - 1;
		if (sy1 >= (int)fb->height)
			sy1 = fb->height - 1;
		if (x0 < sx)
			x0 = sx;
		if (y0 < sy)
			y0 = sy;
		if (x1 > sx1)
			x1 = sx1;
		if (y1 > sy1)
			y1 = sy1;
	}
	if (x0 > x1 || y0 > y1)
		return;
	uint32_t color = pack_color(v->color);
	for (int ty = y0; ty <= y1; ty += TILE_SIZE) {
		for (int tx = x0; tx <= x1; tx += TILE_SIZE) {
			int ex = tx + TILE_SIZE - 1;
			if (ex > x1)
				ex = x1;
			int ey = ty + TILE_SIZE - 1;
			if (ey > y1)
				ey = y1;
			FragmentTileJob *jobt = tile_job_acquire();
			if (!jobt)
				continue;
			jobt->x0 = tx;
			jobt->y0 = ty;
			jobt->x1 = ex;
			jobt->y1 = ey;
			jobt->color = color;
			jobt->depth = v->z;
			jobt->fb = fb;
			jobt->sprite_mode = GL_TRUE;
			jobt->sprite_cx = v->x;
			jobt->sprite_cy = v->y;
			jobt->sprite_size = size;
			thread_pool_submit(process_fragment_tile_job, jobt,
					   STAGE_FRAGMENT);
		}
	}
}

void process_raster_job(void *task_data)
{
	RasterJob *job = (RasterJob *)task_data;
	pipeline_rasterize_triangle(&job->tri, job->viewport, job->fb);
	raster_job_release(job);
}
