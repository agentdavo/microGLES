#include "gl_raster.h"
#include "gl_fragment.h"
#include "../gl_logger.h"
#define PIPELINE_USE_GLSTATE 0
_Static_assert(PIPELINE_USE_GLSTATE == 0, "pipeline must not touch gl_state");
#include "../gl_memory_tracker.h"
#include "../gl_thread.h"

static uint32_t pack_color(const GLfloat c[4])
{
	uint8_t r = (uint8_t)(c[0] * 255.0f + 0.5f);
	uint8_t g = (uint8_t)(c[1] * 255.0f + 0.5f);
	uint8_t b = (uint8_t)(c[2] * 255.0f + 0.5f);
	uint8_t a = (uint8_t)(c[3] * 255.0f + 0.5f);
	return (a << 24) | (b << 16) | (g << 8) | r;
}

void pipeline_rasterize_triangle(const Triangle *tri, Framebuffer *fb)
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
	int iminx = (int)minx;
	if (iminx < 0)
		iminx = 0;
	int iminy = (int)miny;
	if (iminy < 0)
		iminy = 0;
	int imaxx = (int)maxx;
	if (imaxx >= (int)fb->width)
		imaxx = fb->width - 1;
	int imaxy = (int)maxy;
	if (imaxy >= (int)fb->height)
		imaxy = fb->height - 1;
	uint32_t color = pack_color(tri->v0.color);
	for (int ty = iminy; ty <= imaxy; ty += TILE_SIZE) {
		for (int tx = iminx; tx <= imaxx; tx += TILE_SIZE) {
			int ex = tx + TILE_SIZE - 1;
			if (ex > imaxx)
				ex = imaxx;
			int ey = ty + TILE_SIZE - 1;
			if (ey > imaxy)
				ey = imaxy;
			FragmentTileJob *jobt = MT_ALLOC(
				sizeof(FragmentTileJob), STAGE_FRAGMENT);
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

void pipeline_rasterize_point(const Vertex *v, GLfloat size, Framebuffer *fb)
{
	int half = (int)(size * 0.5f);
	int x0 = (int)(v->x - half);
	int y0 = (int)(v->y - half);
	int x1 = (int)(v->x + half);
	int y1 = (int)(v->y + half);
	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 >= (int)fb->width)
		x1 = fb->width - 1;
	if (y1 >= (int)fb->height)
		y1 = fb->height - 1;
	uint32_t color = pack_color(v->color);
	for (int ty = y0; ty <= y1; ty += TILE_SIZE) {
		for (int tx = x0; tx <= x1; tx += TILE_SIZE) {
			int ex = tx + TILE_SIZE - 1;
			if (ex > x1)
				ex = x1;
			int ey = ty + TILE_SIZE - 1;
			if (ey > y1)
				ey = y1;
			FragmentTileJob *jobt = MT_ALLOC(
				sizeof(FragmentTileJob), STAGE_FRAGMENT);
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
	pipeline_rasterize_triangle(&job->tri, job->fb);
	MT_FREE(job, STAGE_RASTER);
}
