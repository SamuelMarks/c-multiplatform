# Data Grids & Tables

Displaying tabular data efficiently is a core requirement for many enterprise applications. The `ui_datagrid_base.h` component provides a high-performance, virtualized table capable of rendering millions of rows while maintaining 60 FPS.

## Virtualization by Default

The Data Grid does not create DOM nodes for every row in your dataset. Instead, it only instantiates UI components for the rows currently visible on the screen (plus a small buffer). As the user scrolls, these components are recycled and populated with new data.

## Setting Up a Data Grid

To use a data grid, you must define its columns and connect it to a data source (`ui_data_source.h`).

```c
#include "ui_datagrid_base.h"
#include "ui_data_source.h"

// 1. Initialize the grid component
ui_component_t* grid = ui_datagrid_create(arena);

// 2. Define columns (ID, Title, Width, Sortable)
ui_datagrid_add_column(grid, "id", "Employee ID", 100.0f, true);
ui_datagrid_add_column(grid, "name", "Full Name", 250.0f, true);
ui_datagrid_add_column(grid, "department", "Department", 150.0f, false);

// 3. Connect a data source
ui_data_source_t* employees = my_backend_fetch_employees();
ui_datagrid_set_source(grid, employees);
```

## Custom Cell Renderers

By default, the grid renders data as simple text. To display complex UI elements (like status badges, avatars, or action buttons) inside a cell, you provide a custom render callback.

```c
void render_status_badge(ui_component_t* cell_container, void* cell_data) {
    const char* status = (const char*)cell_data;

    ui_component_t* badge = ui_badge_create(cell_container->arena, status);

    if (strcmp(status, "Active") == 0) {
        ui_badge_set_color(badge, UI_COLOR_SUCCESS);
    } else {
        ui_badge_set_color(badge, UI_COLOR_DANGER);
    }

    ui_dom_append_child(cell_container, badge);
}

// Attach the renderer to a specific column
ui_datagrid_set_cell_renderer(grid, "status_col", render_status_badge);
```

## Sorting and Filtering

When a user clicks a sortable column header, the grid emits a sort event. If your data source is remote (e.g., a SQL database), you typically intercept this event, update your backend query, and refresh the data source. If the data is local, the grid can sort it in memory.

```c
void on_column_sort(ui_component_t* grid, ui_datagrid_sort_event_t event) {
    printf("Sorting by %s, ascending: %d\n", event.column_id, event.is_ascending);
    // ... trigger backend reload ...
}

ui_datagrid_on_sort(grid, on_column_sort);
```
