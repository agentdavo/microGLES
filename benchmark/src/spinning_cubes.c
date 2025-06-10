#include "benchmark.h"
#include "gl_texture.h"
#include "gl_utils.h"
#include "logger.h"
#include "matrix_utils.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void run_spinning_cubes(BenchmarkResult *result) {
  const int cube_count = 12;
  const int frames = 100;
  const int face_pixels = 64 * 64; // approximate pixels per face
  GLubyte *tex = tracked_malloc(face_pixels * 4);
  memset(tex, 0xAA, face_pixels * 4);
  TextureOES *t =
      CreateTextureOES(GL_TEXTURE_2D_OES, GL_RGBA8_OES, 64, 64, GL_TRUE);
  if (t) {
    TexImage2DOES(t, 0, GL_RGBA8_OES, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    BindTextureOES(GL_TEXTURE_2D_OES, t);
  }
  glEnable(GL_FOG);
  glFogf(GL_FOG_DENSITY, 0.5f);

  mat4 model;
  mat4_identity(&model);

  double pixels_drawn = 0.0;
  clock_t start = clock();
  for (int f = 0; f < frames; ++f) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int c = 0; c < cube_count; ++c) {
      mat4_rotate_x(&model, 2.0f);
      mat4_rotate_y(&model, 3.0f);
      mat4_rotate_z(&model, 1.0f);
      glLoadMatrixf(model.data);
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, NULL);
      pixels_drawn += 6 * face_pixels; // 6 faces per cube
    }
  }
  clock_t end = clock();

  if (t)
    tracked_free(t, sizeof(TextureOES));
  tracked_free(tex, face_pixels * 4);

  compute_result(start, end, result);
  double secs = (double)(end - start) / CLOCKS_PER_SEC;
  result->pixels_per_second = pixels_drawn / secs;
  LOG_INFO("Spinning Cubes Fill Rate: %.2f MP/s",
           result->pixels_per_second / 1e6);
}
