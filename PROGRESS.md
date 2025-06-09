# OpenGL ES 1.1 Implementation Progress

This document tracks which core OpenGL ES 1.1 entry points have
been implemented in the software renderer. The list is derived from
the official Khronos reference pages.

## Legend
- [x] Implemented
- [ ] Not yet implemented / stub

## Functions

- [x] glActiveTexture
- [x] glAlphaFunc

- [x] glBindBuffer
- [x] glBindTexture
- [x] glBlendFunc
- [x] glBufferData
- [x] glBufferSubData

- [x] glClear
- [x] glClearColor
- [x] glClearDepth
- [x] glClearStencil
- [x] glClientActiveTexture
- [x] glClipPlane
- [x] glColor
- [x] glColorMask
- [x] glColorPointer
- [x] glCompressedTexImage2D
- [x] glCompressedTexSubImage2D
- [x] glCopyTexImage2D
- [x] glCopyTexSubImage2D
- [x] glCullFace

### D
- [x] glDeleteBuffers
- [x] glDeleteTextures
- [x] glDepthFunc
- [x] glDepthMask
- [x] glDepthRange
- [x] glDisable
- [x] glDisableClientState
- [x] glDrawArrays
- [x] glDrawElements

### E
- [x] glEnable
- [x] glEnableClientState

### F
- [x] glFinish
- [x] glFlush
- [x] glFog
- [x] glFrontFace
- [x] glFrustum

### G
- [x] glGenBuffers
- [x] glGenTextures
- [x] glGet
- [x] glGetBufferParameteriv
- [x] glGetClipPlane
- [x] glGetError
- [x] glGetLight
- [x] glGetMaterial
- [x] glGetPointerv
- [x] glGetString
- [x] glGetTexEnv
- [x] glGetTexParameter

### H
- [x] glHint

### I
- [x] glIsBuffer
- [x] glIsEnabled
- [x] glIsTexture

### L
- [x] glLight
- [x] glLightModel
- [x] glLineWidth
- [x] glLoadIdentity
- [x] glLoadMatrix
- [x] glLogicOp

### M
- [x] glMaterial
- [x] glMatrixMode
- [x] glMultMatrix
- [x] glMultiTexCoord

### N
- [x] glNormal
- [x] glNormalPointer

### O
- [x] glOrtho

### P
- [x] glPixelStorei
- [x] glPointParameter
- [x] glPointSize
- [x] glPointSizePointerOES

## Fixed-Point Variants

The renderer also provides fixed-point versions of several functions.

- [x] glAlphaFuncx
- [x] glClearColorx
- [x] glClearDepthx
- [x] glClipPlanex
- [x] glColor4x
- [x] glDepthRangex
- [x] glFogx
- [x] glFogxv
- [x] glFrustumx
- [x] glGetClipPlanex
- [x] glGetFixedv
- [x] glLightx
- [x] glMaterialx
- [x] glMultMatrixx
- [x] glNormal3x
- [x] glOrthox
- [x] glPointParameterx
- [x] glPointParameterxv
- [x] glPointSizex
- [x] glPolygonOffsetx
- [x] glRotatex
- [x] glSampleCoveragex
- [x] glScalex
- [x] glTexEnvx
- [x] glTexEnvxv
- [x] glTexParameterx
- [x] glTexParameterxv
- [x] glTranslatex
- [x] glPolygonOffset
- [x] glPopMatrix
- [x] glPushMatrix

### R
- [x] glReadPixels
- [x] glRotate

### S
- [x] glSampleCoverage
- [x] glScale
- [x] glScissor
- [x] glShadeModel
- [x] glStencilFunc
- [x] glStencilMask
- [x] glStencilOp

### T
- [x] glTexCoordPointer
- [x] glTexEnv
- [x] glTexImage2D
- [x] glTexParameter
- [x] glTexSubImage2D
- [x] glTranslate

### V
- [x] glVertexPointer
- [x] glViewport

## Extensions

The following extension entry points are stubbed or implemented. They
correspond to the extension strings advertised by `renderer_get_extensions`.

- [x] glIsRenderbufferOES
- [x] glBindRenderbufferOES
- [x] glDeleteRenderbuffersOES
- [x] glGenRenderbuffersOES
- [x] glRenderbufferStorageOES
- [x] glGetRenderbufferParameterivOES
- [x] glIsFramebufferOES
- [x] glBindFramebufferOES
- [x] glDeleteFramebuffersOES
- [x] glGenFramebuffersOES
- [x] glCheckFramebufferStatusOES
- [x] glFramebufferTexture2DOES
- [x] glFramebufferRenderbufferOES
- [x] glGetFramebufferAttachmentParameterivOES
- [x] glGenerateMipmapOES
- [x] glDrawTexsOES
- [x] glDrawTexiOES
- [x] glDrawTexxOES
- [x] glDrawTexsvOES
- [x] glDrawTexivOES
- [x] glDrawTexxvOES
- [x] glDrawTexfOES
- [x] glDrawTexfvOES
- [x] glBlendEquationOES
- [x] glBlendFuncSeparateOES
- [x] glBlendEquationSeparateOES
- [x] glTexGenfOES
- [x] glTexGenfvOES
- [x] glTexGeniOES
- [x] glTexGenivOES
- [x] glGetTexGenfvOES
- [x] glGetTexGenivOES
- [x] glCurrentPaletteMatrixOES
- [x] glLoadPaletteFromModelViewMatrixOES
- [x] glMatrixIndexPointerOES
- [x] glWeightPointerOES
- [x] glPointSizePointerOES

