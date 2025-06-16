# Conformance Test Results

The conformance suite is table‑driven and covers most of OpenGL ES 1.1.
The following table summarizes the key tests executed via the
`renderer_conformance` target.

| Test                   | Result |
|------------------------|--------|
| Framebuffer completeness | Pass |
| Texture creation         | Pass |
| Framebuffer colors       | Pass |
| Software framebuffer     | Pass |
| Enable/Disable           | Pass |
| Viewport                 | Pass |
| Matrix stack             | Pass |
| Clear state              | Pass |
| Buffer objects           | Pass |
| Texture setup            | Pass |
| Blend func               | Pass |
| Scissor state            | Pass |
| Mask state               | Pass |
| Depth/Alpha/Stencil      | Pass |
| Call glLogicOp           | Pass |
| All entrypoints          | Pass |

The rasterizer now performs scissor clipping during fragment generation.

## Function Coverage

| Function | Test |
|----------|------|
| `glEnable`/`glDisable` | `enable_disable` |
| `glViewport` | `viewport` |
| `glMatrixMode` etc. | `matrix_stack` |
| `glClearColor`/`glClearDepthf`/`glClearStencil` | `clear_state` |
| `glGenBuffers`/`glBufferData` | `buffer_objects` |
| `glGenTextures`/`glTexImage2D` | `texture_creation`, `texture_setup` |
| `glBlendFunc` | `blend_func` |
| `glScissor` | `scissor_state` |
| `glColorMask`/`glDepthMask` | `mask_state` |
| `glAlphaFunc`/`glDepthFunc`/`glStencilFunc` | `depth_alpha_stencil` |
| `glLogicOp` | `call_glLogicOp` |
| *others* | `all_entrypoints` |
