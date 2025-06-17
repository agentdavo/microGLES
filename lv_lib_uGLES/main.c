/**
 * @file lv_tinygl_test.c
 *
 * Simple LVGL demo embedding microGLES.
 */

#include "lvgl/lvgl.h"
#include "gl_init.h"
#include "gl_logger.h"
#include "gl_memory_tracker.h"
#include "gl_thread.h"
#include "pipeline/gl_framebuffer.h"
#include <GLES/gl.h>
#include <math.h>
#include <stdatomic.h>
#include <stdint.h>

#define CANVAS_WIDTH 160
#define CANVAS_HEIGHT 160

static lv_obj_t *canvas;
static lv_color_t
	cbuf[LV_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(CANVAS_WIDTH, CANVAS_HEIGHT)];

static Framebuffer *fb;
static uint32_t *lv_pixels;
static int width;
static int height;

static void anim(lv_task_t *t);
static void copy_fb(void);

void showmodel_update(void);
void showmodel_init(int w, int h, void *cbuf, char *filename);

void lv_tinygl_test(void)
{
	canvas = lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT,
			     LV_IMG_CF_TRUE_COLOR_ALPHA);
	lv_obj_align(canvas, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);

	showmodel_init(CANVAS_WIDTH, CANVAS_HEIGHT, cbuf,
		       "./lv_lib_tinygl/test.obj");

	lv_task_create(anim, 30, LV_TASK_PRIO_LOW, NULL);
}

static void anim(lv_task_t *t)
{
	(void)t;
	showmodel_update();
	lv_obj_invalidate(canvas);
}

static void copy_fb(void)
{
	uint32_t pixels = (uint32_t)width * (uint32_t)height;
	for (uint32_t i = 0; i < pixels; ++i)
		lv_pixels[i] = atomic_load(&fb->color_buffer[i]);
}

void showmodel_init(int w, int h, void *buffer, char *filename)
{
	(void)filename;
	width = w;
	height = h;
	lv_pixels = buffer;

	logger_init(NULL, LOG_LEVEL_INFO);
	memory_tracker_init();
	thread_pool_init_from_env();

	fb = GL_init_with_framebuffer((uint32_t)w, (uint32_t)h);
}

void showmodel_update(void)
{
	static float t = 0.f;
	glClearColor((sinf(t) * 0.5f + 0.5f), 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	thread_pool_wait();
	copy_fb();
	t += 0.1f;
}
