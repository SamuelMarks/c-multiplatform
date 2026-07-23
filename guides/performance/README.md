# Performance & Memory Management

As a C-based framework, managing resources efficiently is critical for achieving consistent 60+ FPS rendering and minimizing power consumption.

## Core Topics

*   **[Memory Arenas](memory-arenas.md):** Understanding and utilizing `ui_arena.h` for zero-overhead, bulk-deallocated component memory management.
*   **Layout Optimization:** Avoiding layout thrashing and understanding the `ui_layout_observer.h` pipeline.
*   **Render Caching:** Strategies for caching expensive drawing operations.
*   **[Resource Management & Asset Streaming](asset-streaming.md):** Safely loading, streaming, and unloading large assets like images and fonts (`ui_asset_streamer.h`, `ui_image_decoder.h`).
