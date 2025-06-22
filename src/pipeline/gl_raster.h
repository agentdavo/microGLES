#ifndef GL_RASTER_H
#define GL_RASTER_H

#include "../gl_types.h"
#include "gl_framebuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

void pipeline_rasterize_triangle(const Triangle *restrict tri,
				 const GLint *restrict viewport,
				 Framebuffer *restrict fb);
void pipeline_rasterize_point(const Vertex *restrict v, GLfloat size,
			      const GLint *restrict viewport,
			      Framebuffer *restrict fb);

#define TILE_SIZE 16

typedef struct {
	uint32_t x0, y0, x1, y1;
	uint32_t color;
	float depth;
	Framebuffer *fb;
	GLboolean sprite_mode;
	GLfloat sprite_cx;
	GLfloat sprite_cy;
	GLfloat sprite_size;
} FragmentTileJob;

typedef struct {
	Triangle tri;
	Framebuffer *fb;
	GLint viewport[4];
} RasterJob;

void process_raster_job(void *task_data);
void process_fragment_tile_job(void *task_data);

#ifdef __cplusplus
}
#endif

#endif /* GL_RASTER_H */
