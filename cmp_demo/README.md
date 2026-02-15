C Multiplatform Demo
====================

This is a reference demo for `LibCMPC` implementing 3 screens with Material 3 widgets.

## Screens

1. **Login**: Text fields, layout, button logic.
2. **Showcase**: `M3Scaffold`, `CMPListView` (virtualized recycling), `M3Switch`, `M3Slider`, `M3Chip`.
3. **Detail**: Navigation depth, shared element transitions (simulated via layout).

## Build

The demo is included in the project structure.

```bash
mkdir build && cd build
cmake .. -DDEMO_BACKEND_SDL3=ON
cmake --build .
./cmp_demo_sdl3
```

## Platform Support

* **macOS**: `cmp_demo_cocoa.app` (Native Cocoa backend)
* **Windows**: `cmp_demo_win32.exe` (Native GDI backend)
* **Linux**: `cmp_demo_gtk4` (GTK4 backend)
* **Web**: `cmp_demo_web` (Emscripten/WebGL)
* **SDL3**: `cmp_demo_sdl3` (Cross-platform debug backend)

## Notes on Blank Window

If you see a blank window:

1. Ensure `demo_app_init_resources` is called.
2. Ensure fonts are mappable. The demo requests "System", "Segoe UI", or "Helvetica Neue".
3. Verify `cmp_render_list_execute` is called in the render loop.
