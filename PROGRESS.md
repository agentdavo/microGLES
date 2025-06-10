# OpenGL ES 1.1 Implementation Progress

This file provides a snapshot of the renderer's API coverage. The list
below summarizes groups of entry points that are currently implemented.

## Core API
- Buffer objects (`glGenBuffers`, `glBindBuffer`, `glBufferData`,
  `glBufferSubData`, `glDeleteBuffers`)
- Framebuffer and renderbuffer objects (OES variants)
- Texture management (`glGenTextures`, `glBindTexture`, `glTexImage2D`,
  `glTexSubImage2D`, `glDeleteTextures`, etc.)
- Matrix stack operations (`glMatrixMode`, `glLoadIdentity`,
  `glMultMatrixf`, `glPushMatrix`, `glPopMatrix`)
- Drawing commands (`glDrawArrays`, `glDrawElements`)
- State queries (`glGet*`, `glIsEnabled`, `glIsTexture`, etc.)
- Viewport and scissor control (`glViewport`, `glScissor`)
- Lighting and material routines (`glLight`, `glMaterial`)
- Miscellaneous functions such as `glClear`, `glEnable`, `glDisable`,
  `glBlendFunc`, etc.

## Extensions
The renderer advertises the following OpenGL ES 1.1 extensions and
provides stub or full implementations for their entry points:

- GL_OES_draw_texture
- GL_OES_matrix_get
- GL_OES_point_size_array
- GL_OES_point_sprite
- GL_OES_framebuffer_object
- GL_OES_EGL_image
- GL_OES_EGL_image_external
- GL_OES_required_internalformat
- GL_OES_fixed_point
- GL_OES_texture_env_crossbar
- GL_OES_texture_mirrored_repeat
- GL_OES_texture_cube_map
- GL_OES_blend_subtract
- GL_OES_blend_func_separate
- GL_OES_blend_equation_separate
- GL_OES_stencil_wrap
- GL_OES_extended_matrix_palette

This document is updated whenever new groups of entry points are
implemented or existing stubs are filled in.
