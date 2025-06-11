#include "gl_primitive.h"
#include "gl_raster.h"
#include "../gl_thread.h"
#include "../gl_memory_tracker.h"

void pipeline_assemble_triangle(Triangle *dst, const Vertex *v0,
				const Vertex *v1, const Vertex *v2)
{
	*dst = (Triangle){ *v0, *v1, *v2 };
}

static float edge(const Vertex *a, const Vertex *b, const Vertex *c)
{
	return (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
}

void process_primitive_job(void *task_data)
{
	PrimitiveJob *job = (PrimitiveJob *)task_data;
	Triangle tri;
	pipeline_assemble_triangle(&tri, &job->verts[0], &job->verts[1],
				   &job->verts[2]);
	if (edge(&tri.v0, &tri.v1, &tri.v2) <= 0.f) {
		MT_FREE(job, STAGE_PRIMITIVE);
		return; // culled
	}
	RasterJob *rjob = MT_ALLOC(sizeof(RasterJob), STAGE_RASTER);
	if (!rjob) {
		MT_FREE(job, STAGE_PRIMITIVE);
		return;
	}
	rjob->tri = tri;
	rjob->fb = job->fb;
	MT_FREE(job, STAGE_PRIMITIVE);
	thread_pool_submit(process_raster_job, rjob, STAGE_RASTER);
}
