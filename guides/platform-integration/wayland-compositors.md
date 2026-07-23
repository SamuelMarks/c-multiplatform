# Wayland Compositors

For Linux deployments, the framework acts as a first-class Wayland client. It avoids legacy X11 overhead by talking directly to the Wayland display server, leveraging shared memory (SHM) or EGL for hardware-accelerated rendering.

## Architecture

When compiled for Wayland, the `ui_engine.h` spins up a dedicated Wayland event loop alongside the core UI event loop. It manages the `wl_display`, `wl_registry`, and binds to required protocols (like `xdg_wm_base` for window management and `wp_presentation` for accurate VSync).

## EGL vs. SHM Rendering

The framework supports two rendering backends on Wayland:

1.  **Shared Memory (wl_shm):**
    Software rasterization. The framework allocates a memory buffer, renders the UI pixels into it via CPU, and passes a file descriptor to the compositor. Excellent for headless environments, embedded devices without a GPU, or basic tooling.
2.  **EGL / Vulkan:**
    Hardware rasterization. The framework creates an `egl_window` and uses OpenGL ES or Vulkan to render directly to the GPU. This is required for complex shaders (`ui_effects_shaders.h`), rich animations, and 60+ FPS high-resolution rendering.

## Handling Wayland Events

The framework abstracts most of the low-level Wayland protocol, but you can hook into lifecycle events to handle window manager specific actions.

```c
#include "ui_engine.h"
#include "ui_wayland_integration.h" // Available only on Linux builds

void on_wayland_configure(ui_window_t* window, int width, int height, uint32_t states) {
    // Fired when the Wayland compositor requests a resize or state change
    // e.g., the user tiled the window to the left half of the screen

    if (states & UI_WAYLAND_STATE_MAXIMIZED) {
        printf("Window was maximized!\n");
    }

    // The engine automatically resizes the root component,
    // but you might want to adjust custom layout constraints here.
}

// Attach the listener during startup
ui_window_on_configure(my_app_window, on_wayland_configure);
```

## Subsurfaces and Video

For high-performance media playback (`ui_media.h`), the framework can utilize Wayland **Subsurfaces** (`wl_subsurface`).

Instead of copying video frames into the UI canvas, a subsurface allows a hardware video decoder (like VA-API) to composite video frames directly to the screen via the compositor, perfectly synced behind or in front of your UI layer.
