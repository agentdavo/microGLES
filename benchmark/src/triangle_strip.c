#include "benchmark.h"
#include "gl_utils.h"
#include "logger.h"
#include "memory_tracker.h"
#include <string.h>

void run_triangle_strip(int vertex_count, BenchmarkResult *result) {
  GLfloat *verts = tracked_malloc(sizeof(GLfloat) * vertex_count * 3);
  for (int i = 0; i < vertex_count * 3; ++i) {
    verts[i] = (GLfloat)i / (GLfloat)vertex_count;
  }
  clock_t start = clock();
  for (int frame = 0; frame < 100; ++frame) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    volatile GLfloat sum = 0.f;
    for (int i = 0; i < vertex_count * 3; ++i)
      sum += verts[i];
    (void)sum;
  }
  clock_t end = clock();
  tracked_free(verts, sizeof(GLfloat) * vertex_count * 3);
  compute_result(start, end, result);
  LOG_INFO("Triangle Strip %d: %.2f FPS, %.2f ms/frame", vertex_count,
           result->fps, result->cpu_time_ms);
}
