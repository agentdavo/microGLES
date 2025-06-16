#include "gl_primitive.h"
#include "gl_raster.h"
#include "../gl_thread.h"
#include "../pool.h"
#define PIPELINE_USE_GLSTATE 0
_Static_assert(PIPELINE_USE_GLSTATE == 0, "pipeline must not touch gl_state");
#include "../gl_memory_tracker.h"
#include <string.h>

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
	RasterJob *rjob = raster_job_acquire();
	if (!rjob) {
		MT_FREE(job, STAGE_PRIMITIVE);
		return;
	}
	rjob->tri = tri;
	rjob->fb = job->fb;
	memcpy(rjob->viewport, job->viewport, sizeof(job->viewport));
	MT_FREE(job, STAGE_PRIMITIVE);
	thread_pool_submit(process_raster_job, rjob, STAGE_RASTER);
}
