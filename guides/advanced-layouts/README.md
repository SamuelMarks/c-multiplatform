# Advanced Layouts & Observers

Beyond basic Flexbox and Grid, the framework provides highly specialized layout managers and reactive observer APIs to build responsive and dynamic interfaces.

## Core Topics

*   **Specialty Layouts:**
    *   **[Masonry Layouts](masonry-layouts.md):** Creating Pinterest-style cascading grid layouts (`ui_masonry_layout_base.h`).
    *   **[Dockable Layouts](dockable-layouts.md):** Building IDE-like interfaces with draggable, dockable panels (`ui_dockable_layout_base.h`).
    *   **Canonical Layouts:** Standardized app layouts for various screen sizes (`ui_canonical_layout_base.h`).
*   **[Observers](layout-observers.md):**
    *   **Intersection:** Detecting when elements enter or leave the viewport for lazy loading (`ui_intersection_observer.h`).
    *   **Mutation:** Watching for structural changes within the DOM tree (`ui_mutation_observer.h`).
    *   **Breakpoints:** Reacting to predefined viewport width/height thresholds (`ui_breakpoint_observer.h`).
*   **Geometry & Anchoring:** Precise positioning and mathematical geometry tools (`ui_geometry.h`, `ui_geometry_anchor.h`).
