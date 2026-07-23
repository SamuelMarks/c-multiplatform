# Component State & Invalidation

In a reactive UI framework, when underlying data changes, the UI needs to update. However, re-rendering the entire DOM tree on every minor variable change would cripple performance. The `ui_component.h` module handles State and Invalidation to ensure efficient, targeted repaints.

## Local Component State

Every `ui_component_t` has a `void* user_data` pointer intended to hold the specific state for that instance.

```c
struct my_button_state {
    int counter;
    int is_hovered;
};

struct ui_component* create_my_counter_btn(struct ui_arena* arena) {
    struct ui_component* btn = ui_component_create(arena, UI_TYPE_BUTTON);

    // Allocate local state in the same arena
    struct my_button_state* state = ui_arena_push_type(arena, struct my_button_state);
    state->counter = 0;

    ui_component_set_user_data(btn, state);

    // ... setup callbacks ...
    return btn;
}
```

## The Invalidation Pipeline

When you mutate the `user_data`, the engine does not automatically know about it. You must signal the engine that the component's visual representation is out of sync with its logical state. You do this via **Invalidation**.

There are three levels of invalidation, ordered from least to most expensive:

1.  **`UI_INVALIDATE_PAINT`**: The state change only affects visual styling (e.g., color changed, text changed but bounding box remained exactly the same). The engine will just redraw the pixels.
2.  **`UI_INVALIDATE_LAYOUT`**: The state change might affect the component's physical dimensions (e.g., text became much longer, padding increased). The engine will recalculate the flexbox math for this component and its parents before painting.
3.  **`UI_INVALIDATE_DATA`**: (Specifically for Data Grids or Lists). The underlying array of data changed. The component needs to destroy its children and re-instantiate them from the data source before layout and paint.

```c
enum ui_error on_btn_clicked(struct ui_button_base *btn, void *user_data) {
    struct my_button_state *state = (struct my_button_state*)user_data;

    // Mutate state
    state->counter++;

    // Update the DOM text
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Clicked %d times", state->counter);

    struct ui_component *comp = NULL;
    ui_button_base_get_component(btn, &comp);
    ui_component_set_text(comp, buffer);

    // Signal the engine. Changing text almost always requires a layout recalculation.
    ui_component_invalidate(comp, UI_INVALIDATE_LAYOUT);

    return UI_ERROR_NONE;
}
```

## Global State (Providers)

For state that needs to be shared across many disconnected components (e.g., the currently logged-in user profile, or the active Theme), passing pointers down through 20 layers of UI is tedious.

The framework supports a Context/Provider pattern. You attach data to a parent node, and any deeply nested child can query up the tree to find it.

```c
// At the root of the app
ui_dom_provide_context(app_root, "active_user", my_user_struct);

// Deep inside a child component
my_user_t* user = ui_dom_consume_context(my_deep_avatar, "active_user");
```
