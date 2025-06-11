#ifndef GL_RASTER_H
#define GL_RASTER_H

#include "../gl_types.h"
#include "gl_framebuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

void pipeline_rasterize_triangle(const Triangle *tri, Framebuffer *fb);

#ifdef __cplusplus
}
#endif

#endif /* GL_RASTER_H */
