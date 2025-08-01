# Project Overview

-----
# Description

This project was designed as a C++ demo for a case study on how to integrate Dear ImGui with GLFW and gl3w with modern C++ idom . It demonstrates polymorphic shape handling, custom rendering using OpenGL, and GUI integration via Dear ImGui. I intentionally avoided physics or external engines to focus on build tooling, architecture, and interaction design

# Design

## Core Libraries

### GLFW

  * **Purpose:** Cross-platform window creation and input handling.
  * **Provides:** Window management, OpenGL context creation, and keyboard/mouse event handling.
  * **Why Needed:** Abstracts operating system-specific windowing APIs across Linux, Windows, and macOS.

### gl3w

  * **Purpose:** OpenGL function loader.
  * **Provides:** Runtime loading of modern OpenGL functions.
  * **Why Needed:** Modern OpenGL requires explicit loading of function pointers at runtime.

### Dear ImGui

  * **Purpose:** Immediate-mode GUI library.
  * **Provides:** A suite of UI widgets (e.g., buttons, windows, input fields) rendered using OpenGL.
  * **Why Needed:** Facilitates the creation of user interfaces for elements such as map controls and coordinate lists.

## How They Work Together

```
GLFW   → Creates window + OpenGL context
gl3w   → Loads OpenGL functions for that context
ImGui  → Uses loaded OpenGL functions to render the UI
```

**Flow:** **GLFW** manages the window and user input. **gl3w** makes the necessary OpenGL functions available to the application. **ImGui** then renders the graphical user interface using these loaded OpenGL functions. Finally, the dev's application renders the map tiles within the same OpenGL context.

**Backends:** `imgui_impl_glfw.cpp` and `imgui_impl_opengl3.cpp` serve as integration backends, bridging ImGui with GLFW and OpenGL, respectively.

-----

# Compiling

## Linux

To download and extract dependencies, execute the following command:

```bash
docker run --rm -it -v "$(pwd)":/home/dev/app map_dev_env:latest bash -c "rm -r thirdparties; ./download_and_extract_dependancy.sh"
```

### Build Commands for Source Code

The **`BUILD_RELEASE`** flag controls the build type, allowing the dev to switch between "**release**" and "**debug**" modes. By default, the build will be in "release" mode.

#### Release Mode

To build in release mode:

```bash
docker run --rm -it -v "$(pwd)":/home/dev/app map_dev_env:latest bash -c "rm -r build && mkdir build && cd build && cmake .. && cmake --build ."
```

#### Debug Mode

If the dev plans to debug on their host machine, it's crucial to add the following line to their debugger configuration (.gdbinit). This is because the Docker build will generate symbol tables with paths specific to the Docker environment, which will almost certainly differ from the project's path on the host machine.

```
set substitute-path /home/dev/app <YOUR_PROJECT_PATH>
```

To build in debug mode:

```bash
docker run --rm -it -v "$(pwd)":/home/dev/app map_dev_env:latest bash -c "rm -r build && mkdir build && cd build && cmake .. -DBUILD_RELEASE=OFF && cmake --build ."
```

## Windows

TBD