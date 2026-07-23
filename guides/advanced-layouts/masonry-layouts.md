# Masonry Layouts

A masonry layout (often popularized by Pinterest) is a grid layout where items of varying heights are placed into columns, optimizing the use of vertical space by placing the next item into the shortest available column. The framework provides `ui_masonry_layout_base.h` to achieve this.

## Why not just CSS Grid?

While CSS Grid is incredibly powerful, creating a true, dynamic masonry layout where items flow into the *shortest* column regardless of DOM order is notoriously difficult with standard CSS Grid without pre-calculating row spans. The custom Masonry Layout manager handles this algorithmically at the C level.

## Creating a Masonry Grid

```c
#include "ui_masonry_layout_base.h"

// 1. Create the layout container
ui_component_t* photo_grid = ui_masonry_layout_create(arena);

// 2. Configure column count and spacing (gap)
ui_masonry_layout_set_columns(photo_grid, 3); // 3 columns
ui_masonry_layout_set_gap(photo_grid, 16.0f); // 16px gap between items

// 3. Add items of varying heights
ui_component_t* tall_img = ui_image_create_with_aspect(arena, "assets/tall.jpg", 0.5f);
ui_component_t* short_img = ui_image_create_with_aspect(arena, "assets/wide.jpg", 1.5f);

ui_dom_append_child(photo_grid, tall_img);
ui_dom_append_child(photo_grid, short_img);
// ... add more items ...
```

## Responsive Masonry

In most applications, you want the number of columns to change based on the viewport width (e.g., 1 column on mobile, 3 on desktop). You can achieve this by combining the Masonry Layout with a Breakpoint Observer (`ui_breakpoint_observer.h`).

```c
void on_breakpoint_changed(ui_component_t* grid, ui_breakpoint_t breakpoint) {
    if (breakpoint == UI_BREAKPOINT_MOBILE) {
        ui_masonry_layout_set_columns(grid, 1);
    } else if (breakpoint == UI_BREAKPOINT_TABLET) {
        ui_masonry_layout_set_columns(grid, 2);
    } else {
        ui_masonry_layout_set_columns(grid, 4);
    }
}

// Attach the observer
ui_breakpoint_observe(photo_grid, on_breakpoint_changed);
```

## Virtualized Masonry

If your masonry grid needs to display thousands of items (like an infinite scrolling image feed), you should use the `ui_data_source.h` binding variant of the masonry layout. This ensures that only the items currently visible in the viewport's scroll bounds are actually allocated and painted.
