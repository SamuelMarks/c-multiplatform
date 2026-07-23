# Command Palettes

Power users expect to navigate complex applications using only their keyboards. The Command Palette (`ui_command_palette_base.h`) provides an extensible, fuzzy-searchable interface, typically invoked via `Ctrl+K` or `Cmd+K`.

## Initializing the Palette

The command palette is an overlay component. You initialize it and register it with the hotkey registry to trigger its appearance.

```c
#include "ui_command_palette_base.h"
#include "ui_hotkey_registry.h"
#include "ui_overlay_director.h"

// 1. Create the palette instance
ui_component_t* cmd_palette = ui_command_palette_create(arena);

// 2. Register the global hotkey to show it
ui_hotkey_register(engine->hotkeys, "Cmd+K", on_toggle_palette, cmd_palette);

void on_toggle_palette(void* context) {
    ui_component_t* palette = (ui_component_t*)context;
    // Show in the modal layer, centered on screen
    ui_overlay_director_show_modal(palette->engine, palette);
}
```

## Registering Commands

Commands are registered into the palette with a title, an optional subtitle/description, an icon, and the callback to execute.

```c
void on_create_new_file(void* context) {
    printf("Creating new file...\n");
}

ui_command_t* cmd = ui_command_palette_add_action(
    cmd_palette,
    "File: New",
    "Create a new document",
    icon_document_add
);

ui_command_set_callback(cmd, on_create_new_file, NULL);
```

## Providers and Async Loading

For applications with thousands of potential commands (e.g., searching for a specific user out of a database of 100k users), you don't load them all into memory at once. Instead, you use a **Command Provider**.

A provider listens to the user's keystrokes and asynchronously yields results back to the palette.

```c
void search_users_provider(const char* search_term, ui_command_sink_t* sink) {
    // 1. Query the database based on `search_term`
    // 2. For each result, push a temporary command to the sink
    ui_command_sink_push(sink, "User: Alice Smith", icon_user, on_open_user);
    ui_command_sink_push(sink, "User: Bob Jones", icon_user, on_open_user);
}

// Register the provider
ui_command_palette_add_provider(cmd_palette, search_users_provider);
```

## Fuzzy Matching

By default, standard string matching is used. However, the palette includes a highly optimized fuzzy matcher (similar to fzf) to handle typos and partial matches gracefully, automatically highlighting the matched characters in the UI list.
