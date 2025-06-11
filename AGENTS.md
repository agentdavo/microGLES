# microGLES — Contributor Rules & AI-Agent Guide  
*(Applies to any code under `src/` and `benchmark/`)*

---

## 1  Golden Requirements (“**MUST**”)

| # | Requirement | Notes |
|---|-------------|-------|
| 1 | **OpenGL ES 1.1 conformance is paramount**. Any change failing the conformance suite is rejected. | Run `./build/bin/conformance` before every commit. |
| 2 | **Builds must succeed** in *release* and *debug/sanitizer* modes: | ```bash<br>cmake -S . -B build -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize"<br>cmake --build build<br><br>cmake -S . -B build_debug -DCMAKE_C_FLAGS="-std=gnu11 -Og -g -fsanitize=undefined,address"<br>cmake --build build_debug<br>``` |
| 3 | **Format all source** via the CMake `format` target *before* committing. | Runs `clang-format -i` with repo’s `.clang-format`. |
| 4 | **Never include new public GLES headers**. All user-visible symbols come from `<GLES/gl.h>` and `<GLES/glext.h>`. |
| 5 | **Memory & threading** must route through our wrappers: | *Allocation* → `gl_memory_tracker` • *Threads* → `gl_thread` |
| 6 | **Unit, benchmark, and conformance tests must pass**. | Benchmarks live in `benchmark/`; invoke `./build/bin/benchmark`. |
| 7 | **Commit messages** start with a one-line summary ≤ 72 chars, followed by wrapped body text. |

---

## 2  Strong Recommendations (“**SHOULD**”)

| # | Recommendation | Rationale |
|---|----------------|-----------|
| 1 | **Verify C11 built-ins** (`alignas`, `_Atomic`, `_Generic`, `__builtin_expect`) with your toolchain before using them. | Bare-metal targets sometimes lack full support. |
| 2 | **Keep modules narrow:** | *API* → `src/gl_api_*.c` • *Pipeline* → `src/pipeline/*.c` • *Utilities* → `src/*.c` (logger, memory, math). |
| 3 | **No new public headers** unless they are Khronos-supplied. Private helpers go in `*.h` under `src/` and are included with `#include "..."`. |
| 4 | **One concern per file**:<br>`gl_api_texture.c` handles only texture state; `gl_api_draw.c` only issues draw calls, etc. |
| 5 | **Validate** error paths (`glGetError`) with negative tests after touching API code. |
| 6 | **Use `valgrind`, `gdb`, or compiler sanitizers** to root-cause any test failure before pushing. |
| 7 | **Document tricky sections**: <br>  • coordinate-space conversions in the vertex stage <br>  • atomic depth/stencil writes in the fragment stage <br>  • thread-pool contracts. |

---

## 3  File-Structure Conventions

````

src/
├─ gl\_api\_state.c          // glEnable/Disable + queries
├─ gl\_api\_matrix.c         // matrix stack ops
├─ gl\_api\_vertex\_array.c  // client arrays setup
├─ gl\_api\_draw\.c          // draw commands (arrays, elements)
├─ gl\_api\_texture.c        // texture objects & TexEnv
├─ gl\_api\_lighting.c       // light & material state
├─ gl\_api\_depthstencil.c   // depth & stencil funcs / masks
├─ gl\_api\_blend.c          // blend factors (already exists)
├─ gl\_api\_pixels.c         // glClear, glReadPixels, masks
├─ gl\_api\_fbo.c            // FBO / RBO extension glue
├─ …                         // utilities, logger, memory
└─ pipeline/
├─ gl\_vertex.c          // transform + lighting
├─ gl\_primitive.c       // assemble + cull + clip
├─ gl\_raster.c          // edge setup, tile jobs
├─ gl\_fragment.c        // shade + blend + write
└─ gl\_framebuffer.c     // atomic depth/stencil/color

```

*No API file exposes new public prototypes.*  
Shared helpers (e.g. `lookup_buffer()`) live in a **private** header under `src/`.

---

## 4  Commit Checklist (quick version)

1. `cmake …` builds succeed (release & sanitizer).
2. `./build/bin/conformance` **PASS**.
3. `./build/bin/benchmark` runs; no > 5 % perf regression unless justified.
4. `valgrind --error-exitcode=1` on main demo shows **0 leaks / invalid writes**.
5. `git clang-format HEAD~1` → no diff.
6. Update `README.md` **if** build flags or public behaviour changed.
7. Push / merge request.

---
