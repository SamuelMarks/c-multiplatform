# Native OS Dialogs

While the framework allows you to build completely custom UI dialogs (`ui_dialog_base.h`), there are times when you must or should use the host operating system's native dialogs. This is particularly true for File Open/Save dialogs and critical system alerts.

The `ui_os_dialogs.h` header provides a cross-platform abstraction over these native APIs (Win32 `GetOpenFileName`, macOS `NSOpenPanel`, GTK `GtkFileChooser`, etc.).

## File Open & Save Dialogs

Opening a file picker pauses the C execution on some platforms, so these APIs are typically implemented asynchronously, invoking a callback when the user makes a selection.

```c
#include "ui_os_dialogs.h"

void on_file_selected(const char* file_path) {
    if (file_path) {
        printf("User selected: %s\n", file_path);
        // Load the file into the application
    } else {
        printf("User cancelled the dialog.\n");
    }
}

void prompt_for_image() {
    ui_os_dialog_file_config_t config = {
        .title = "Select an Image",
        .default_path = "~/Pictures",
        .filter_name = "Image Files (*.png, *.jpg)",
        .filter_extensions = "*.png;*.jpg;*.jpeg",
        .allow_multiple = false
    };

    // Shows the native OS file picker
    ui_os_dialog_open_file(&config, on_file_selected);
}
```

## System Alerts

If a fatal error occurs before the UI engine can initialize, or if you need to show an alert that blocks the entire OS windowing system, you can trigger a native message box.

```c
void show_fatal_error() {
    ui_os_dialog_alert_config_t config = {
        .title = "Application Error",
        .message = "Failed to initialize the Vulkan renderer. Please update your drivers.",
        .type = UI_OS_DIALOG_TYPE_ERROR,
        .buttons = UI_OS_DIALOG_BUTTONS_OK
    };

    // This is typically a synchronous, blocking call
    ui_os_dialog_show_alert(&config);
}
```

## Color Pickers

Some platforms (like macOS and Windows) provide rich, system-wide color pickers that users can drag around the screen and use to sample pixels from other applications.

```c
void on_color_picked(ui_color_t color) {
    ui_css_set_property(root, "--theme-color", ui_color_to_hex(color));
}

// Opens the floating OS color picker
ui_os_dialog_show_color_picker(initial_color, on_color_picked);
```
