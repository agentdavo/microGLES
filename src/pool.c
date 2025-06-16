#include "pool.h"
#include "gl_memory_tracker.h"
#include "pipeline/gl_vertex.h"
#include "pipeline/gl_raster.h"
#include <stdlib.h>

struct PoolNode {
	struct PoolNode *next_free;
	struct PoolNode *next_all;
};

struct ObjectPool {
	struct PoolNode *free_list;
	struct PoolNode *all_nodes;
	mtx_t mutex;
	size_t obj_size;
	unsigned capacity;
	stage_tag_t stage;
};

/* global pools for common job types */
static ObjectPool g_vertex_job_pool;
static ObjectPool g_raster_job_pool;
static ObjectPool g_tile_job_pool;
#define JOB_POOL_CAPACITY 512

void pool_init(ObjectPool *pool, size_t obj_size, unsigned capacity,
	       stage_tag_t stage)
{
	*pool = (ObjectPool){ 0 };
	pool->obj_size = obj_size;
	pool->capacity = capacity;
	pool->stage = stage;
	mtx_init(&pool->mutex, mtx_plain);
	for (unsigned i = 0; i < capacity; ++i) {
		struct PoolNode *node =
			MT_ALLOC(sizeof(struct PoolNode) + obj_size, stage);
		if (!node)
			break;
		node->next_free = pool->free_list;
		pool->free_list = node;
		node->next_all = pool->all_nodes;
		pool->all_nodes = node;
	}
}

void *pool_acquire(ObjectPool *pool)
{
	mtx_lock(&pool->mutex);
	struct PoolNode *node = pool->free_list;
	if (node)
		pool->free_list = node->next_free;
	mtx_unlock(&pool->mutex);
	if (!node)
		return NULL;
	return (void *)(node + 1);
}

void pool_release(ObjectPool *pool, void *obj)
{
	if (!obj)
		return;
	struct PoolNode *node = ((struct PoolNode *)obj) - 1;
	mtx_lock(&pool->mutex);
	node->next_free = pool->free_list;
	pool->free_list = node;
	mtx_unlock(&pool->mutex);
}

void pool_destroy(ObjectPool *pool)
{
	struct PoolNode *node = pool->all_nodes;
	while (node) {
		struct PoolNode *next = node->next_all;
		MT_FREE(node, pool->stage);
		node = next;
	}
	mtx_destroy(&pool->mutex);
	*pool = (ObjectPool){ 0 };
}

void job_pools_init(void)
{
	pool_init(&g_vertex_job_pool, sizeof(VertexJob), JOB_POOL_CAPACITY,
		  STAGE_VERTEX);
	pool_init(&g_raster_job_pool, sizeof(RasterJob), JOB_POOL_CAPACITY,
		  STAGE_RASTER);
	pool_init(&g_tile_job_pool, sizeof(FragmentTileJob), JOB_POOL_CAPACITY,
		  STAGE_FRAGMENT);
}

void job_pools_destroy(void)
{
	pool_destroy(&g_tile_job_pool);
	pool_destroy(&g_raster_job_pool);
	pool_destroy(&g_vertex_job_pool);
}

VertexJob *vertex_job_acquire(void)
{
	return (VertexJob *)pool_acquire(&g_vertex_job_pool);
}

void vertex_job_release(VertexJob *job)
{
	pool_release(&g_vertex_job_pool, job);
}

RasterJob *raster_job_acquire(void)
{
	return (RasterJob *)pool_acquire(&g_raster_job_pool);
}

void raster_job_release(RasterJob *job)
{
	pool_release(&g_raster_job_pool, job);
}

FragmentTileJob *tile_job_acquire(void)
{
	return (FragmentTileJob *)pool_acquire(&g_tile_job_pool);
}

void tile_job_release(FragmentTileJob *job)
{
	pool_release(&g_tile_job_pool, job);
}
