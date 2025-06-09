/* main.c */

#include "gl_extensions.h"
#include "gl_framebuffer_object.h"
#include "gl_state.h"
#include "gl_texture.h"
#include "gl_utils.h"
#include "logger.h"
#include "memory_tracker.h"
#include <stdio.h>

int main() {

  /* Initialize Logger */
  if (!InitLogger("renderer.log", LOG_LEVEL_DEBUG)) {
    fprintf(stderr, "Failed to initialize logger.\n");
    return -1;
  }

  /* Initialize Memory Tracker */
  if (!InitMemoryTracker()) {
    LOG_FATAL("Failed to initialize Memory Tracker.");
    return -1;
  }

  /* Initialize GLState */
  InitGLState(&gl_state);

  /* Log supported extensions */
  const GLubyte *ext = renderer_get_extensions();
  LOG_INFO("Supported extensions: %s", ext);

  /* Your renderer initialization and operations */

  /* Example: Generate a renderbuffer */
  GLuint rb;
  glGenRenderbuffersOES(1, &rb);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, rb);
  glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_RGBA4_OES, 256, 256);

  /* Example: Generate a framebuffer */
  GLuint fb;
  glGenFramebuffersOES(1, &fb);
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb);
  glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,
                               GL_RENDERBUFFER_OES, rb);
  GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
  if (status == GL_FRAMEBUFFER_COMPLETE_OES) {
    LOG_INFO("Framebuffer %u is complete.", fb);
  } else {
    LOG_ERROR("Framebuffer %u is incomplete. Status: 0x%X", fb, status);
  }

  /* Example: Create a texture */
  TextureOES *tex =
      CreateTextureOES(GL_TEXTURE_2D_OES, GL_RGBA4_OES, 256, 256, GL_TRUE);
  if (tex) {
    TexImage2DOES(tex, 0, GL_RGBA4_OES, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE,
                  NULL);
    BindTextureOES(GL_TEXTURE_2D_OES, tex);
    /* Free the texture to avoid memory leaks */
    tracked_free(tex, sizeof(TextureOES));
  }

  /* Print memory stats */
  PrintMemoryUsage();

  /* Your renderer operations */

  /* Cleanup */
  CleanupGLState(&gl_state);
  ShutdownMemoryTracker(); // Should report any leaks
  PrintMemoryUsage();      // Should show zero allocations if all are freed

  /* Cleanup Logger */
  ShutdownLogger();

  return 0;
}
