/**
 * @file WAYLAND_PLAN.md
 * @brief WAYLAND_PLAN.md implementation.
 */
/**
 * @file WAYLAND_PLAN.md
 * @brief WAYLAND_PLAN.md implementation.
 */
# Wayland Integration Plan

This document outlines the architecture and execution strategy for introducing native Wayland support to the C-Multiplatform UI engine on Linux, while preserving the existing X11 implementation as a fallback for non-Wayland environments.

## 1. Architectural Strategy: Compile-Time Toggling
Currently, `ui_window_backend_linux.c` statically binds to X11 and GLX. To prevent binary bloat and maintain a minimal footprint, we will use a **Compile-Time Build Toggle** instead of runtime dynamic dispatch.

- **Build Configuration:** A CMake option (`UI_ENABLE_WAYLAND`) will dictate which Linux backend implementation is compiled.
  - If `ON` (default on modern systems), the engine will strictly compile against the Wayland protocols and EGL.
  - If `OFF`, the engine will compile against the existing X11/GLX implementation.
- **Modularity:**
  - `src/ui_window_backend_linux_x11.c`: Contains the current `XOpenDisplay` and `glX` logic.
  - `src/ui_window_backend_linux_wayland.c`: Contains the new `wl_display_connect` and `EGL` logic.
  - The build system will map the implementation of `ui_window_backend_linux_create` from one of these files based on the CMake configuration, abstracting the choice away from the rest of the application code.

## 2. Build System & CMake Integration
We must extend the build system to gracefully detect and compile Wayland dependencies without breaking the build on older X11-only systems.

- Add a CMake option `UI_ENABLE_WAYLAND` (default ON for Linux).
- Use `pkg-config` (via `FindPkgConfig`) to locate:
  - `wayland-client`
  - `wayland-egl`
  - `egl`
- Use the `wayland-scanner` tool at build-time to generate the C headers and glue code for the `xdg-shell` protocol (required for creating application windows in modern Wayland compositors) from the system's `xdg-shell.xml` definition.
- If dependencies are missing, print a CMake warning and fall back to compiling the X11-only version automatically.

## 3. Wayland Native Implementation
The Wayland backend will be implemented strictly adhering to C89 and the `ui_window_backend` vtable structure.

### A. Initialization & Registry (`create_window`)
- Connect to the compositor (`wl_display_connect`).
- Bind to required globals via the `wl_registry`:
  - `wl_compositor` (for creating surfaces)
  - `xdg_wm_base` (for turning surfaces into application windows)
  - `wl_shm` (for software buffer fallbacks/cursors)
  - `wl_seat` (for future input handling)
- Create a `wl_surface`.
- Assign it the `xdg_surface` and `xdg_toplevel` roles, setting the window title natively.

### B. Rendering Context (EGL)
Wayland does not use GLX. We must pair Wayland with EGL to render OpenGL/GLES content.
- Use `wl_egl_window_create()` to bridge the `wl_surface` into an EGL-compatible native window.
- Initialize EGL (`eglGetDisplay`, `eglInitialize`).
- Choose an EGL configuration with 24-bit depth and RGBA support (`eglChooseConfig`).
- Create and activate the context (`eglCreateContext`, `eglMakeCurrent`).

### C. Event Loop (`poll_events`)
Wayland's event loop is strictly asynchronous and callback-driven.
- We will pump the Wayland queue using `wl_display_dispatch_pending()` and `wl_display_flush()`.
- Implement `xdg_toplevel` listener callbacks:
  - `.close` -> Maps to `UI_EVENT_WINDOW_CLOSE`.
  - `.configure` -> Maps to `UI_EVENT_WINDOW_RESIZE` (acknowledging the configure event natively).

### D. Buffer Swapping (`swap_buffers`)
- Map the backend `swap_buffers` call to `eglSwapBuffers()`.

## 4. Execution Steps
1. **Refactor Phase:** Isolate current X11 code into a specific `_x11.c` internal file and setup the dispatcher in `_linux.c`. Validate the build and execution.
2. **CMake Phase:** Add the `wayland-scanner` generator and library linking rules. Validate that it builds correctly on a Linux system with Wayland packages installed.
3. **Wayland Boilerplate:** Implement the `wl_registry` listener and EGL setup.
4. **Window Implementation:** Implement the `xdg_shell` listeners and surface creation. Ensure the `window_basic` example renders a blank window natively via Wayland.
5. **Renderer Bridge:** Validate that `ui_renderer_gles2.c` correctly targets the EGL context via standard OpenGL calls.
