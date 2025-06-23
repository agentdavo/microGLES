#include "gl_state.h"
#include "gl_init.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include "command_buffer.h"
#include <GLES/gl.h>

int main(void)
{
	if (!logger_init("stage_logging_demo.log", LOG_LEVEL_DEBUG)) {
		return -1;
	}
	if (!memory_tracker_init()) {
		LOG_FATAL("Failed to initialize Memory Tracker.");
		return -1;
	}
	if (!thread_pool_init_from_env()) {
		LOG_FATAL("Failed to init thread pool");
		return -1;
	}
	command_buffer_init();
	InitGLState(&gl_state);
	Framebuffer *fb = GL_init_with_framebuffer(256, 256);
	if (!fb) {
		LOG_FATAL("Failed to create framebuffer");
		return -1;
	}

	GLfloat verts[] = { -0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
			    0.0f,  0.0f,  0.5f, 0.0f };
	GLfloat colors[] = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			     0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f };
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	thread_pool_wait();
	framebuffer_write_bmp(fb, "stage_demo.bmp");

	command_buffer_shutdown();
	thread_pool_shutdown();
	GL_cleanup_with_framebuffer(fb);
	CleanupGLState(&gl_state);
	memory_tracker_shutdown();
	memory_tracker_report();
	logger_shutdown();
	return 0;
}
