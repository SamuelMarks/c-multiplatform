\page philosophy Theory & Philosophy

# Theory & Philosophy

LibCMPC is built upon a few core tenets that guide all architectural decisions:

## 1. Zero Dependencies
Software bitrots when the dependencies it relies on are abandoned or significantly altered. LibCMPC avoids this by shipping zero external runtime dependencies. Layouts, rendering queues, and data structures are all baked into the core.

## 2. Strict C89
By targeting ANSI C (C89), LibCMPC ensures that it can be compiled by virtually any compiler on the planet. From legacy embedded systems to modern WASM environments, the toolchain overhead is kept to an absolute minimum.

## 3. Retained Mode UI, Data-Driven Updates
While immediate mode UIs (IMGUI) are popular for their simplicity, they struggle with accessibility, predictive text, and power efficiency (constantly redrawing). LibCMPC uses a retained widget tree that only recalculates layout and repaints when explicitly dirtied.
