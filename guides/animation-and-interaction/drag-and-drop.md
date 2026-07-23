# Drag & Drop

Rich desktop and web applications require robust drag-and-drop capabilities to reorder lists, move files, and organize layouts. The `ui_drag_drop.h` module provides a high-level API to handle these interactions across platforms.

## Making a Component Draggable

To initiate a drag, you configure a component as a "Drag Source" and provide the payload data it represents.

```c
#include "ui_drag_drop.h"

// Define the callback that provides the data when a drag actually starts
ui_drag_payload_t* on_drag_start(ui_component_t* source_component) {
    ui_drag_payload_t* payload = ui_drag_payload_create(source_component->engine);

    // Set MIME types for cross-application drag (e.g., dragging out to the OS)
    ui_drag_payload_set_string(payload, "text/plain", "file_123.txt");

    // Set a custom pointer for in-app drag (avoids serialization overhead)
    my_file_t* file_data = get_file_metadata();
    ui_drag_payload_set_pointer(payload, "app/x-my-file", file_data);

    return payload;
}

// Make the component draggable
ui_component_t* file_icon = ui_image_create(arena, "assets/file.png");
ui_drag_source_enable(file_icon, on_drag_start);
```

### The Drag Preview

When the user drags the component, the engine automatically generates a translucent visual preview (a snapshot of the component) that follows the cursor. You can customize this by providing a `drag_preview_renderer` callback to draw a custom badge or stack of items.

## Creating a Drop Target

To receive dragged items, you configure a component as a "Drop Target".

```c
#include "ui_drag_drop.h"

enum ui_error on_drag_enter(struct ui_component *target, struct ui_drag_payload *payload, int *out_accepted) {
    // Set out_accepted to 1 if this target accepts the payload's MIME type
    if (ui_drag_payload_has(payload, "app/x-my-file")) {
        ui_css_set_class(target, "drop-target-active", 1); // Highlight visually
        *out_accepted = 1;
        return UI_ERROR_NONE;
    }
    *out_accepted = 0; // Reject the drop (cursor changes to 'forbidden')
    return UI_ERROR_NONE;
}
```
enum ui_error on_drag_leave(struct ui_component *target) {
    // Clean up visual highlight
    ui_css_set_class(target, "drop-target-active", 0);
    return UI_ERROR_NONE;
}

enum ui_error on_drop(struct ui_component *target, struct ui_drag_payload *payload) {
    struct my_file *file = ui_drag_payload_get_pointer(payload, "app/x-my-file");

    // Process the drop logic (e.g., move the file to this folder)
    move_file_to_folder(file, target);

    return on_drag_leave(target);
}

// Make the folder container a drop target
struct ui_component *folder = NULL;
ui_container_create(&folder);
ui_drop_target_enable(folder, on_drag_enter, on_drag_leave, on_drop);
```

## Reordering Lists

For the specific (and very common) case of dragging items within a single list to reorder them, you don't need to implement raw drag-and-drop. The framework provides a `ui_list_set_reorderable(my_list, true)` modifier that handles the drag handles, placeholder insertion, and array mutation automatically.
