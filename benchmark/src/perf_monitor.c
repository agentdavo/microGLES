#include "benchmark.h"
#include "gl_state.h"
#include "gl_init.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include "command_buffer.h"
#include "matrix_utils.h"
#include "gl_utils.h"
#include "pipeline/gl_framebuffer.h"
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

static bool check_fb_content(const Framebuffer *fb)
{
	bool has_non_white = false;
	bool has_non_black = false;
	for (uint32_t y = 0; y < fb->height; ++y) {
		for (uint32_t x = 0; x < fb->width; ++x) {
			uint32_t c = framebuffer_get_pixel(fb, x, y) &
				     0x00FFFFFFu;
			if (c != 0xFFFFFFu)
				has_non_white = true;
			if (c != 0x000000u)
				has_non_black = true;
			if (has_non_white && has_non_black)
				return true;
		}
	}
	return has_non_white && has_non_black;
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
	if (!win) {
		LOG_ERROR("x11_window_create failed. DISPLAY=%s",
			  getenv("DISPLAY"));
	} else {
		LOG_INFO("x11_window_create succeeded");
	}
	Display *dpy = NULL;
	GLXContext glx_ctx = NULL;
	if (win) {
		dpy = x11_window_get_display(win);
		int glx_major = 0, glx_minor = 0;
		if (!glXQueryVersion(dpy, &glx_major, &glx_minor)) {
			LOG_ERROR("glXQueryVersion failed");
		} else {
			LOG_INFO("GLX %d.%d available", glx_major, glx_minor);
		}
		static int visual_attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24,
					     GLX_DOUBLEBUFFER, None };
		XVisualInfo *vi =
			glXChooseVisual(dpy, DefaultScreen(dpy), visual_attr);
		if (!vi) {
			LOG_ERROR("glXChooseVisual failed");
		} else {
			LOG_INFO("glXChooseVisual succeeded");
		}
		glx_ctx = glXCreateContext(dpy, vi, NULL, True);
		if (glx_ctx) {
			LOG_INFO("glXCreateContext succeeded");
		} else {
			LOG_ERROR("glXCreateContext failed");
		}
		if (vi)
			XFree(vi);
		if (glx_ctx &&
		    glXMakeCurrent(dpy, (GLXDrawable)(uintptr_t)win, glx_ctx)) {
			LOG_INFO("glXMakeCurrent succeeded");
		} else if (glx_ctx) {
			LOG_ERROR("glXMakeCurrent failed");
		}
	}
#else
	X11Window *win = NULL;
#endif

	init_gl();
	generate_pyramid_geometry();
	LOG_INFO("Pyramid geometry generated");
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
	LOG_INFO("Entering render loop");
	const int face_pixels = 64 * 64;
	int frame_idx = 0;
	for (int sec = 0; sec < 10; ++sec) {
		LOG_INFO("Starting second %d", sec + 1);
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
				glXSwapBuffers(dpy,
					       (GLXDrawable)(uintptr_t)win);
#endif
			if (frame_idx < 2) {
				bool fb_ok = check_fb_content(fb);
#ifdef HAVE_X11
				bool win_ok =
					x11_window_has_non_monochrome(win);
#else
				bool win_ok = fb_ok;
#endif
				printf("Framebuffer frame %d %s (window %s)\n",
				       frame_idx + 1, fb_ok ? "PASS" : "FAIL",
				       win_ok ? "PASS" : "FAIL");
				++frame_idx;
			}
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
		LOG_INFO("Second %d summary: %.2f MP/s polys, %.2f MP/s pixels",
			 sec + 1, polys / wall / 1e6, pix / wall / 1e6);
		thread_realtime_report();
		thread_profile_start();
	}
#ifdef HAVE_X11
	if (glx_ctx) {
		glXMakeCurrent(dpy, None, NULL);
		glXDestroyContext(dpy, glx_ctx);
		glx_ctx = NULL;
	}
	if (win)
		x11_window_destroy(win);
#endif
	LOG_INFO("Render loop complete. Cleaning up");
	thread_pool_wait();
	command_buffer_shutdown();
	thread_pool_shutdown();
	GL_cleanup_with_framebuffer(fb);
	CleanupGLState(&gl_state);
	memory_tracker_shutdown();
	logger_shutdown();
	return 0;
}
