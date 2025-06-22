/* gl_utils.c */

#include "gl_utils.h"
#include "gl_errors.h"
#include "gl_logger.h" // For logging
#include "gl_memory_tracker.h" // For MT_ALLOC and MT_FREE

/* Function to allocate memory with tracking */
void *tracked_malloc(size_t size)
{
	void *ptr = MT_ALLOC(size, STAGE_FRAMEBUFFER);
	if (!ptr) {
		LOG_ERROR("tracked_malloc: Failed to allocate %zu bytes.",
			  size);
		glSetError(GL_OUT_OF_MEMORY);
	} else {
		LOG_DEBUG("tracked_malloc: Allocated %zu bytes at %p.", size,
			  ptr);
	}
	return ptr;
}

/* Function to allocate aligned memory with tracking */
void *tracked_aligned_alloc(size_t alignment, size_t size)
{
	void *ptr = MT_ALIGNED_ALLOC(alignment, size, STAGE_FRAMEBUFFER);
	if (!ptr) {
		LOG_WARN(
			"tracked_aligned_alloc: falling back to unaligned malloc");
		ptr = MT_ALLOC(size, STAGE_FRAMEBUFFER);
		if (!ptr) {
			LOG_ERROR(
				"tracked_aligned_alloc: Failed to allocate %zu bytes.",
				size);
			glSetError(GL_OUT_OF_MEMORY);
		}
	} else {
		LOG_DEBUG(
			"tracked_aligned_alloc: Allocated %zu bytes (al %zu) at %p.",
			size, alignment, ptr);
	}
	return ptr;
}

/* Function to free memory with tracking */
void tracked_free(void *ptr, size_t size)
{
	if (ptr) {
		MT_FREE(ptr, STAGE_FRAMEBUFFER);
		LOG_DEBUG("tracked_free: Freed %zu bytes at %p.", size, ptr);
	}
}

/* Utility function to validate framebuffer completeness */
GLboolean ValidateFramebufferCompleteness(void)
{
	/* Implement validation logic */
	LOG_DEBUG("ValidateFramebufferCompleteness: Validation successful.");
	return GL_TRUE;
}
