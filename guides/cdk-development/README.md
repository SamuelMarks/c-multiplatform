# CDK Development: Building Base Components

The Component Development Kit (CDK) contains the unstyled, structural base components of the engine (e.g., `ui_button_base`). If you are implementing a completely new UI primitive (like a custom data grid or a knob), you must build a new `_base` component.

## 1. Structure of a Base Component

A base component is an opaque C struct that holds a root `ui_dom_node` and its internal signals.

```c
/* ui_knob_base.h */
struct ui_knob_base;

enum ui_error ui_knob_base_create(
    struct ui_engine *engine,
    struct ui_knob_base **out_knob,
    struct ui_control_value_accessor **out_cva /* Only needed for form controls */
);
struct ui_dom_node* ui_knob_base_get_node(struct ui_knob_base *knob);
void ui_knob_base_destroy(struct ui_knob_base *knob);
```

```c
/* ui_knob_base.c (Internal Implementation) */
struct ui_knob_base {
    struct ui_dom_node *root;
    struct ui_signal *value_sig;

    /* CVA vtable for forms integration */
    struct ui_control_value_accessor cva;
};
```

## 2. Instantiating the DOM and ARIA

Base components are responsible for DOM creation and Accessibility (ARIA) roles.

```c
enum ui_error ui_knob_base_create(struct ui_engine *engine, struct ui_knob_base **out_knob, struct ui_control_value_accessor **out_cva) {
    struct ui_knob_base *knob = malloc(sizeof(struct ui_knob_base));
    if (!knob) return UI_ERROR_OUT_OF_MEMORY;

    /* Create the root node */
    ui_dom_node_create(engine, "div", &knob->root);

    /* Enforce ARIA accessibility */
    ui_aria_set_role(knob->root, "slider");
    ui_aria_set_attribute(knob->root, "aria-valuemin", "0");
    ui_aria_set_attribute(knob->root, "aria-valuemax", "100");

    /* Make it focusable via Keyboard (Tab) */
    ui_dom_node_set_attribute(knob->root, "tabindex", "0");

    /* ... initialize signals and CVA ... */

    *out_knob = knob;
    return UI_ERROR_NONE;
}
```

## 3. Event Processing (The Core Logic)

A base component does not receive events automatically. The Design System wrapper (or the window root) explicitly forwards events to it via a `process_event` function.

```c
enum ui_error ui_knob_base_process_event(struct ui_knob_base *knob, const struct ui_event *event, double timestamp_ms) {
    if (event->type == UI_EVENT_KEY_DOWN) {
        if (event->event_data.keyboard.key_code == UI_KEY_UP) {
            /* Increment internal signal */
            int current = ui_signal_get_int(knob->value_sig);
            ui_signal_set_int(knob->value_sig, current + 1);
            return UI_ERROR_NONE; /* Event handled successfully */
        }
    }

    if (event->type == UI_EVENT_MOUSE_DOWN || event->type == UI_EVENT_TOUCH_START) {
        /* Handle mouse/touch drag math */
        return UI_ERROR_NONE;
    }

    return UI_ERROR_NONE;
}
```

## 4. Implementing the CVA (Control Value Accessor)

If your component is an interactive input, you must map its internal state to the `ui_control_value_accessor` vtable so it works with `ui_form_group`.

```c
/* Forward declarations of CVA callbacks */
static void write_value_cb(void *user_data, void *value);
static void register_on_change_cb(void *user_data, ui_cva_change_fn fn, void *fn_data);

/* Inside ui_knob_base_create: */
knob->cva.user_data = knob;
knob->cva.write_value = write_value_cb;
knob->cva.register_on_change = register_on_change_cb;

*out_cva = &knob->cva;
```
When the reactive form changes the value programmatically, the engine calls `write_value_cb`. When the user drags the knob, the knob must fire the callback registered via `register_on_change_cb`.
