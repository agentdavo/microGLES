/* gl_texture.h */

#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <stddef.h>

#ifndef GL_TEXTURE_2D_OES
#define GL_TEXTURE_2D_OES 0x0DE1
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TextureOES {
  GLuint id;                  // Texture ID
  GLenum target;              // GL_TEXTURE_2D, etc.
  GLenum internalformat;      // GL_RGBA4_OES, etc.
  GLsizei width;              // Texture width
  GLsizei height;             // Texture height
  GLboolean mipmap_supported; // True if mipmaps are supported
  GLint current_level;        // Current mipmap level
} TextureOES;

/* Function prototypes */
TextureOES *CreateTextureOES(GLenum target, GLenum internalformat,
                             GLsizei width, GLsizei height, GLboolean mipmap);
void TexImage2DOES(TextureOES *tex, GLint level, GLenum internalformat,
                   GLsizei width, GLsizei height, GLenum format, GLenum type,
                   const void *pixels);
void BindTextureOES(GLenum target, TextureOES *tex);

#ifdef __cplusplus
}
#endif

#endif /* GL_TEXTURE_H */