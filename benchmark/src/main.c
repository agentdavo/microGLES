#include "benchmark.h"
#include "gl_state.h"
#include "logger.h"
#include "memory_tracker.h"
#include <stdio.h>

int main() {
  if (!InitLogger("benchmark.log", LOG_LEVEL_INFO)) {
    fprintf(stderr, "Failed to initialize logger.\n");
    return -1;
  }
  if (!InitMemoryTracker()) {
    LOG_FATAL("Failed to initialize Memory Tracker.");
    return -1;
  }
  InitGLState(&gl_state);

  BenchmarkResult result;
  run_triangle_strip(100, &result);
  run_triangle_strip(1000, &result);
  run_triangle_strip(10000, &result);
  run_textured_quad(&result);
  run_lit_cube(1, &result);
  run_lit_cube(0, &result);
  run_fbo_benchmark(&result);
  run_spinning_gears(&result);
  run_spinning_cubes(&result);
  run_multitexture_demo(&result);
  run_alpha_blend_demo(&result);

  CleanupGLState(&gl_state);
  ShutdownMemoryTracker();
  PrintMemoryUsage();
  ShutdownLogger();
  return 0;
}
