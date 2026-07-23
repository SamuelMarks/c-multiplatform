# View Transitions

The View Transitions API (`ui_css_view_transition.h`) provides a powerful mechanism for creating seamless visual transitions between different DOM states, without needing to manually orchestrate complex CSS animations for every element.

It works by taking a visual snapshot of the UI before a change, applying the DOM mutation, taking a snapshot after the change, and then letting the compositor crossfade and morph between the two states.

## Basic Usage

To trigger a transition, you wrap your state-changing logic inside a transition block.

```c
#include "ui_css_view_transition.h"
#include "ui_dom_node.h"

// Define the function that changes the DOM
void update_my_dom(void* user_data) {
    ui_component_t* container = (ui_component_t*)user_data;

    // Example: Swap out children
    ui_dom_remove_all_children(container);
    ui_dom_append_child(container, create_new_view());
}

// Trigger the transition
void on_button_click(ui_component_t* btn) {
    ui_component_t* root_container = get_root(btn);

    ui_start_view_transition(update_my_dom, root_container);
}
```

## Named Transitions

By default, the engine will crossfade the entire screen. To animate specific elements independently (e.g., a hero image moving from a list to a detail page), you must assign them matching `view-transition-name` properties in your CSS, or via the C API.

```c
// On the list item image
ui_css_set_property(list_image, "view-transition-name", "hero-banner");

// On the detail page image
ui_css_set_property(detail_image, "view-transition-name", "hero-banner");
```

When the transition runs, the engine will automatically calculate the layout differences between the two `hero-banner` elements and interpolate their size, position, and transforms.

## Customizing the Animation

You can customize the transition physics and duration using standard CSS pseudo-elements:

```css
::view-transition-group(hero-banner) {
    animation-duration: 0.5s;
    animation-timing-function: cubic-bezier(0.4, 0, 0.2, 1);
}
```
