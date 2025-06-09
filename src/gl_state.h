/* gl_state.h */

#ifndef GL_STATE_H
#define GL_STATE_H

#include <GLES/gl.h>
#include <GLES/glext.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations to avoid circular dependencies */
struct RenderbufferOES;
struct FramebufferOES;
struct TextureOES;

/* Maximum number of renderbuffers, framebuffers, and textures */
#define MAX_RENDERBUFFERS 1024
#define MAX_FRAMEBUFFERS 1024
#define MAX_TEXTURES 1024

/* GLState structure to manage OpenGL ES state */
typedef struct {
    GLuint next_renderbuffer_id;      // Next available renderbuffer ID
    struct RenderbufferOES* renderbuffers[MAX_RENDERBUFFERS]; // Array of renderbuffers
    GLint renderbuffer_count;         // Number of active renderbuffers
    struct RenderbufferOES* bound_renderbuffer; // Currently bound renderbuffer

    GLuint next_framebuffer_id;       // Next available framebuffer ID
    struct FramebufferOES* framebuffers[MAX_FRAMEBUFFERS]; // Array of framebuffers
    GLint framebuffer_count;          // Number of active framebuffers
    struct FramebufferOES* bound_framebuffer; // Currently bound framebuffer
    struct FramebufferOES default_framebuffer; // Default framebuffer (ID 0)

    GLuint texture_count;             // Number of active textures
    struct TextureOES* textures[MAX_TEXTURES]; // Array of textures
} GLState;

/* Global GLState instance */
extern GLState gl_state;

/* Function prototypes */
void InitGLState(GLState* state);
void CleanupGLState(GLState* state);

#ifdef __cplusplus
}
#endif

#endif /* GL_STATE_H */