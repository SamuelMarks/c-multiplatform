# Multi-Page Forms & Signal Navigation

This example demonstrates how to build a multi-step form (wizard) utilizing the C-Multiplatform Component Development Kit (CDK).

It showcases three core concepts:
1. **`ui_form_builder`**: Constructing a reactive form group safely in C89.
2. **Control Value Accessors (CVA)**: Two-way binding interactive widgets to form controls.
3. **`ui_router`**: Passing reactive state (`ui_form_group_t*`) across screens.

## Architecture Overview

In a typical setup, you allocate a long-lived `ui_arena` for the application state (the form group).
The `ui_router` matches paths (e.g., `"/step1"`, `"/step2"`) and instantiates screens using `ui_route_factory_t` callbacks. When navigating, the router passes the shared form state along so each screen can bind its components to the same underlying reactive graph.

---

## 1. Defining the Form & Router Setup

First, initialize the form builder, create the form structure, and set up the router.

```c
#include "ui_form_builder.h"
#include "ui_router.h"
#include "ui_arena.h"

/* ... initialization of engine/arena omitted for brevity ... */

/* Create the shared form state */
struct ui_form_builder *builder = NULL;
ui_form_builder_create(app_arena, &builder);

/* Create a form structure:
   {
      user: { name: "", email: "" },
      preferences: { notifications: true }
   }
*/
ui_form_builder_group_start(builder, "user");
{
    union ui_signal_payload name_val = { .ptr_val = "" };
    ui_form_builder_control(builder, "name", name_val, UI_SIGNAL_TYPE_POINTER, NULL, NULL);

    union ui_signal_payload email_val = { .ptr_val = "" };
    ui_form_builder_control(builder, "email", email_val, UI_SIGNAL_TYPE_POINTER, NULL, NULL);
}
ui_form_builder_group_end(builder);

ui_form_builder_group_start(builder, "preferences");
{
    union ui_signal_payload notif_val = { .bool_val = UI_TRUE };
    ui_form_builder_control(builder, "notifications", notif_val, UI_SIGNAL_TYPE_BOOL, NULL, NULL);
}
ui_form_builder_group_end(builder);

/* Build the root form group */
ui_form_group_t *root_form = NULL;
ui_form_builder_build(builder, &root_form);
ui_form_builder_destroy(builder);

/* Setup the Router */
struct ui_router *router = NULL;
ui_router_create(&router);

/* Register screen factories (defined below) */
ui_router_add_route(router, "/step1", factory_step_1, NULL);
ui_router_add_route(router, "/step2", factory_step_2, NULL);

/* Start the application by navigating to the first step, passing the form as state */
ui_router_navigate_with_state(router, "/step1", root_form);
```

---

## 2. Screen 1: User Info (Binding Controls)

The screen factory receives the `ui_route_request`, extracts the shared state (the form), creates input widgets, and wires them to the form using the Control Value Accessor pattern.

```c
#include "ui_input_base.h"
#include "ui_button_base.h"

ui_error_t factory_step_1(const struct ui_route_request *req, void *user_data, struct ui_component **out_screen) {
    /* 1. Extract the shared form state */
    ui_form_group_t *form = (ui_form_group_t *)ui_route_request_get_state(req);

    /* 2. Create the screen container (omitted layout code) */

    /* 3. Create Name Input Widget and extract its CVA */
    struct ui_input_base *name_input = NULL;
    struct ui_control_value_accessor *name_cva = NULL;
    ui_input_base_create(&name_input, &name_cva);

    /* 4. Bind the CVA to the form control "user.name" */
    struct ui_form_control *name_control = ui_form_group_get_control(form, "user.name");
    ui_form_control_bind_cva(name_control, name_cva);

    /* 5. Create a "Next" button. We bind its click event to navigate to Step 2 */
    struct ui_button_base *next_btn = NULL;
    ui_button_base_create(&next_btn);

    /* In a real app, this callback would trigger `ui_router_navigate_with_state(router, "/step2", form)` */
    /* Additionally, we could bind the button's `disabled_signal` to `ui_form_control_get_invalid_signal(name_control)` */

    return UI_ERROR_NONE;
}
```

---

## 3. Screen 2: Preferences

The second screen retrieves the exact same form pointer. Any updates made here instantly reflect in the reactive graph.

```c
#include "ui_slide_toggle_base.h"

ui_error_t factory_step_2(const struct ui_route_request *req, void *user_data, struct ui_component **out_screen) {
    /* Extract the exact same shared form state */
    ui_form_group_t *form = (ui_form_group_t *)ui_route_request_get_state(req);

    /* Create a Slide Toggle widget for notifications */
    struct ui_slide_toggle_base *toggle = NULL;
    struct ui_control_value_accessor *toggle_cva = NULL;
    ui_slide_toggle_base_create(&toggle, &toggle_cva);

    /* Bind to "preferences.notifications" */
    struct ui_form_control *notif_control = ui_form_group_get_control(form, "preferences.notifications");
    ui_form_control_bind_cva(notif_control, toggle_cva);

    /* Form submission logic would read the final values from `root_form` */
    return UI_ERROR_NONE;
}
```

## Summary
- **Data flows in one direction:** From the `ui_form_group` down to the widgets.
- **Events flow back up:** User interactions with the `ui_input_base` trigger the `CVA` callback, which updates the `ui_form_control` signal, firing validations and effects automatically.
- **State is preserved:** Because `ui_form_group_t` lives in a parent `ui_arena`, destroying Step 1's DOM components during navigation does not lose the user's entered data.
