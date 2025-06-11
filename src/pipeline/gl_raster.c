#include "gl_raster.h"
#include "gl_fragment.h"
#include "../gl_logger.h"
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

void process_fragment_tile_job(void *task_data)
{
	FragmentTileJob *job = (FragmentTileJob *)task_data;
	for (uint32_t y = job->y0; y <= job->y1; ++y)
		for (uint32_t x = job->x0; x <= job->x1; ++x) {
			Fragment frag = { x, y, job->color, job->depth };
			pipeline_shade_fragment(&frag, job->fb);
		}
	MT_FREE(job, STAGE_FRAGMENT);
}
