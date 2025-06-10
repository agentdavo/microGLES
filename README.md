#OpenGL ES 1.1 Renderer

Welcome to the microGLES OpenGL ES 1.1 Renderer project! This project provides a robust implementation of framebuffer objects (FBOs), renderbuffers, textures, matrix utilities, and comprehensive memory tracking to facilitate efficient and reliable rendering operations. The renderer is designed with performance and maintainability in mind, incorporating optimized data structures, thread safety, and detailed logging.

## Table of Contents

- [Features](#features)
- [Directory Structure](#directory-structure)
- [File Descriptions](#file-descriptions)
  - [Core Components](#core-components)
    - [GL State Management](#gl-state-management)
    - [Framebuffer Objects](#framebuffer-objects)
    - [Texture Management](#texture-management)
    - [Matrix Utilities](#matrix-utilities)
    - [Utility Functions](#utility-functions)
  - [Memory Tracking](#memory-tracking)
  - [Logging System](#logging-system)
  - [Main Application](#main-application)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Build Instructions](#build-instructions)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Framebuffer Objects (FBOs):** Create, bind, and manage FBOs with color, depth, and stencil attachments.
- **Renderbuffers:** Generate and configure renderbuffers with various internal formats.
- **Texture Management:** Create, bind, and manipulate textures, including mipmap generation.
- **Matrix Utilities:** Perform essential matrix operations for 3D transformations and projections.
- **Global State Management:** Maintain and track the state of OpenGL objects efficiently.
- **Memory Tracking:** Detect and report memory leaks with detailed allocation information.
- **Logging System:** Comprehensive logging with multiple severity levels for easy debugging.
- **Thread Safety:** Ensure safe operations in multi-threaded environments using mutexes.
- **Software Framebuffer:** Simple RGBA framebuffer for bare-metal targets with BMP output support.
- **Benchmark Suite:** Measure performance using tests like triangle strips,
  textured quads, framebuffer operations, a spinning gears demo,
  a fill-rate test with multiple textured cubes and fog,
  and a multitexture combiner demo.
- **Extension Querying:** Use `renderer_get_extensions()` to obtain the list of supported OpenGL ES extensions.
- **Extension Pack Stubs:** Basic implementations of the OpenGL ES 1.1 Extension Pack functions log a "not yet supported" message.
- **Fixed-Point API:** Extension entry points for the `GL_OES_fixed_point`
  functionality are declared in `gl_extensions.h` for devices that rely on
  fixed-point math.

## Directory Structure

```
OpenGLES_Renderer/
├── src/
│   ├── gl_framebuffer_object.c
│   ├── gl_framebuffer_object.h
│   ├── gl_state.c
│   ├── gl_state.h
│   ├── gl_texture.c
│   ├── gl_texture.h
│   ├── gl_utils.c
│   ├── gl_utils.h
│   ├── memory_tracker.c
│   ├── memory_tracker.h
│   ├── logger.c
│   ├── logger.h
│   ├── matrix_utils.c
│   ├── matrix_utils.h
│   └── main.c
├── include/
│   └── (Header files)
├── logs/
│   └── renderer.log
├── build/
│   └── (Build artifacts)
├── README.md
└── PROGRESS.md
```

## File Descriptions

### Core Components

#### GL State Management

- **`gl_state.h`**
  
  Defines the `GLState` structure, which maintains the current state of OpenGL objects such as renderbuffers, framebuffers, and textures. It also tracks the currently bound renderbuffer and framebuffer, as well as the default framebuffer.

- **`gl_state.c`**
  
  Implements the initialization and cleanup functions for the `GLState` structure. It manages the lifecycle of renderbuffers, framebuffers, and textures, ensuring proper memory management and state consistency.

#### Framebuffer Objects

- **`gl_framebuffer_object.h`**
  
  Declares the functions related to framebuffer object (FBO) operations, including creation, binding, attachment, and status checking. It ensures that all FBO-related functionalities are accessible to other components.

- **`gl_framebuffer_object.c`**
  
  Implements the FBO functionalities declared in `gl_framebuffer_object.h`. This includes creating, binding, deleting renderbuffers and framebuffers, attaching renderbuffers or textures to framebuffers, and checking framebuffer status. The implementation ensures thread safety and integrates with the global `GLState`.

#### Texture Management

- **`gl_texture.h`**
  
  Declares the `TextureOES` structure and functions for managing textures, including creation, deletion, binding, setting image data, and mipmap generation.

- **`gl_texture.c`**
  
  Implements the texture management functions declared in `gl_texture.h`. It handles the lifecycle of textures, ensuring they are correctly created, bound, and configured. The module also integrates with the memory tracker to monitor texture-related allocations.

#### Matrix Utilities

- **`matrix_utils.h`**
  
  Declares the `Matrix4` structure and functions for creating and manipulating 4x4 matrices. These utilities are essential for handling transformations, projections, and other matrix-based operations in 3D space.

- **`matrix_utils.c`**
  
  Implements the functions declared in `matrix_utils.h`. This file contains the logic for initializing matrices, applying transformations, multiplying matrices, and generating projection matrices. It ensures efficient and accurate matrix computations critical for 3D rendering.

#### Utility Functions

- **`gl_utils.h`**
  
  Provides declarations for utility functions used across the renderer, including memory management (`tracked_malloc`, `tracked_free`), error handling (`glSetError`), and framebuffer validation.

- **`gl_utils.c`**
  
  Implements the utility functions declared in `gl_utils.h`. This includes wrapping standard memory allocation functions with tracking capabilities, managing OpenGL error states, and providing helper functions for framebuffer validation.

### Memory Tracking

- **`memory_tracker.h`**
  
  Declares the memory tracking functions and macros used to monitor memory allocations and deallocations. It includes function prototypes for initialization, shutdown, allocation overrides, and memory statistics reporting. Macros are provided to override standard memory functions (`malloc`, `calloc`, `realloc`, `free`) with tracking-enabled versions.

- **`memory_tracker.c`**
  
  Implements a comprehensive memory tracking system that records each memory allocation with details such as pointer address, size, source file, and line number. It detects memory leaks upon shutdown by reporting allocations that were not properly freed. The tracker ensures thread safety using mutexes and maintains current and peak memory usage statistics.

### Logging System

- **`logger.h`**
  
  Declares the logging system functions and severity levels. It provides interfaces for logging messages at various levels (e.g., `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`).

- **`logger.c`**
  
  Implements the logging system declared in `logger.h`. It handles the initialization of log files, formatting of log messages, and outputting logs based on the configured severity level. The logger ensures that all modules can record relevant information for debugging and monitoring.

### Main Application

- **`main.c`**
  
  Demonstrates the integration of all components within a sample application. It initializes the logger and memory tracker, creates and configures renderbuffers and framebuffers, manages textures, sets up transformation matrices using the matrix utilities, and performs cleanup operations. The main application serves as an example of how to utilize the renderer's functionalities effectively.

## Getting Started

### Prerequisites

- **C Compiler:** Ensure you have a C compiler that supports the C11 standard (GNU extensions enabled).
- **OpenGL ES 1.1 Headers and Libraries:** Required for compiling OpenGL ES functionalities.
- **pthread Library:** For thread safety in the memory tracker.
- **Math Library:** For mathematical operations (`-lm` flag).
- **Make or Build System:** To compile the project.

### Build Instructions

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/agentdavo/microGLES.git
   cd microGLES
   ```

2. **Compile the Source Files:**

   Use `gcc` or another compiler to build the project. Here's an example using `gcc`:

   ```bash
   gcc -o renderer \
       src/main.c \
       src/gl_state.c \
       src/gl_texture.c \
       src/gl_framebuffer_object.c \
       src/matrix_utils.c \
       src/gl_utils.c \
       src/memory_tracker.c \
       src/logger.c \
       -Iinclude \
       -lGLESv1_CM -lpthread -lm
   ```

   **Notes:**

   - Adjust the include paths (`-Iinclude`) based on your project structure.
   - Link against the OpenGL ES library (`-lGLESv1_CM`), pthreads (`-lpthread`), and the math library (`-lm`).
   - Ensure all necessary source files are included in the compilation command.

3. **Build with CMake (Recommended):**

   ```bash
   cmake -S . -B build
   cmake --build build
   ./build/bin/renderer
   ```

   To build and run the benchmark suite:

   ```bash
  cmake --build build --target benchmark
  ./build/bin/benchmark
  ```

   To build and run the conformance tests:

   ```bash
   cmake --build build --target conformance
   ./build/bin/conformance
   ```

4. **Run the Application:**

   ```bash
#If you used the gcc example
   ./renderer

#If you built using CMake
   ./build/bin/renderer
   ```

## Usage

The renderer provides a set of functions to manage OpenGL ES objects and perform rendering operations. Here's a brief overview of typical usage patterns:

1. **Initialization:**

   Initialize the logger and memory tracker at the start of your application.

   ```c
   if (!InitLogger("renderer.log", LOG_LEVEL_DEBUG)) {
  fprintf(stderr, "Failed to initialize logger.\n");
  return -1;
}

if (!InitMemoryTracker()) {
  LOG_FATAL("Failed to initialize Memory Tracker.");
  return -1;
}

InitGLState(&gl_state);

const GLubyte *ext = renderer_get_extensions();
LOG_INFO("Supported extensions: %s", ext);
```

    2. *
    *Creating and Binding Renderbuffers : **

   ```c GLuint rb;
glGenRenderbuffersOES(1, &rb);
glBindRenderbufferOES(GL_RENDERBUFFER_OES, rb);
glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_RGBA4_OES, 256, 256);
```

    3. *
    *Creating and Binding Framebuffers : **

   ```c GLuint fb;
glGenFramebuffersOES(1, &fb);
glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb);
glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,
                             GL_RENDERBUFFER_OES, rb);
GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
if (status == GL_FRAMEBUFFER_COMPLETE_OES) {
  LOG_INFO("Framebuffer %u is complete.", fb);
} else {
  LOG_ERROR("Framebuffer %u is incomplete. Status: 0x%X", fb, status);
}
```

    4. **Creating and Managing Textures
    : **

   ```c TextureOES *tex =
          CreateTextureOES(GL_TEXTURE_2D_OES, GL_RGBA4_OES, 256, 256, GL_TRUE);
if (tex) {
  TexImage2DOES(tex, 0, GL_RGBA4_OES, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE,
                NULL);
  BindTextureOES(GL_TEXTURE_2D_OES, tex);
}
```

    5. *
    *Setting Up Transformation Matrices : **

   ```c Matrix4 model,
    view, projection, mvp;
Matrix4_InitIdentity(&model);
Matrix4_Translate(&model, 1.0f, 2.0f, -3.0f);
Matrix4_Rotate(&model, 45.0f, 0.0f, 1.0f, 0.0f);

Matrix4_InitIdentity(&view);
Matrix4_Translate(&view, 0.0f, 0.0f, -10.0f);

Matrix4_Perspective(&projection, 60.0f, 16.0f / 9.0f, 0.1f, 100.0f);

Matrix4_Multiply(&mvp, &view, &model);
Matrix4_Multiply(&mvp, &projection, &mvp);

Matrix4_Print(&mvp);
```

### Basic Rendering Example

The snippet below demonstrates a minimal render loop using the most common
OpenGL ES 1.1 commands:

```c
void render_frame(void) {
    glViewport(0, 0, 240, 160);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);
    static const GLfloat verts[6] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.0f,  0.5f
    };
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);
    glFlush();
}
```

    6. *
    *Cleanup : **

               Perform cleanup operations before exiting the application to
                   ensure all resources are properly released.

   ```c CleanupGLState(&gl_state);
ShutdownMemoryTracker(); // Reports any memory leaks
PrintMemoryUsage();      // Should show zero allocations if all are freed

/* Cleanup Logger */
CleanupLogger();
   ```

## Contributing

Contributions are welcome! Please follow these steps to contribute to the project:

1. **Fork the Repository:** Click the "Fork" button at the top-right corner of the repository page.

2. **Clone Your Fork:**

   ```bash
   git clone https://github.com/agentdavo/microGLES.git
   cd microGLES
   ```

3. **Create a New Branch:**

   ```bash
   git checkout -b feature/your-feature-name
   ```

4. **Make Your Changes:** Implement your feature or bug fix.

5. **Commit Your Changes:**

   ```bash
   git commit -m "Add feature: your feature description"
   ```

6. **Push to Your Fork:**

   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create a Pull Request:** Navigate to the original repository and create a pull request describing your changes.

## License

This project is licensed under the [MIT License](LICENSE). See the [LICENSE](LICENSE) file for details.