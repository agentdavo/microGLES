#ifndef POOL_H
#define POOL_H
#include <stddef.h>
#include "portable/c11threads.h"
#include "gl_thread.h" /* stage_tag_t */
#include "pipeline/gl_vertex.h"
#include "pipeline/gl_raster.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ObjectPool ObjectPool;

void pool_init(ObjectPool *pool, size_t obj_size, unsigned capacity,
	       stage_tag_t stage);
void *pool_acquire(ObjectPool *pool);
void pool_release(ObjectPool *pool, void *obj);
void pool_destroy(ObjectPool *pool);

/* Job specific helpers */
void job_pools_init(void);
void job_pools_destroy(void);
VertexJob *vertex_job_acquire(void);
void vertex_job_release(VertexJob *job);
RasterJob *raster_job_acquire(void);
void raster_job_release(RasterJob *job);
FragmentTileJob *tile_job_acquire(void);
void tile_job_release(FragmentTileJob *job);

#ifdef __cplusplus
}
#endif

#endif /* POOL_H */
