/* gl_texture.c */
#include "gl_texture.h"
#include "gl_utils.h"
#include "logger.h"
#include "memory_tracker.h"

TextureOES *CreateTextureOES(GLenum target, GLenum internalformat,
                             GLsizei width, GLsizei height, GLboolean mipmap) {
  TextureOES *tex = (TextureOES *)tracked_malloc(sizeof(TextureOES));
  if (!tex) {
    LOG_ERROR("CreateTextureOES: Failed to allocate memory.");
    return NULL;
  }
  tex->id = 0; // Assign ID later
  tex->target = target;
  tex->internalformat = internalformat;
  tex->width = width;
  tex->height = height;
  tex->mipmap_supported = mipmap;
  tex->current_level = 0;
  tex->wrap_s = GL_REPEAT;
  tex->wrap_t = GL_REPEAT;
  tex->min_filter = GL_NEAREST_MIPMAP_LINEAR;
  tex->mag_filter = GL_LINEAR;
  return tex;
}

void TexImage2DOES(TextureOES *tex, GLint level, GLenum internalformat,
                   GLsizei width, GLsizei height, GLenum format, GLenum textype,
                   const void *pixels) {
  (void)level;
  (void)format;
  (void)textype;
  (void)pixels;
  tex->internalformat = internalformat;
  tex->width = width;
  tex->height = height;
  LOG_DEBUG("TexImage2DOES: Set texture ID %u to level %d, size %dx%d.",
            tex->id, level, width, height);
}

void BindTextureOES(GLenum target, TextureOES *tex) {
  (void)target;
  LOG_DEBUG("BindTextureOES: Bound texture ID %u.", tex ? tex->id : 0);
}
