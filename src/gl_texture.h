/* gl_texture.h */

#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include <stddef.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    GLuint id;                // Texture ID
    GLenum target;            // GL_TEXTURE_2D, etc.
    GLenum internalformat;    // GL_RGBA4_OES, etc.
    GLsizei width;            // Texture width
    GLsizei height;           // Texture height
    GLboolean mipmap_supported; // True if mipmaps are supported
    GLint current_level;      // Current mipmap level
} TextureOES;

/* Function prototypes */
TextureOES* CreateTextureOES(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLboolean mipmap);
void TexImage2DOES(TextureOES* tex, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
void BindTextureOES(GLenum target, TextureOES* tex);

#ifdef __cplusplus
}
#endif

#endif /* GL_TEXTURE_H */