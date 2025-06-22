/* fixed_point.h */

#ifndef FIXED_POINT_H
#define FIXED_POINT_H
/**
 * @file fixed_point.h
 * @brief Fixed point conversion helpers.
 */

#include <GLES/gl.h>
#include <stdint.h>

#define FIXED_TO_FLOAT(x) ((GLfloat)(x) / 65536.0f)
#define FLOAT_TO_FIXED(x) ((GLfixed)((x) * 65536.0f))

#ifdef __cplusplus
extern "C" {
#endif

static inline GLfloat fixed_to_float(GLfixed v)
{
	return (GLfloat)v / 65536.0f;
}

static inline GLfixed float_to_fixed(GLfloat f)
{
	return (GLfixed)(f * 65536.0f);
}

static inline GLfixed fixed_mul(GLfixed a, GLfixed b)
{
	return (GLfixed)(((int64_t)a * (int64_t)b) >> 16);
}

static inline GLfixed fixed_div(GLfixed a, GLfixed b)
{
	return (GLfixed)(((int64_t)a << 16) / b);
}

#ifdef __cplusplus
}
#endif

#endif /* FIXED_POINT_H */
