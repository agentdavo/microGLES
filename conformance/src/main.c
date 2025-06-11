#include "gl_state.h"
#include "gl_init.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "tests.h"
#include <stdio.h>

int main()
{
	if (!logger_init("conformance.log", LOG_LEVEL_INFO)) {
		fprintf(stderr, "Failed to init logger.\n");
		return -1;
	}
	if (!memory_tracker_init()) {
		LOG_FATAL("Failed to init Memory Tracker.");
		return -1;
	}
	InitGLState(&gl_state);
	Framebuffer *fb = GL_init_with_framebuffer(64, 64);
	if (!fb) {
		LOG_FATAL("Failed to create framebuffer");
		return -1;
	}

	int pass = 1;
	if (!test_framebuffer_complete()) {
		LOG_ERROR("Framebuffer completeness test failed");
		pass = 0;
	}
	if (!test_texture_creation()) {
		LOG_ERROR("Texture creation test failed");
		pass = 0;
	}
	if (!test_framebuffer_colors()) {
		LOG_ERROR("Framebuffer color write test failed");
		pass = 0;
	}
	if (!test_framebuffer_module()) {
		LOG_ERROR("Software framebuffer test failed");
		pass = 0;
	}
	if (!test_enable_disable()) {
		LOG_ERROR("Enable/Disable test failed");
		pass = 0;
	}
	if (!test_viewport()) {
		LOG_ERROR("Viewport test failed");
		pass = 0;
	}
	if (!test_matrix_stack()) {
		LOG_ERROR("Matrix stack test failed");
		pass = 0;
	}
	if (!test_clear_state()) {
		LOG_ERROR("Clear state test failed");
		pass = 0;
	}
	if (!test_buffer_objects()) {
		LOG_ERROR("Buffer object test failed");
		pass = 0;
	}
	if (!test_texture_setup()) {
		LOG_ERROR("Texture setup test failed");
		pass = 0;
	}
	if (!test_blend_func()) {
		LOG_ERROR("Blend func test failed");
		pass = 0;
	}
	if (!test_scissor_state()) {
		LOG_ERROR("Scissor state test failed");
		pass = 0;
	}

	GL_cleanup_with_framebuffer(fb);
	CleanupGLState(&gl_state);
	memory_tracker_shutdown();
	memory_tracker_report();
	logger_shutdown();

	if (pass) {
		printf("All tests passed\n");
		return 0;
	} else {
		printf("Tests failed\n");
		return 1;
	}
}
