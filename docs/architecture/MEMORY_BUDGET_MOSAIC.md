# Memory Budgeting for Mosaic View

## Goal
To render the Material 3 Component Mosaic view without exceeding tight memory budgets, targeting WebAssembly (e.g., 16MB minimum heap) and low-end embedded Android devices.

## Constraints
1. The grid can contain dozens of components.
2. If every component instantiates a full, interactive `cmp_ui_node_t` tree with hundreds of child elements, memory will spike linearly.
3. Rendering many complex nodes generates heavy layout passes and large VBO (Vertex Buffer Object) uploads.

## Budgeting Limits
- **Thumbnail Memory per Card:** Max 4KB of struct overhead.
- **Global Texture Cache:** 4MB max for pre-rendered proxies.
- **Node Limit:** The mosaic view should not exceed 2,000 active nodes at any time, even with a grid of 50 items.

## Strategy: Proxy Thumbnails
Instead of mounting real interactive widgets, we use "Proxy Layer" representations.
- **Vector Drawing Proxies:** We implement a lightweight `m3_thumbnail_render()` routine that uses basic `cmp_rect_t` and `cmp_circle_t` primitives to draw the *shape* of a button, slider, or switch, rather than building the actual `cmp_ui_button` component.
- **Virtualization:** The mosaic uses `CMP_FLEX_WRAP` combined with a windowing/virtualization observer. Only cards currently intersecting the viewport (plus a small overscroll buffer) are allocated and drawn. Cards out of view are discarded from the `cmp_pool_t`.

## Impact on Rendering Pipeline
The proxy shapes share the exact same style dictionary and color variables as the real components, meaning when a theme switch occurs, the proxies reflect the new colors instantaneously without needing the real components instantiated.
