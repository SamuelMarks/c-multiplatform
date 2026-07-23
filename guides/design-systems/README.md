# Implementing Design Systems

This exhaustive guide covers how to layer distinct visual design systems (like Material 3, Fluent 2, or Cupertino) on top of the C-Multiplatform UI Engine's Component Development Kit (CDK).

## 1. The Component Development Kit (CDK) Paradigm

The CDK provides "headless" or unstyled components. When you instantiate a `ui_button_base` or `ui_slider_base`, you are not creating a painted rectangle; you are instantiating a DOM sub-tree governed by strict behavioral logic.

**What the CDK handles:**
*   **Accessibility (ARIA):** Setting `role="button"`, `aria-checked`, `aria-valuemin`.
*   **Focus & Keyboard:** Managing `tabindex`, capturing `Space` or `Enter` keys, mapping arrow keys to slider increments.
*   **Structural Layout Constraints:** Defining `min-width`, `display: flex`, and structural boundaries.
*   **Reactive Data Binding:** Tying internal node properties to the global `ui_signal` graph.

**What the Design System handles:**
*   Visual styles (Colors, Typography, Border Radii, Box Shadows).
*   Animations and Transitions (e.g., Material Ripple, Fluent Reveal Highlight).
*   Padding and subjective metrics (using Design Tokens).

## 2. The Golden Rules of Wrapping Components

### A. Never Duplicate State
A design system wrapper must never maintain its own parallel state.
*   *Wrong:* A `material_checkbox` struct containing an `int is_checked` boolean.
*   *Right:* A `material_checkbox` struct that delegates entirely to `ui_checkbox_base`. When the Material layer needs to render the checked visual, it reads the computed value from the underlying base component.

### B. Event Delegation & Interception
Events flow top-down. Your wrapper will intercept hardware events (Mouse, Touch, Keyboard) from the engine.
*   You **must** forward events to the base component so it can update its internal state.
*   You **may** intercept events to trigger visual side-effects.

```c
/* Example Event Delegation */
enum ui_error fluent_button_process_event(struct fluent_button *btn, const struct ui_event *event, double timestamp_ms) {
    if (event->type == UI_EVENT_MOUSE_MOVE) {
        /* Design System logic: Calculate and update the Fluent hover highlight position */
        update_fluent_reveal_effect(btn, event->event_data.mouse.x, event->event_data.mouse.y);
    }

    /* Core logic: Forward the event to the CDK base component so it handles clicks, focus, and ARIA */
    return ui_button_base_process_event(btn->base, event, timestamp_ms);
}
```

## 3. Control Value Accessor (CVA) Integration

If you are wrapping an interactive input (a Form Control), you must cleanly expose the underlying CVA so it can participate in `ui_form_group` reactive forms.

```c
struct fluent_input {
    struct ui_component *dom_node;
    struct ui_input_base *base;
};

/* The constructor */
enum ui_error fluent_input_create(
    struct ui_engine *engine,
    struct fluent_input **out_input,
    struct ui_control_value_accessor **out_cva /* EXPORT THE CVA */
) {
    struct fluent_input *input = malloc(sizeof(struct fluent_input));
    /* ... error checking ... */

    /* Create the base component and capture its CVA */
    ui_input_base_create(engine, &input->base, out_cva);

    /* Construct the DOM node for the Fluent skin */
    ui_component_create(engine, "fluent-input", &input->dom_node);

    /* Nest the base component inside the visual wrapper */
    ui_component_append_child(input->dom_node, ui_input_base_get_node(input->base));

    /* Inject Fluent-specific CSS */
    ui_component_add_class(input->dom_node, "fluent-v2-text-field");

    *out_input = input;
    return UI_ERROR_NONE;
}
```

## 4. Scoped CSS and Design Tokens

Design systems rely heavily on variables to support dynamic themes (Light/Dark mode) and high-contrast accessibility.

### Injecting CSS
Do not hardcode hex colors in C code. Instead, generate scoped CSS classes or utilize the CSS `@scope` module.
*   **[Design Tokens](design-tokens.md):** Load tokens into the global `:root` or CSS Environment (`ui_css_env.h`). E.g., `--md-sys-color-primary: #6750A4;`
*   **Binding Properties:** Map component properties to CSS variables using `ui_component_set_property(node, "--button-bg", "#f00")`.

When the user toggles dark mode, simply update the CSS Custom Properties at the root node. The CSSOM and Layout engine will automatically invalidate and repaint the affected components without a single C-level state change.

## 5. Utilizing Internal Animation Primitives

The engine provides structural primitives for animations (e.g., `ui_ripple_base` for touch feedback).
*   If you are building Material Design, append a `ui_ripple_base` as a sibling to your button's text node.
*   Map pointer down/up events to trigger the ripple. Do not write your own circle-expanding shader if the engine already provides the optimal, batched GL primitive.
