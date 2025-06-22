/* gl_utils.h */

#ifndef GL_UTILS_H
#define GL_UTILS_H
/**
 * @file gl_utils.h
 * @brief Utility routines.
 */

#include <GLES/gl.h> /* OpenGL ES 1.1 */
#include <GLES/glext.h> /* For extension types */
#include <stddef.h> /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Memory tracking functions */
void *tracked_malloc(size_t size);
void tracked_free(void *ptr, size_t size);

#define FIXED_TO_FLOAT(x) ((GLfloat)(x) / 65536.0f)
#define FLOAT_TO_FIXED(x) ((GLfixed)((x) * 65536.0f))

/* Utility function to validate framebuffer completeness */
GLboolean ValidateFramebufferCompleteness(void);

#ifdef __cplusplus
}
#endif

#endif /* GL_UTILS_H */
