#include "gl_state.h"
#include "logger.h"
#include "memory_tracker.h"
#include "tests.h"
#include <stdio.h>

int main() {
  if (!InitLogger("conformance.log", LOG_LEVEL_INFO)) {
    fprintf(stderr, "Failed to init logger.\n");
    return -1;
  }
  if (!InitMemoryTracker()) {
    LOG_FATAL("Failed to init Memory Tracker.");
    return -1;
  }
  InitGLState(&gl_state);

  int pass = 1;
  if (!test_framebuffer_complete()) {
    LOG_ERROR("Framebuffer completeness test failed");
    pass = 0;
  }
  if (!test_texture_creation()) {
    LOG_ERROR("Texture creation test failed");
    pass = 0;
  }
  if (!test_framebuffer_colors()) {
    LOG_ERROR("Framebuffer color write test failed");
    pass = 0;
  }

  CleanupGLState(&gl_state);
  ShutdownMemoryTracker();
  PrintMemoryUsage();
  ShutdownLogger();

  if (pass) {
    printf("All tests passed\n");
    return 0;
  } else {
    printf("Tests failed\n");
    return 1;
  }
}
