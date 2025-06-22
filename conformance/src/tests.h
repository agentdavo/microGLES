#ifndef CONFORMANCE_TESTS_H
#define CONFORMANCE_TESTS_H

#include <stddef.h>
#include <GLES/gl.h>
#include "gl_logger.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Test {
	const char *name;
	int (*fn)(void);
};

#define CHECK_OK(expr)                                 \
	do {                                           \
		if (!(expr)) {                         \
			LOG_ERROR("%s failed", #expr); \
			return 0;                      \
		}                                      \
	} while (0)

#define CHECK_GLError(code)                                                    \
	do {                                                                   \
		GLenum err = glGetError();                                     \
		if (err != (code)) {                                           \
			LOG_ERROR("GL error 0x%X expected 0x%X", err, (code)); \
			return 0;                                              \
		}                                                              \
	} while (0)

const struct Test *get_state_tests(size_t *count);
const struct Test *get_matrix_tests(size_t *count);
const struct Test *get_texture_tests(size_t *count);
const struct Test *get_texture_cache_tests(size_t *count);
const struct Test *get_buffer_tests(size_t *count);
const struct Test *get_draw_tests(size_t *count);
const struct Test *get_fbo_tests(size_t *count);
const struct Test *get_depth_tests(size_t *count);
const struct Test *get_extensions_tests(size_t *count);
const struct Test *get_thread_stress_tests(size_t *count);
const struct Test *get_autogen_tests(size_t *count);
const struct Test *get_all_calls_tests(size_t *count);
const struct Test *get_point_sprite_tests(size_t *count);
const struct Test *get_fixed_point_tests(size_t *count);

#ifdef __cplusplus
}
#endif

#endif /* CONFORMANCE_TESTS_H */
