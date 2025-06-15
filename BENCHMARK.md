# Benchmark Results

The `benchmark` executable exercises a collection of small scenes to
measure renderer performance.  The table below lists the scenes and the
frames-per-second (FPS) results observed on the reference machine.  Your
numbers may vary depending on hardware and compiler options.

Test system: Ubuntu 24.04, Intel Xeon Platinum 8370C (5 cores), 10 GiB RAM.

| Benchmark Scene            | FPS |
|----------------------------|-----|
| Triangle Strip (100)       | 174825 FPS |
| Triangle Strip (1000)      | 19853 FPS |
| Triangle Strip (10000)     | 2158 FPS |
| Textured Quad              | 578035 FPS |
| Lit Cube (lighting on)     | 326797 FPS |
| Lit Cube (lighting off)    | 473934 FPS |
| FBO operations             | 100000000 FPS |
| Pipeline Test              | 100000000 FPS |
| Spinning Gears             | 123609 FPS |
| Spinning Cubes             | 75233 MP/s |
| Multitexture Demo          | 2028 MP/s |
| Alpha Blend Demo           | 33333333 FPS |
| Fill Rate Suite            | N/A |
| Stress Test (1M cubes)     | 47 FPS |
