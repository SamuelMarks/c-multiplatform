# Headless E2E Testing

Testing user interfaces traditionally requires a physical display server (like X11, Wayland, or Windows DWM), which makes running tests in Continuous Integration (CI) environments difficult. The framework provides a built-in headless testing engine (`ui_e2e_headless.h`) that completely simulates the rendering and input pipelines in memory.

## Setting Up a Headless Environment

A headless environment boots up a virtual compositor. It parses CSS, calculates layout, and can even execute software rasterization if visual assertions (snapshot testing) are needed.

```c
#include "ui_e2e_headless.h"
#include "my_app.h"

void test_login_flow() {
    // 1. Initialize the headless engine (e.g., 1024x768 virtual screen)
    ui_headless_engine_t* engine = ui_headless_engine_create(1024, 768);

    // 2. Mount the application root component
    ui_component_t* app_root = my_app_create(engine->arena);
    ui_headless_mount(engine, app_root);

    // 3. Advance the engine by one frame to compute initial layout
    ui_headless_tick(engine, 16); // 16ms (~60fps step)

    // ... continue with assertions ...

    ui_headless_engine_destroy(engine);
}
```

## Simulating User Input

The headless engine provides APIs to dispatch synthetic events exactly as the operating system would, ensuring your event handlers and focus management work as expected.

```c
// Find elements by accessibility label, ID, or CSS selector
ui_component_t* username_input = ui_headless_query(engine, "#username");
ui_component_t* submit_btn = ui_headless_query(engine, "[aria-label='Submit']");

// Simulate a user clicking the input and typing
ui_headless_dispatch_click(engine, username_input);
ui_headless_dispatch_text(engine, "test_user@example.com");

// Click the submit button
ui_headless_dispatch_click(engine, submit_btn);

// Tick the engine to process the event loop and trigger async tasks
ui_headless_tick(engine, 16);
```

## Assertions

Once input is simulated, you can assert against the resulting DOM state, CSSOM styles, or component properties.

```c
ui_component_t* error_banner = ui_headless_query(engine, ".error-banner");

// Assert the banner is now visible in the layout tree
assert(ui_headless_is_visible(engine, error_banner));

// Check computed styles
ui_color_t bg_color = ui_cssom_get_computed_color(error_banner, "background-color");
assert(bg_color.r == 255); // Expecting red
```
