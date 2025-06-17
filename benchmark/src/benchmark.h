#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <GLES/gl.h>
#include <stddef.h>
#include <time.h>
#include "pipeline/gl_framebuffer.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BenchmarkResult {
	double fps;
	double cpu_time_ms;
	double pixels_per_second;
	size_t current_memory;
	size_t peak_memory;
} BenchmarkResult;

void compute_result(clock_t start, clock_t end, BenchmarkResult *result);

void run_triangle_strip(int vertex_count, Framebuffer *fb,
			BenchmarkResult *result);
void run_textured_quad(Framebuffer *fb, BenchmarkResult *result);
void run_lit_cube(int lighting, Framebuffer *fb, BenchmarkResult *result);
void run_fbo_benchmark(Framebuffer *fb, BenchmarkResult *result);
void run_pipeline_test(Framebuffer *fb, BenchmarkResult *result);
void run_spinning_gears(Framebuffer *fb, BenchmarkResult *result);
void run_spinning_cubes(Framebuffer *fb, BenchmarkResult *result);
void run_multitexture_demo(Framebuffer *fb, BenchmarkResult *result);
void run_alpha_blend_demo(Framebuffer *fb, BenchmarkResult *result);
void run_milestone1(Framebuffer *fb, BenchmarkResult *result);
void run_milestone2(Framebuffer *fb, BenchmarkResult *result);
void run_texture_stream(Framebuffer *fb, BenchmarkResult *result);
void run_toggle_blend(Framebuffer *fb, BenchmarkResult *result);
void run_fill_rate_suite(Framebuffer *fb, BenchmarkResult results[3]);
void run_stress_test(Framebuffer *fb, BenchmarkResult *result, bool stream_fb,
		     int frames);

#ifdef __cplusplus
}
#endif

#endif /* BENCHMARK_H */
