#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <GLES/gl.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BenchmarkResult {
  double fps;
  double cpu_time_ms;
  size_t current_memory;
  size_t peak_memory;
} BenchmarkResult;

void compute_result(clock_t start, clock_t end, BenchmarkResult *result);

void run_triangle_strip(int vertex_count, BenchmarkResult *result);
void run_textured_quad(BenchmarkResult *result);
void run_lit_cube(int lighting, BenchmarkResult *result);
void run_fbo_benchmark(BenchmarkResult *result);
void run_spinning_gears(BenchmarkResult *result);

#ifdef __cplusplus
}
#endif

#endif /* BENCHMARK_H */
