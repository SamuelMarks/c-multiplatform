# Layout Observers

While CSS Media Queries are great for styling based on the total window size, modern component-driven development requires components to react to *their own* size changes, regardless of the window. The `ui_layout_observer.h` API solves this by allowing C callbacks to fire when a component's bounding box changes.

## The Intersection Observer

The Intersection Observer detects when a component enters or exits the visible viewport (or a specific scrollable container). This is vital for lazy-loading images or triggering infinite scroll pagination.

```c
#include "ui_intersection_observer.h"

void on_visibility_changed(ui_component_t* target, bool is_visible, float ratio) {
    if (is_visible && !has_loaded_data) {
        printf("Component scrolled into view (%.2f%% visible). Fetching data...\n", ratio * 100.0f);
        fetch_heavy_data();
        has_loaded_data = true;
    }
}

// 1. Create the observer (observing the root viewport by default)
ui_modifier_t* observer = ui_intersection_observer_create(arena);

// 2. Attach the callback
ui_intersection_observer_on_change(observer, on_visibility_changed);

// 3. Attach the modifier to a component deep in a scroll view
ui_component_add_modifier(my_lazy_image, observer);
```

## The Resize Observer

The Resize Observer fires when a component's pixel dimensions change due to a window resize, a device rotation, or a change in the DOM structure (e.g., a sibling element was removed, giving this element more Flexbox space).

```c
#include "ui_layout_observer.h"

void on_component_resized(ui_component_t* target, float new_width, float new_height) {
    // Example: A custom chart component needs to recalculate its internal SVG/Canvas paths
    // whenever its layout bounding box changes.

    my_chart_recalculate_geometry(target, new_width, new_height);
    ui_component_invalidate_paint(target);
}

ui_modifier_t* resizer = ui_resize_observer_create(arena, on_component_resized);
ui_component_add_modifier(my_custom_chart, resizer);
```

## The Mutation Observer

The Mutation Observer (`ui_mutation_observer.h`) does not watch physical dimensions, but rather the logical DOM tree structure. It fires when children are added or removed from a target container, or when a specific CSS attribute changes.

This is highly useful for synchronization. For example, if you build a custom "Select" dropdown, you might want to observe the container so that whenever a user programmatically calls `ui_dom_append_child` to add a new `Option`, your custom UI automatically updates to reflect the new choice.
