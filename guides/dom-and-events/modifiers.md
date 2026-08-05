# UI Modifiers (Composition over Inheritance)

A common pitfall in Object-Oriented UI frameworks is deep, rigid inheritance trees (e.g., `Button` inherits from `Control`, which inherits from `Node`, which inherits from `EventDispatcher`).

This framework avoids this by favoring **Composition**. The `ui_modifier.h` API allows you to attach reusable behaviors, event listeners, and dynamic styling to *any* component.

## What is a Modifier?

A modifier is a standalone piece of logic that hooks into a component's lifecycle (Init, Update, Paint, Destroy) and intercepts events.

## Creating a Hover Scale Modifier

Let's create a modifier that slightly enlarges a component when the user hovers over it.

```c
#include "ui_modifier.h"
#include "ui_cssom_api.h"

// Define the internal state of the modifier
struct hover_scale_state {
    int is_hovered;
};

// Event interceptor
ui_error_t on_hover_scale_event(struct ui_modifier *mod, const struct ui_event *event) {
    /* Example modifier state handling. Note: Actual struct fields depend on internal implementation */
    /* struct hover_scale_state *state = (struct hover_scale_state*)mod->user_data; */
    /* struct ui_component *target = mod->target; */

    if (event->type == UI_EVENT_MOUSE_MOVE) {
        /* state->is_hovered = 1; */
        /* ui_css_set_transform_scale(target, 1.05f, 1.05f); */
        return UI_ERROR_NONE; /* We handled it */
    }

    return UI_ERROR_NONE; /* Pass through to the component */
}

// Factory function
ui_error_t create_hover_scale_modifier(struct ui_modifier **out_mod) {
    struct ui_modifier *mod = NULL;
    ui_error_t err = ui_modifier_create(&mod);
    if (err != UI_ERROR_NONE) {
        return err;
    }

    /* mod->on_event = on_hover_scale_event; */

    *out_mod = mod;
    return UI_ERROR_NONE;
}
```

## Attaching Modifiers

Because modifiers are decoupled from the component type, you can attach this hover effect to a Button, an Image, a Card, or any generic container.

```c
struct ui_component *card = NULL;
ui_card_create(&card);

struct ui_modifier *hover_mod = NULL;
if (create_hover_scale_modifier(&hover_mod) == UI_ERROR_NONE) {
    ui_component_add_modifier(card, hover_mod);
}
```

## Chaining Modifiers

Modifiers are executed in the order they are added. This allows you to construct complex components entirely through composition:

```c
ui_component_t* custom_button = ui_container_create(arena);
ui_component_add_modifier(custom_button, create_ripple_effect(arena));
ui_component_add_modifier(custom_button, create_tooltip(arena, "Click me!"));
ui_component_add_modifier(custom_button, create_hover_scale_modifier(arena));
```
