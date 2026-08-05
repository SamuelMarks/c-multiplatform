# Predictive Back Navigation

Modern mobile and desktop operating systems heavily utilize gesture-based navigation. Predictive Back (`ui_predictive_back.h`) enhances the standard swipe-to-go-back gesture by visually revealing the destination screen *before* the user completes the swipe, allowing them to cancel the action dynamically.

## How it Works

Instead of instantly popping the current route off the stack, a predictive back gesture is a continuous stream of events. The framework manages the interpolation between the current screen and the previous screen in the background.

## Implementing the Gesture Handler

To support this, your navigation container needs to listen for the predictive back gesture events and update the view transition accordingly.

```c
#include "ui_predictive_back.h"
#include "ui_navigation.h"

// 1. Enable predictive back on your root navigator
ui_navigator_set_predictive_back_enabled(app_navigator, true);

// 2. The framework provides a default continuous transition.
// But you can customize the callback for bespoke animations (e.g., shrinking the active card).
void on_predictive_back_progress(ui_component_t* nav, float progress) {
    ui_component_t* current_screen = ui_navigator_get_current(nav);
    ui_component_t* previous_screen = ui_navigator_get_previous(nav);

    if (!previous_screen) return;

    // Example: Shrink the current screen and fade it out as progress goes from 0.0 to 1.0
    float scale = 1.0f - (progress * 0.1f); // Shrink to 90%
    float opacity = 1.0f - progress;

    ui_css_set_transform_scale(current_screen, scale, scale);
    ui_css_set_opacity(current_screen, opacity);

    // Reveal the previous screen underneath
    ui_css_set_opacity(previous_screen, progress);
}

ui_navigator_on_predictive_back(app_navigator, on_predictive_back_progress);
```

## Commit vs. Cancel

The gesture can end in two ways:
1.  **Commit:** The user swipes far enough and releases. The framework will fire a `commit` event, snap the progress to `1.0`, and officially pop the route.
2.  **Cancel:** The user swipes back to the origin edge. The framework will fire a `cancel` event, snap the progress back to `0.0`, and keep the current route active.

When using the built-in `ui_navigator` component, these state commits are handled automatically. If you are building a custom navigator, you must subscribe to `ui_predictive_back_on_commit` and handle the DOM manipulation yourself.

## Preventing Back Navigation

Sometimes, a screen shouldn't be swipe-dismissible (e.g., a form with unsaved changes).

```c
// Intercept the back gesture to show an alert instead
ui_error_t on_before_back(struct ui_component *screen, int *out_prevented) {
    if (form_is_dirty(my_form)) {
        show_unsaved_changes_dialog();
        *out_prevented = 1; // Prevent the back gesture
        return UI_ERROR_NONE;
    }

    *out_prevented = 0; // Allow
    return UI_ERROR_NONE;
}
```
