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
#include <GL/glx.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>
/* Pyramid benchmark parameters */
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define NUM_PYRAMIDS 1000

typedef struct {
	GLfloat x, y, z;
} Vec3;

typedef struct {
	Vec3 position;
	Vec3 rotation;
	Vec3 rotationSpeed;
} Pyramid;

static Pyramid pyramids[NUM_PYRAMIDS];
static GLfloat pyramid_vertex_data[18 * 3];
static GLfloat pyramid_color_data[18 * 4];

static float camera_rot_x = 0.0f;
static float camera_rot_y = 0.0f;
static float camera_distance = 300.0f;

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

static void init_gl(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT,
		       0.1f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

static void generate_pyramid_geometry(void)
{
	float size = 1.5f;
	float height = 3.0f;

	float apex[3] = { 0.0f, height / 2.0f, 0.0f };
	float base1[3] = { -size, -height / 2.0f, -size };
	float base2[3] = { size, -height / 2.0f, -size };
	float base3[3] = { size, -height / 2.0f, size };
	float base4[3] = { -size, -height / 2.0f, size };

	GLfloat *p = pyramid_vertex_data;
#define ADD_VTX(v)             \
	do {                   \
		*p++ = (v)[0]; \
		*p++ = (v)[1]; \
		*p++ = (v)[2]; \
	} while (0)
	ADD_VTX(apex);
	ADD_VTX(base1);
	ADD_VTX(base2);
	ADD_VTX(apex);
	ADD_VTX(base2);
	ADD_VTX(base3);
	ADD_VTX(apex);
	ADD_VTX(base3);
	ADD_VTX(base4);
	ADD_VTX(apex);
	ADD_VTX(base4);
	ADD_VTX(base1);
	ADD_VTX(base1);
	ADD_VTX(base3);
	ADD_VTX(base2);
	ADD_VTX(base1);
	ADD_VTX(base4);
	ADD_VTX(base3);
#undef ADD_VTX

	GLfloat *c = pyramid_color_data;
	for (int i = 0; i < 6; ++i) {
		float r = (float)rand() / RAND_MAX;
		float g = (float)rand() / RAND_MAX;
		float b = (float)rand() / RAND_MAX;
		for (int j = 0; j < 3; ++j) {
			*c++ = r;
			*c++ = g;
			*c++ = b;
			*c++ = 1.0f;
		}
	}
}

static void render_scene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -camera_distance);
	glRotatef(camera_rot_x, 1.0f, 0.0f, 0.0f);
	glRotatef(camera_rot_y, 0.0f, 1.0f, 0.0f);
	glVertexPointer(3, GL_FLOAT, 0, pyramid_vertex_data);
	glColorPointer(4, GL_FLOAT, 0, pyramid_color_data);
	for (int i = 0; i < NUM_PYRAMIDS; ++i) {
		glPushMatrix();
		glTranslatef(pyramids[i].position.x, pyramids[i].position.y,
			     pyramids[i].position.z);
		glRotatef(pyramids[i].rotation.x, 1.0f, 0.0f, 0.0f);
		glRotatef(pyramids[i].rotation.y, 0.0f, 1.0f, 0.0f);
		glRotatef(pyramids[i].rotation.z, 0.0f, 0.0f, 1.0f);
		glDrawArrays(GL_TRIANGLES, 0, 18);
		glPopMatrix();
	}
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
	GLXContext glx_ctx = NULL;
	if (win) {
		Display *dpy = x11_window_get_display(win);
		glx_ctx = glXCreateContext(dpy, NULL, NULL, False);
		glXMakeCurrent(dpy, (GLXDrawable)(uintptr_t)win, glx_ctx);
	}
#else
	X11Window *win = NULL;
#endif

	init_gl();
	generate_pyramid_geometry();
	for (int i = 0; i < NUM_PYRAMIDS; ++i) {
		pyramids[i].position.x = (float)(rand() % 300) - 150.0f;
		pyramids[i].position.y = (float)(rand() % 300) - 150.0f;
		pyramids[i].position.z = (float)(rand() % 300) - 150.0f;
		pyramids[i].rotation.x = (float)rand() / RAND_MAX * 360.0f;
		pyramids[i].rotation.y = (float)rand() / RAND_MAX * 360.0f;
		pyramids[i].rotation.z = (float)rand() / RAND_MAX * 360.0f;
		pyramids[i].rotationSpeed.x =
			((float)rand() / RAND_MAX - 0.5f) * 60.0f;
		pyramids[i].rotationSpeed.y =
			((float)rand() / RAND_MAX - 0.5f) * 60.0f;
		pyramids[i].rotationSpeed.z =
			((float)rand() / RAND_MAX - 0.5f) * 60.0f;
	}
	const int face_pixels = 64 * 64;
	for (int sec = 0; sec < 10; ++sec) {
		struct timespec start, now;
		uint64_t cstart, cend;
		clock_gettime(CLOCK_MONOTONIC, &start);
		cstart = thread_get_cycles();
		double polys = 0.0, pix = 0.0;
		do {
			render_scene();
			glFinish();
			for (int i = 0; i < NUM_PYRAMIDS; ++i) {
				pyramids[i].rotation.x +=
					pyramids[i].rotationSpeed.x * 0.016f;
				pyramids[i].rotation.y +=
					pyramids[i].rotationSpeed.y * 0.016f;
				pyramids[i].rotation.z +=
					pyramids[i].rotationSpeed.z * 0.016f;
			}
			polys += NUM_PYRAMIDS * 6.0f;
			pix += NUM_PYRAMIDS * 6.0f * face_pixels;
			thread_pool_wait();
#ifdef HAVE_X11
			if (win)
				glXSwapBuffers(NULL,
					       (GLXDrawable)(uintptr_t)win);
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
	if (glx_ctx) {
		glXDestroyContext(NULL, glx_ctx);
		glx_ctx = NULL;
	}
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
