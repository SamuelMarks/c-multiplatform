# Miller Columns

Miller Columns (also known as cascading lists) are a UI browsing technique used to display hierarchical data. Commonly seen in file managers (like macOS Finder), they allow users to navigate deep, nested folder structures without losing context of where they came from.

The framework provides this via `ui_miller_columns_base.h`.

## Core Concepts

A Miller Column component manages a horizontal scroll view containing multiple vertically-scrolling list components.
*   **Level 0:** The root data list.
*   **Level 1:** The children of the selected item in Level 0.
*   **Level N:** The children of the selected item in Level N-1.

## Implementing a Column Browser

Because a hierarchical tree (like a file system) can be massive, Miller Columns heavily rely on the `ui_data_source.h` to fetch children lazily.

```c
#include "ui_miller_columns_base.h"

// 1. Initialize the component
ui_component_t* browser = ui_miller_columns_create(arena);

// 2. Define the data fetching callback
// When the user clicks a folder, the framework calls this to populate the next column to the right.
void fetch_children_for_node(void* context, const char* node_id, ui_data_source_t* target_source) {
    // Example: Query the file system or a remote API
    my_fs_node_t* children = get_folder_contents(node_id);

    // Populate the data source for the new column
    for (int i = 0; i < children->count; i++) {
        ui_data_source_append(target_source, children[i].id, children[i].name);
    }
}

ui_miller_columns_set_fetcher(browser, fetch_children_for_node);

// 3. Set the root node to kick off the initial render
ui_miller_columns_set_root(browser, "root_folder_id");
```

## Preview Panes

In many applications, clicking a "leaf" node (a file, rather than a folder) shouldn't open a new column of data, but rather a "Preview Pane" displaying information about the item.

```c
// Customize what happens when a leaf node is selected
ui_component_t* render_preview_pane(ui_arena_t* arena, const char* node_id) {
    ui_component_t* pane = ui_container_create(arena);

    // Build a UI showing file metadata, a thumbnail, etc.
    ui_component_t* title = ui_label_create(arena, node_id);
    ui_dom_append_child(pane, title);

    return pane;
}

ui_miller_columns_set_preview_renderer(browser, render_preview_pane);
```

## Keyboard Navigation

The component comes with built-in accessibility and keyboard spatial navigation (`ui_css_spatial_nav.h`).
*   **Up/Down:** Navigates items within the currently focused column.
*   **Right:** Steps into the selected folder (focuses the next column).
*   **Left:** Steps back up the hierarchy (focuses the previous column).
