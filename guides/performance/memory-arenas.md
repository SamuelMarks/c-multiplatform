# Memory Arenas

In a high-performance C UI framework, minimizing `malloc` and `free` calls during the render loop is critical for achieving a stable 60+ FPS and preventing memory fragmentation. To solve this, the framework relies heavily on Memory Arenas (`ui_arena.h`).

## What is a Memory Arena?

A memory arena (often called a bump allocator) requests a large block of memory from the operating system upfront. Allocating memory from the arena simply involves incrementing an internal pointer.

Most importantly, you **do not free individual allocations**. Instead, the entire arena is reset or destroyed at once, reclaiming all memory instantly.

## Usage Patterns in the UI

The framework typically uses two primary arena lifecycles:

### 1. The Frame Arena

This arena is reset at the beginning of every frame. It is perfect for temporary calculations, layout constraints, string formatting, and event payload structures that only need to live for a single pass of the event loop.

```c
#include "ui_arena.h"
#include "ui_engine.h"

void on_render_frame(ui_engine_t* engine) {
    ui_arena_t* frame_arena = ui_engine_get_frame_arena(engine);

    // Fast allocation. No need to free this!
    char* debug_string = ui_arena_push_string(frame_arena, "Frame %d", engine->frame_count);
    ui_draw_text(..., debug_string);

    // The engine automatically resets frame_arena after rendering
}
```

### 2. Component/Tree Arenas

When a new UI component tree or complex route is mounted, an arena can be created specifically for it. All child components, their state, and their local data are allocated here. When the component is unmounted, the entire arena is destroyed, preventing memory leaks natively.

```c
ui_arena_t* my_view_arena = ui_arena_create(8192); // 8KB initial block

// Allocate components inside the view arena
ui_component_t* root = ui_component_create(my_view_arena, UI_TYPE_CONTAINER);
ui_component_t* btn = ui_component_create(my_view_arena, UI_TYPE_BUTTON);

// ... later, when tearing down the view ...
ui_arena_destroy(my_view_arena); // Automatically frees root, btn, and everything else
```

## Best Practices

*   **Never return pointers to frame-arena data** from a function if that data is expected to live across multiple frames.
*   **Use `ui_arena_push_array`** for dynamic lists that won't outlive their parent component.
*   **Avoid large single allocations** in small arenas; the arena will automatically chain new memory blocks, but this adds slight overhead.
