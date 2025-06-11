#ifndef GL_FRAGMENT_H
#define GL_FRAGMENT_H

#include "../gl_types.h"
#include "gl_framebuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

void pipeline_shade_fragment(Fragment *frag, Framebuffer *fb);

typedef struct {
	Fragment frag;
	Framebuffer *fb;
} FragmentJob;

void process_fragment_job(void *task_data);

#ifdef __cplusplus
}
#endif

#endif /* GL_FRAGMENT_H */
