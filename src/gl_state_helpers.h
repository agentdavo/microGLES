#ifndef GL_STATE_HELPERS_H
#define GL_STATE_HELPERS_H
/**
 * @file gl_state_helpers.h
 * @brief Atomic helpers for GL state version tracking.
 */

#include <GLES/gl.h>
#include <stdatomic.h>

#define SET_BOOL(dst, val, ver)                                             \
	do {                                                                \
		(dst) = (val);                                              \
		atomic_fetch_add_explicit(&(ver), 1, memory_order_relaxed); \
	} while (0)

#define SET_FLOAT4(dst, val, ver)                                           \
	do {                                                                \
		const GLfloat *_v = (val);                                  \
		(dst)[0] = _v[0];                                           \
		(dst)[1] = _v[1];                                           \
		(dst)[2] = _v[2];                                           \
		(dst)[3] = _v[3];                                           \
		atomic_fetch_add_explicit(&(ver), 1, memory_order_relaxed); \
	} while (0)

#define SET_BOOL_AND_BUMP(ctx, field, value, version)                    \
	do {                                                             \
		if ((ctx)->field != (value)) {                           \
			(ctx)->field = (value);                          \
			atomic_fetch_add_explicit(&(version), 1,         \
						  memory_order_relaxed); \
		}                                                        \
	} while (0)

#endif /* GL_STATE_HELPERS_H */
