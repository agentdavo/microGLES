#ifndef CONFORMANCE_TESTS_H
#define CONFORMANCE_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

int test_framebuffer_complete(void);
int test_texture_creation(void);
int test_framebuffer_colors(void);
int test_enable_disable(void);
int test_viewport(void);
int test_matrix_stack(void);
int test_clear_state(void);
int test_buffer_objects(void);
int test_texture_setup(void);
int test_blend_func(void);
int test_scissor_state(void);
int test_framebuffer_module(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFORMANCE_TESTS_H */
