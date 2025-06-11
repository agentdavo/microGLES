When modifying code in this folder:

- **SHOULD** test C11 features with the toolchain to ensure compatibility and verify support for `alignas`, `_Generic`, `_Atomic`, and `__builtin_expect`.
- **MUST** prioritise OpenGL ES 1.1 compliance.
- **SHOULD** rely on the existing `gl_memory_tracker` and `gl_thread` modules instead of direct standard library allocation or threading calls.
- **MUST** verify builds with CMake:
  ```bash
  cmake -S . -B build -DCMAKE_C_FLAGS="-std=gnu11 -O3 -ftree-vectorize"
  cmake --build build
  ```
- **SHOULD** test debug and sanitizer builds:
  ```bash
  cmake -S . -B build_debug -DCMAKE_C_FLAGS="-std=gnu11 -Og -g -fsanitize=undefined,address"
  cmake --build build_debug
  ```
- Run the benchmark and conformance tests to verify runtime behaviour:
  `./build/bin/benchmark` and `./build/bin/conformance`.
- Update README.md if the public API or build steps change.
- Source files must be formatted via the `format` target before committing:
  `cmake --build build --target format`.
- Benchmarks and conformance tests follow the same rules.
