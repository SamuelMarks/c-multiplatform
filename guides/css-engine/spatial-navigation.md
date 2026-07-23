# Spatial Navigation (D-Pad & TV)

When designing interfaces for Smart TVs, game consoles, or specialized embedded systems, users navigate via a directional pad (D-Pad) or arrow keys rather than a mouse. The framework implements the **W3C Spatial Navigation** specification (`ui_css_spatial_nav.h`) to handle this natively.

## How it Works

The engine automatically calculates the shortest geometric distance between focusable elements based on the direction pressed (Up, Down, Left, Right). You do not need to manually hardcode focus indices or "next focus" targets.

## Enabling Spatial Navigation

Spatial navigation is controlled primarily through CSS properties, meaning your layout dictates the navigation flow automatically.

```css
/* Enable spatial navigation for the entire application */
:root {
    spatial-navigation-action: focus;
}

/* Make sure elements clearly indicate focus */
.card:focus {
    outline: 4px solid #00E5FF;
    transform: scale(1.05);
}
```

## Focus Containers (Traps)

Sometimes, you want to restrict navigation to a specific zone (e.g., a modal dialog or a side menu) so the user doesn't accidentally navigate out of it. You use `spatial-navigation-contain`.

```css
.modal-dialog {
    /* Prevent the focus from escaping the modal via arrow keys */
    spatial-navigation-contain: contain;
}
```

## Overriding Navigation

If the automatic geometric calculation makes a poor choice (e.g., jumping across a large gap to an unintended button), you can explicitly override the target using `spatial-navigation-up`, `down`, `left`, or `right`.

```css
.btn-save {
    /* Force the 'Right' arrow to focus the specific element with ID 'btn-cancel' */
    spatial-navigation-right: url(#btn-cancel);
}
```

## C API Integration

You can programmatically trigger spatial navigation movements from C, which is useful if you are receiving raw gamepad input events.

```c
#include "ui_css_spatial_nav.h"

void on_gamepad_dpad_pressed(ui_engine_t* engine, int direction) {
    switch (direction) {
        case GAMEPAD_UP:
            ui_spatial_nav_move(engine, UI_SPATIAL_DIR_UP);
            break;
        case GAMEPAD_DOWN:
            ui_spatial_nav_move(engine, UI_SPATIAL_DIR_DOWN);
            break;
        // ...
    }
}
```
