# Reactive Form Models

Handling complex forms with validation, dynamic fields, and deep state requires a structured approach. The framework uses a Reactive Form Model (`ui_form_builder.h`) that allows you to construct form state programmatically and independently from the UI layout.

## Core Concepts

*   **`ui_form_control_t`**: The fundamental building block. Represents a single input field (e.g., a text box, a checkbox). It tracks the value, validation status, and user interaction (dirty, touched).
*   **`ui_form_group_t`**: A collection of controls. Its validation status depends on all its children. Used for representing objects.
*   **`ui_form_array_t`**: An indexed list of controls. Useful for dynamic forms where the user can add multiple items (e.g., "Add another email").
*   **`ui_form_builder_t`**: A utility struct to streamline the creation of complex groups and arrays.

## Example: Building a Login Form

```c
#include "ui_form_builder.h"
#include "ui_form_validators.h"

ui_form_group_t* create_login_form(ui_arena_t* arena) {
    ui_form_builder_t fb = ui_form_builder_init(arena);

    // Create the group and define the controls
    ui_form_group_t* form = ui_form_builder_group(&fb, 2,
        ui_form_builder_control(&fb, "username", "",
            ui_validator_required()),

        ui_form_builder_control(&fb, "password", "",
            ui_validator_compose(2,
                ui_validator_required(),
                ui_validator_min_length(8)
            ))
    );

    return form;
}
```

## Binding to the UI

Once the model is created, you bind it to your visual components. When the user types, the UI component updates the `ui_form_control_t`, which automatically recalculates validation across the entire `ui_form_group_t`.

```c
// Assuming `username_input` is a UI text field component
ui_input_bind_control(username_input, ui_form_group_get(form, "username"));
```

## Checking State

You can query the form group at any time, typically when a "Submit" button is clicked:

```c
if (ui_form_is_valid(form)) {
    const char* username = ui_form_get_string(form, "username");
    // Proceed with authentication...
} else {
    // UI automatically highlights errors based on control state,
    // but you can also manually trigger an error banner here.
    ui_form_mark_all_touched(form);
}
```
