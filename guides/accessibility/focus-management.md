# Focus Management & Traps

Ensuring a logical and controllable keyboard focus flow is critical for accessibility (a11y) and power-user workflows. The `ui_focus_manager.h` and `ui_focus_trap.h` APIs give you absolute control over what the user can interact with via the `Tab` and `Shift+Tab` keys.

## The Focus Manager

The Focus Manager handles the global focus state of the application. It automatically calculates the logical "next" focusable element based on the DOM tree order.

```c
#include "ui_focus_manager.h"

// Programmatically focus a specific component (e.g., focusing the first input of a form on load)
ui_component_t* username_input = ui_input_create(arena);
ui_focus_manager_request_focus(engine->focus_manager, username_input);

// Query the currently focused element
ui_component_t* active_element = ui_focus_manager_get_active(engine->focus_manager);
```

### Making Components Focusable

By default, generic containers are not focusable. You must explicitly configure them to accept focus using the `tabindex` API.

```c
// Allows the component to receive focus via the Tab key
ui_dom_set_tab_index(my_custom_card, 0);

// Allows the component to receive focus programmatically, but skips it during Tab navigation
ui_dom_set_tab_index(my_hidden_input, -1);
```

## Focus Traps

When a modal dialog or full-screen menu opens, the user should not be able to press `Tab` and accidentally focus an element in the background application layer. A **Focus Trap** intercepts `Tab` navigation and cycles it infinitely within a specific container.

```c
#include "ui_focus_trap.h"

ui_component_t* modal_container = ui_container_create(arena);

// 1. Initialize the trap modifier
ui_modifier_t* trap = ui_focus_trap_create(arena);

// 2. Attach it to the modal
ui_component_add_modifier(modal_container, trap);

// Now, if the user presses Tab on the last element inside 'modal_container',
// the engine will instantly loop focus back to the first element in the container.
```

*Note: If you use the `ui_overlay_director.h` to display a modal, it automatically applies a focus trap for you.*

## Restoring Focus

When a trapped modal closes, it is an accessibility best practice to return focus to the button that originally opened it. The Focus Manager maintains a short history stack for this exact purpose.

```c
// Before opening the modal, the focus manager remembers the active button
ui_overlay_director_dismiss(engine, my_modal);

// The focus manager automatically pops the stack and refocuses the "Open Modal" button
```
