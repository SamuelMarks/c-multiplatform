# Getting Started: Application Lifecycle

This guide covers the fundamental boilerplate required to initialize the C-Multiplatform UI Engine, spawn a window, mount a reactive component, and run the main event loop.

## 1. Engine Initialization

Because the engine does not rely on global singletons, you must explicitly create and pass a `struct ui_engine` instance. This struct encapsulates the thread pools, memory arenas, and the reactive state graph.

```c
#include <stdio.h>
#include "ui_engine.h"
#include "ui_window.h"
#include "ui_execution_context.h"
#include "components/ui_button_base.h"

int main(int argc, char **argv) {
    struct ui_engine *engine = NULL;
    struct ui_window *window = NULL;
    struct ui_button_base *btn = NULL;
    ui_error_t err = UI_ERROR_NONE;

    /* 1. Initialize the core engine (allocates arenas and thread pools) */
    err = ui_engine_create(&engine);
    if (err != UI_ERROR_NONE) {
        printf("Failed to create UI Engine.\n");
        return 1;
    }

    /* 2. Create the OS-level window (Platform Abstraction Layer) */
    err = ui_window_create(engine, "Hello C-Multiplatform", 800, 600, &window);
    if (err != UI_ERROR_NONE) {
        goto cleanup;
    }

    /* 3. Instantiate a base component */
    err = ui_button_base_create(&btn);
    if (err != UI_ERROR_NONE) {
        goto cleanup;
    }

    /* 4. Mount the component to the window's root DOM node */
    ui_window_mount(window, ui_button_base_get_node(btn));

    /* 5. Enter the blocking execution loop */
    /* This function will not return until ui_window_close is called. */
    err = ui_execution_context_run(engine);

cleanup:
    /* Deterministic Teardown */
    if (btn) ui_button_base_destroy(btn);
    if (window) ui_window_destroy(window);
    if (engine) ui_engine_destroy(engine);

    return err == UI_ERROR_NONE ? 0 : 1;
}
```

## 2. The Execution Loop (`ui_execution_context`)

When you call `ui_execution_context_run`, the engine takes over the main thread.
The internal loop performs the following sequence every frame (ideally targeting 60 or 120 FPS, slaved to VSync or Audio PTS):

1.  **Poll Events:** The OS window backend is polled. Hardware events (Mouse, Keyboard, Window Resize) are mapped to `struct ui_event` and dispatched to the DOM.
2.  **Resolve Promises:** Any background thread tasks (e.g., a downloaded image) that resolved are executed here, ensuring their callbacks run safely on the main thread.
3.  **Process Signals:** The reactive graph is evaluated. Dirty `ui_computed` values are updated, and `ui_effect` closures are fired, which typically mutate the DOM.
4.  **Layout Pass:** If the DOM or CSSOM changed, the layout engine calculates strict geometric constraints (Flexbox/Grid/Block).
5.  **Render Pass:** The updated DOM is batched into OpenGL/WebGL draw calls and swapped to the display buffer.
6.  **Arena Reset:** The per-frame `ui_arena` memory allocator is instantly wiped, freeing all transient memory (layout nodes, string concatenations) in `O(1)` time.

## 3. Graceful Shutdown

To exit the application, a component or event handler must call `ui_engine_request_exit(engine)`. This sets an atomic flag that cleanly terminates the `ui_execution_context_run` loop, allowing your `main` function to proceed to the `goto cleanup` block.
