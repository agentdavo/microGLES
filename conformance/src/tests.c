#include "tests.h"
#include "gl_framebuffer_object.h"
#include "gl_texture.h"
#include "gl_utils.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>

int test_framebuffer_complete(void) {
  GLuint rb;
  glGenRenderbuffersOES(1, &rb);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, rb);
  glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_RGBA4_OES, 16, 16);

  GLuint fb;
  glGenFramebuffersOES(1, &fb);
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb);
  glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,
                               GL_RENDERBUFFER_OES, rb);
  GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
  glDeleteFramebuffersOES(1, &fb);
  glDeleteRenderbuffersOES(1, &rb);
  LOG_INFO("Framebuffer status 0x%X", status);
  return status == GL_FRAMEBUFFER_COMPLETE_OES;
}

int test_texture_creation(void) {
  TextureOES *tex =
      CreateTextureOES(GL_TEXTURE_2D_OES, GL_RGBA4_OES, 16, 16, GL_TRUE);
  if (!tex) {
    LOG_ERROR("CreateTextureOES failed");
    return 0;
  }
  TexImage2DOES(tex, 0, GL_RGBA4_OES, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  BindTextureOES(GL_TEXTURE_2D_OES, tex);
  tracked_free(tex, sizeof(TextureOES));
  LOG_INFO("Texture creation succeeded");
  return 1;
}

static int write_bmp(const char *filename, const unsigned char *data, int width,
                     int height) {
  FILE *f = fopen(filename, "wb");
  if (!f) {
    LOG_ERROR("Failed to open %s", filename);
    return 0;
  }
  int row_padded = (width * 3 + 3) & ~3;
  unsigned int filesize = 54 + row_padded * height;
  unsigned char file_header[14] = {'B', 'M'};
  file_header[2] = (unsigned char)(filesize);
  file_header[3] = (unsigned char)(filesize >> 8);
  file_header[4] = (unsigned char)(filesize >> 16);
  file_header[5] = (unsigned char)(filesize >> 24);
  file_header[10] = 54;
  fwrite(file_header, 1, 14, f);

  unsigned char info_header[40] = {0};
  info_header[0] = 40;
  info_header[4] = (unsigned char)(width);
  info_header[5] = (unsigned char)(width >> 8);
  info_header[6] = (unsigned char)(width >> 16);
  info_header[7] = (unsigned char)(width >> 24);
  info_header[8] = (unsigned char)(height);
  info_header[9] = (unsigned char)(height >> 8);
  info_header[10] = (unsigned char)(height >> 16);
  info_header[11] = (unsigned char)(height >> 24);
  info_header[12] = 1;
  info_header[14] = 24;
  fwrite(info_header, 1, 40, f);

  unsigned char *row = (unsigned char *)tracked_malloc(row_padded);
  for (int y = height - 1; y >= 0; --y) {
    const unsigned char *src = data + y * width * 3;
    memcpy(row, src, width * 3);
    memset(row + width * 3, 0, row_padded - width * 3);
    fwrite(row, 1, row_padded, f);
  }
  tracked_free(row, row_padded);
  fclose(f);
  LOG_INFO("Wrote %s", filename);
  return 1;
}

int test_framebuffer_colors(void) {
  const int w = 256;
  const int h = 256;
  unsigned char *buf = (unsigned char *)tracked_malloc(w * h * 3);
  if (!buf)
    return 0;
  struct ColorCase {
    const char *name;
    unsigned char r, g, b;
  } cases[] = {{"red.bmp", 255, 0, 0},
               {"green.bmp", 0, 255, 0},
               {"blue.bmp", 0, 0, 255},
               {"black.bmp", 0, 0, 0},
               {"white.bmp", 255, 255, 255}};
  int pass = 1;
  for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    for (int p = 0; p < w * h; ++p) {
      buf[p * 3 + 0] = cases[i].b;
      buf[p * 3 + 1] = cases[i].g;
      buf[p * 3 + 2] = cases[i].r;
    }
    if (!write_bmp(cases[i].name, buf, w, h))
      pass = 0;
  }
  tracked_free(buf, w * h * 3);
  return pass;
}
