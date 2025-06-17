# Portability Guidelines

microGLES aims to build on any platform with a C11 toolchain. The core sources
rely only on standard headers:

```c
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdalign.h>
```

For threading, include `portable/c11threads.h`. When the compiler provides the
standard `<threads.h>` header, it will be used. Otherwise the header falls back
to the compact `tinycthread` implementation so no additional dependencies are
required.

Other platform specific functions are isolated in `src/portable/` so new ports
only need to adjust that directory. Avoid OS APIs elsewhere and prefer the
helpers provided by the project.

Use CMake to generate project files for the target platform and enable the
standard build flags found in `README.md`.
