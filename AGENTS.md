When modifying code in this folder:

1. Format all `.c` and `.h` files using `clang-format -i` before committing.
2. Ensure the project still builds using CMake:
   ```bash
   cmake -S . -B build -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize"
   cmake --build build
   ```
   You **SHOULD** also verify debug and sanitizer builds:
   ```bash
   cmake -S . -B build_debug -DCMAKE_C_FLAGS="-std=gnu11 -Og -g -fsanitize=undefined,address"
   cmake --build build_debug
   ```
3. Update README.md in this folder if the public API or build steps change
4. Benchmark sources under `benchmark/` follow the same rules.
5. The `benchmark/` directory contains performance demos exercising common
   OpenGL ES 1.1 rendering paths. When adding new benchmarks, update
   `BENCHMARK.md` with the measured results.
6. The `conformance/` directory provides functional tests that validate the
   renderer against the OpenGL ES 1.1 specification. Any new feature should be
   accompanied by a conformance test and the results recorded in
   `CONFORMANCE.md`.
7. Follow an iterative, block-by-block implementation process. After each
   block of functionality is added, immediately run the conformance suite to
   verify correct behaviour. This keeps the system reliable and adaptable to
   different hardware.
8. Source files must be formatted with `clang-format -i` using the
   configuration at the repository root (`.clang-format`).
9. Assume no standard library; use custom memory and threading APIs. Test C11
   features with the toolchain to ensure compatibility and verify support for
   `alignas`, `_Generic`, `_Atomic`, and `__builtin_expect`. Always prioritise
   OpenGL ES 1.1 compliance.
