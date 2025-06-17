#ifndef GL_MEMORY_TRACKER_H
#define GL_MEMORY_TRACKER_H
/**
 * @file gl_memory_tracker.h
 * @brief Simple memory allocation tracker for debugging.
 */

#include <stddef.h>
#include "portable/c11threads.h"
#include "gl_thread.h" /* for stage_tag_t */

#ifdef __cplusplus
extern "C" {
#endif

int memory_tracker_init(void);
void memory_tracker_shutdown(void);
void *memory_tracker_allocate(size_t size, stage_tag_t stage, const char *type,
			      int line);
void *memory_tracker_calloc(size_t n, size_t size, stage_tag_t stage,
			    const char *type, int line);
void *memory_tracker_realloc(void *ptr, size_t size, stage_tag_t stage,
			     const char *type, int line);
void memory_tracker_free(void *ptr, stage_tag_t stage, const char *type,
			 int line);
size_t memory_tracker_current(void);
size_t memory_tracker_peak(void);
void memory_tracker_report(void);

#define MT_ALLOC(sz, stage) \
	memory_tracker_allocate((sz), (stage), __func__, __LINE__)
#define MT_CALLOC(n, sz, stage) \
	memory_tracker_calloc((n), (sz), (stage), __func__, __LINE__)
#define MT_REALLOC(p, sz, stage) \
	memory_tracker_realloc((p), (sz), (stage), __func__, __LINE__)
#define MT_FREE(p, stage) memory_tracker_free((p), (stage), __func__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif /* GL_MEMORY_TRACKER_H */
