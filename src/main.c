/* main.c */

#include <GLES/gl.h>
#include <GLES/glext.h>
#include "gl_api_fbo.h"
#include "gl_state.h"
#ifdef HAVE_X11
#include "x11_window.h"
#endif
#include "gl_init.h"
#include "gl_types.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern const GLubyte *renderer_get_extensions(void);

int main(int argc, char **argv)
{
	bool use_x11 = false;
	unsigned win_w = 256;
	unsigned win_h = 256;
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--x11-window") == 0)
			use_x11 = true;
		else if (strncmp(argv[i], "--width=", 8) == 0)
			win_w = (unsigned)atoi(argv[i] + 8);
		else if (strncmp(argv[i], "--height=", 9) == 0)
			win_h = (unsigned)atoi(argv[i] + 9);
	}
	/* Initialize Logger */
	if (!logger_init("renderer.log", LOG_LEVEL_DEBUG)) {
		fprintf(stderr, "Failed to initialize logger.\n");
		return -1;
	}

	/* Initialize Memory Tracker */
	if (!memory_tracker_init()) {
		LOG_FATAL("Failed to initialize Memory Tracker.");
		return -1;
	}

	/* Initialize GLState */
	InitGLState(&gl_state);
	Framebuffer *display = GL_init_with_framebuffer(win_w, win_h);
	if (!display) {
		LOG_FATAL("Failed to create framebuffer");
		return -1;
	}

	/* Log supported extensions */
	const GLubyte *ext = renderer_get_extensions();
	LOG_INFO("Supported extensions: %s", ext);

	/* Your renderer initialization and operations */

	/* Example: Generate a renderbuffer */
	GLuint rb;
	glGenRenderbuffersOES(1, &rb);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, rb);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_RGBA4_OES, 256, 256);

	/* Example: Generate a framebuffer */
	GLuint fb;
	glGenFramebuffersOES(1, &fb);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES,
				     GL_COLOR_ATTACHMENT0_OES,
				     GL_RENDERBUFFER_OES, rb);
	GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
	if (status == GL_FRAMEBUFFER_COMPLETE_OES) {
		LOG_INFO("Framebuffer %u is complete.", fb);
	} else {
		LOG_ERROR("Framebuffer %u is incomplete. Status: 0x%X", fb,
			  status);
	}

	/* Example: Create a texture */
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	glDeleteTextures(1, &tex);

	/* Print memory stats */
	memory_tracker_report();

#ifdef HAVE_X11
	X11Window *win = NULL;
	if (use_x11)
		win = x11_window_create(win_w, win_h, "microGLES");
	if (win) {
		x11_window_show_image(win, display);
		sleep(2);
		x11_window_destroy(win);
	}
#else
	(void)use_x11;
#endif

	/* Your renderer operations */

	/* Cleanup */
	GL_cleanup_with_framebuffer(display);
	CleanupGLState(&gl_state);
	memory_tracker_shutdown(); // Should report any leaks
	memory_tracker_report(); // Should show zero allocations if all are freed

	/* Cleanup Logger */
	logger_shutdown();

	return 0;
}
