# Headful E2E Testing

While headless testing (`ui_e2e_headless.h`) is incredibly fast and perfect for verifying DOM state and business logic, it does not utilize the actual GPU, OS window manager, or native input stack. To ensure pixel-perfect rendering and true OS integration, you use Headful End-to-End testing (`ui_e2e_headful.h`).

## The Headful Engine

A headful test physically opens a window on the machine running the test (or in a virtual framebuffer like Xvfb in CI environments). It utilizes the actual EGL/Vulkan/DirectX backend.

```c
#include "ui_e2e_headful.h"
#include "my_app.h"

void test_complex_shader_rendering() {
    // Spin up an actual OS window (e.g., 1280x720)
    ui_headful_engine_t* engine = ui_headful_engine_create(1280, 720);

    // Mount the app
    ui_component_t* app = my_app_create(engine->arena);
    ui_headful_mount(engine, app);

    // Wait for the window to appear and the first frame to render
    ui_headful_wait_for_idle(engine);

    // ... perform assertions ...

    ui_headful_engine_destroy(engine); // Closes the window
}
```

## Driving Native Input

Unlike headless tests where you dispatch synthetic C structs directly to the components, headful tests simulate input at the OS level. This tests the framework's platform integration layer (e.g., ensuring Wayland or Win32 pointer events are parsed correctly).

```c
// Move the actual OS mouse cursor to the center of the button
ui_component_t* submit_btn = ui_headful_query(engine, "#submit");
ui_headful_mouse_move_to(engine, submit_btn);

// Perform a real OS-level mouse click
ui_headful_mouse_click(engine, UI_MOUSE_BUTTON_LEFT);

// Type text using simulated OS keystrokes
ui_headful_keyboard_type(engine, "test_user");
```

## Visual Regression (Snapshot Testing)

Because headful tests run through the real GPU pipeline, they are the ideal place to perform Visual Regression testing. This ensures that a CSS change hasn't accidentally broken the visual layout.

```c
// Take a screenshot of a specific component (or the whole window)
ui_image_buffer_t* snapshot = ui_headful_take_snapshot(engine, submit_btn);

// Compare it against the "golden" reference image stored in the repo
// with a 1% pixel tolerance for slight GPU anti-aliasing differences.
bool matches = ui_e2e_compare_image(snapshot, "tests/snapshots/submit_btn_golden.png", 0.01f);

assert(matches);
```

## CI/CD Considerations

Running headful tests in a CI pipeline (like GitHub Actions) usually requires setting up a virtual display server, as CI runners do not have physical monitors attached.

For Linux runners, you typically use `xvfb-run`:

```bash
# Run the test suite inside a virtual X11 framebuffer
xvfb-run -a ./build/tests/e2e_headful_tests
```
