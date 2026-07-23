# Dockable Layouts

Building complex desktop applications, such as Integrated Development Environments (IDEs), video editors, or trading terminals, often requires a highly customizable windowing system inside the main application shell. The `ui_dockable_layout_base.h` provides a robust engine for dragging, splitting, and docking panels.

## The Layout Tree

A dockable layout is represented as a tree of nodes. Nodes can be:
1.  **Splitters:** Divide the available space horizontally or vertically between two child nodes.
2.  **Tab Groups:** A container that holds multiple overlapping panels, displayed as tabs.
3.  **Panels:** The actual leaf nodes containing your application content (e.g., "Terminal", "File Explorer").

## Initializing a Dockable Layout

```c
#include "ui_dockable_layout_base.h"

ui_component_t* create_ide_layout(ui_arena_t* arena) {
    // 1. Create the root layout manager
    ui_dockable_layout_t* dock_sys = ui_dockable_layout_create(arena);

    // 2. Define the initial split (e.g., 25% left, 75% right)
    ui_dock_node_t* root_split = ui_dock_split_horizontal(dock_sys, 0.25f);

    // 3. Create left side (Sidebar)
    ui_dock_node_t* left_tabs = ui_dock_create_tab_group(dock_sys);
    ui_dock_tab_group_add(left_tabs, create_file_explorer_panel());

    // 4. Create right side (Editor and Terminal)
    ui_dock_node_t* right_split = ui_dock_split_vertical(dock_sys, 0.70f); // 70% top, 30% bottom

    ui_dock_node_t* editor_tabs = ui_dock_create_tab_group(dock_sys);
    ui_dock_tab_group_add(editor_tabs, create_code_editor_panel());

    ui_dock_node_t* terminal_tabs = ui_dock_create_tab_group(dock_sys);
    ui_dock_tab_group_add(terminal_tabs, create_terminal_panel());

    // 5. Wire the tree together
    ui_dock_node_set_children(root_split, left_tabs, right_split);
    ui_dock_node_set_children(right_split, editor_tabs, terminal_tabs);

    ui_dockable_layout_set_root(dock_sys, root_split);

    // Return the visual component representing the layout
    return ui_dockable_layout_get_component(dock_sys);
}
```

## Dragging and Docking

The framework handles user interaction automatically. When a user clicks and drags a tab:
1.  A floating preview window is generated.
2.  As the user drags over other panels, **Docking Drop Zones** (Center, Top, Bottom, Left, Right) are highlighted.
3.  When released, the layout tree is automatically mutated (e.g., dropping on the 'Left' zone creates a new horizontal splitter).

## Serialization

Users expect their custom layouts to persist across sessions. You can serialize the entire dockable tree state to JSON and restore it on startup.

```c
// Save state
char* json_state = ui_dockable_layout_serialize(dock_sys, frame_arena);
save_to_disk("layout.json", json_state);

// Restore state
ui_dockable_layout_deserialize(dock_sys, load_from_disk("layout.json"), panel_factory_callback);
```
