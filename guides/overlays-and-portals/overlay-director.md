# Overlay Director

UIs frequently require elements that visually break out of the standard Document Object Model (DOM) flow. These include modal dialogs, tooltips, context menus, and dropdowns. The `ui_overlay_director.h` manages the lifecycle, stacking contexts (z-index), and focus trapping for these floating elements.

## The Problem with Standard Flow

If a dropdown menu is deeply nested in the DOM tree, its visual rendering can be clipped by parent elements utilizing `overflow: hidden`, or its `z-index` might be trapped in a lower stacking context.

The **Overlay Director** solves this by physically hoisting the floating UI elements into a special root-level container, while maintaining the *logical* connection to the component that spawned them.

## Using the Overlay Director

When you need to show a transient overlay, you register it with the director.

```c
#include "ui_overlay_director.h"
#include "ui_popover_base.h"

void show_user_profile_popover(ui_component_t* avatar_button) {
    // 1. Create the floating component
    ui_component_t* popover = create_profile_popover_ui();

    // 2. Configure anchoring strategy
    ui_overlay_config_t config = {
        .anchor = avatar_button,
        .placement = UI_PLACEMENT_BOTTOM_START,
        .offset_y = 8.0f, // 8px gap
        .flags = UI_OVERLAY_FLAG_DISMISS_ON_OUTSIDE_CLICK
    };

    // 3. Show the overlay
    ui_overlay_director_show(avatar_button->engine, popover, &config);
}
```

## Layers and Z-Index Management

The director maintains several distinct conceptual layers to guarantee that critical UI elements always render on top:

1.  **Application Layer (Base):** Standard DOM components.
2.  **Popup Layer:** Dropdowns, popovers, and tooltips.
3.  **Modal Layer:** Dialogs and bottom sheets. Accompanied by a darkened backdrop (`ui_backdrop.h`).
4.  **System Layer:** Urgent alerts and toasts.

When an overlay is pushed to the Modal Layer, the director automatically configures a **Focus Trap** (`ui_focus_trap.h`), preventing keyboard navigation (Tab/Shift+Tab) from interacting with the Application Layer beneath it.

## Lifecycle and Dismissal

Overlays can be dismissed programmatically or via user interaction (e.g., clicking outside the bounds or pressing the `Escape` key).

```c
// Programmatic dismissal
ui_overlay_director_dismiss(engine, popover);
```

When an overlay with `UI_OVERLAY_FLAG_DISMISS_ON_OUTSIDE_CLICK` is active, the director intercepts the global input stream. If a click falls outside the spatial bounds of the overlay, it drops the event and dismisses the overlay safely.
