# ADR 002: State Management for Cascading Theme and Language Changes

## Status
Accepted

## Context
When a user switches themes (e.g., Light to Dark) or languages (e.g., English to Arabic), the entire UI tree (`cmp_ui_node_t`) must react. A naive approach would be to tear down the entire UI tree, free the memory, and rebuild it from scratch. However, this causes:
1. Significant CPU spikes and dropped frames (jank).
2. Loss of transient state (e.g., cursor position in a text input, scroll offsets).
3. Heap fragmentation.

We need a way to cascade these global state changes down to existing nodes dynamically and repainting only what is necessary.

## Decision
We will implement **Dynamic Property Bindings** combined with the **Event Bus** and a **Double-Buffered State Tree**.

## Rationale
1. **The Event Bus (`CMP_EVENT_THEME_CHANGED`, `CMP_EVENT_LOCALE_CHANGED`):** The core loop (`cmp_modality_t`) will broadcast global state change events.
2. **Node Reactivity (`*_bind` attributes):** Instead of assigning hardcoded `cmp_color_t` values, developers assign keys via `cmp_ui_node_bind_bg_color(node, CMP_M3_COLOR_SURFACE)`. 
3. **Dirty Flags (`DIRTY_LAYOUT`, `DIRTY_PAINT`):** When the event bus receives a theme change, it invalidates the current computed theme cache and walks the `cmp_ui_node_t` tree. For any node with a bound property, it flags it with `DIRTY_PAINT`. If it's a locale change (which affects string lengths and writing direction), it flags `DIRTY_LAYOUT`.
4. **Transition Interpolation:** Since we are not tearing down the tree, the `cmp_compositor_anim_t` engine can take the *Current Frame* computed colors and the *Next Frame* computed colors and automatically schedule a 300ms cross-fade interpolation.
5. **Memory Pools:** Because the nodes themselves aren't destroyed and recreated, the `cmp_pool_t` slab allocator doesn't churn, maintaining stable memory footprints.

## Consequences
- **Memory Overhead:** Adding binding keys and dirty flags increases the byte size of `cmp_ui_node_t` slightly. We must use bitfields (`cmp_state_flags_t`) to pack these efficiently.
- **Complexity:** The layout and paint phases now have to dynamically resolve properties from the active Theme Dictionary or i18n Dictionary before applying them, adding a slight O(1) pointer-chasing cost during rasterization. This is mitigated by resolving these values once during the layout tick and storing the computed output.
