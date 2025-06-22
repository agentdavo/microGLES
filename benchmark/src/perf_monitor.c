#include "benchmark.h"
#include "gl_state.h"
#include "gl_init.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include "command_buffer.h"
#include "matrix_utils.h"
#include "gl_utils.h"
#ifdef HAVE_X11
#include "x11_window.h"
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

/* Cube geometry reused from stress_test.c */
static const GLfloat cube_vertices[] = {
	-0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,	 0.5f,	-0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f,
	-0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,	 -0.5f, 0.5f,  -0.5f,
	0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,	 0.5f,	-0.5f, -0.5f,
	0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f,
	0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,	-0.5f, 0.5f,
	0.5f,  -0.5f, 0.5f,  -0.5f, 0.5f,  -0.5f, 0.5f,	 0.5f,	0.5f,  0.5f,
	-0.5f, 0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,	 -0.5f, -0.5f
};
static const GLfloat cube_normals[] = {
	0, 0,  -1, 0, 0,  -1, 0,  0, -1, 0,  0, -1, 0,	0,  1, 0,  0,  1,
	0, 0,  1,  0, 0,  1,  -1, 0, 0,	 -1, 0, 0,  -1, 0,  0, -1, 0,  0,
	1, 0,  0,  1, 0,  0,  1,  0, 0,	 1,  0, 0,  0,	-1, 0, 0,  -1, 0,
	0, -1, 0,  0, -1, 0,  0,  1, 0,	 0,  1, 0,  0,	1,  0, 0,  1,  0
};
static const GLfloat cube_tex[] = { 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0,
				    1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1,
				    0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0,
				    1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1 };
static const GLubyte cube_indices[] = { 0,  1,	2,  0,	2,  3,	4,  5,	6,
					4,  6,	7,  8,	9,  10, 8,  10, 11,
					12, 13, 14, 12, 14, 15, 16, 17, 18,
					16, 18, 19, 20, 21, 22, 20, 22, 23 };

static long get_tid(void)
{
#ifdef SYS_gettid
	return syscall(SYS_gettid);
#else
	return (long)getpid();
#endif
}

static double ts_diff(const struct timespec *a, const struct timespec *b)
{
	return (double)(a->tv_sec - b->tv_sec) +
	       (double)(a->tv_nsec - b->tv_nsec) / 1e9;
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	if (!logger_init("perf_monitor.log", LOG_LEVEL_INFO)) {
		fprintf(stderr, "Failed to initialize logger.\n");
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
	thread_profile_start();
	InitGLState(&gl_state);
	Framebuffer *fb = GL_init_with_framebuffer(1024, 768);
	if (!fb) {
		LOG_FATAL("Failed to init framebuffer");
		return -1;
	}
#ifdef HAVE_X11
	X11Window *win = x11_window_create(1024, 768, "microGLES Perf");
#else
	X11Window *win = NULL;
#endif
	const int cube_count = 12;
	const int face_pixels = 64 * 64;
	GLubyte *tex = tracked_malloc(face_pixels * 4);
	memset(tex, 0xAA, face_pixels * 4);
	GLuint t;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, tex);
	tracked_free(tex, face_pixels * 4);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, cube_vertices);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, cube_normals);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, cube_tex);
	glEnable(GL_FOG);
	glFogf(GL_FOG_DENSITY, 0.5f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,
		       (GLfloat[]){ 0.1f, 0.1f, 0.1f, 1.0f });
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (GLfloat[]){ 0.9f, 0.9f, 0.9f, 1.0f });
	glLightfv(GL_LIGHT0, GL_POSITION,
		  (GLfloat[]){ 0.0f, 0.0f, 2.0f, 0.0f });
	mat4 model;
	mat4_identity(&model);
	for (int sec = 0; sec < 10; ++sec) {
		struct timespec start, now;
		uint64_t cstart, cend;
		clock_gettime(CLOCK_MONOTONIC, &start);
		cstart = thread_get_cycles();
		double polys = 0.0, pix = 0.0;
		do {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for (int c = 0; c < cube_count; ++c) {
				mat4_rotate_x(&model, 2.0f);
				mat4_rotate_y(&model, 3.0f);
				mat4_rotate_z(&model, 1.0f);
				glLoadMatrixf(model.data);
				glDrawElements(GL_TRIANGLES, 36,
					       GL_UNSIGNED_BYTE, cube_indices);
				polys += 12.0;
				pix += 6.0 * face_pixels;
			}
			thread_pool_wait();
#ifdef HAVE_X11
			if (win)
				x11_window_show_image(win, fb);
#endif
			clock_gettime(CLOCK_MONOTONIC, &now);
		} while (ts_diff(&now, &start) < 1.0);
		cend = thread_get_cycles();
		double wall = ts_diff(&now, &start);
		double cpu_us = thread_cycles_to_us(cend - cstart);
		double cpu_pct = wall > 0.0 ? (cpu_us / (wall * 1e6)) * 100.0 :
					      0.0;
		static bool header_printed = false;
		if (!header_printed) {
			printf("%-7s %-10s %-6s %-11s %-11s\n", "TID",
			       "Mem(KB)", "CPU%", "Poly(MP/s)", "Pix(MP/s)");
			header_printed = true;
		}
		printf("%7ld %10zu %6.1f %11.2f %11.2f\n", get_tid(),
		       memory_tracker_current() / 1024, cpu_pct,
		       polys / wall / 1e6, pix / wall / 1e6);
		thread_realtime_report();
		thread_profile_start();
	}
#ifdef HAVE_X11
	if (win)
		x11_window_destroy(win);
#endif
	thread_pool_wait();
	command_buffer_shutdown();
	thread_pool_shutdown();
	GL_cleanup_with_framebuffer(fb);
	CleanupGLState(&gl_state);
	memory_tracker_shutdown();
	logger_shutdown();
	return 0;
}
