# Motion & Physics

Static interfaces feel rigid and unresponsive. The framework integrates a physics-based motion engine (`ui_css_motion.h`) that allows you to animate properties using standard CSS keyframes or realistic spring physics.

## CSS Transitions & Keyframes

The most common way to animate is using standard CSS `transition` and `@keyframes` declarations. The framework's CSSOM fully supports these.

```css
.button {
    background-color: blue;
    transform: scale(1.0);
    /* Animate changes to background-color and transform over 200ms */
    transition: background-color 200ms ease-in-out, transform 200ms cubic-bezier(0.4, 0, 0.2, 1);
}

.button:hover {
    background-color: darkblue;
    transform: scale(1.05);
}
```

## Spring Physics (C API)

While standard CSS bezier curves are great for simple A-to-B transitions, interactive elements (like dragging a panel that "snaps" back into place, or a bouncing scroll view) require realistic physics.

The C API allows you to drive numeric properties using a damped harmonic oscillator (Spring).

```c
#include "ui_css_motion.h"

// Triggered when the user lets go of a dragged element
void on_drag_release(ui_component_t* panel) {
    // 1. Define the spring characteristics
    ui_spring_config_t spring = {
        .stiffness = 200.0f,
        .damping = 20.0f,
        .mass = 1.0f
    };

    // 2. Animate the X translation back to 0.0 using the spring
    // The engine automatically calculates the velocity from the user's drag
    // to ensure a smooth handoff from the finger to the physics engine.
    ui_motion_animate_spring(
        panel,
        UI_CSS_PROP_TRANSFORM_TRANSLATE_X,
        0.0f,   // Target value
        &spring // Physics config
    );
}
```

## Motion Paths

You can also animate elements along a predefined vector path rather than simply interpolating between X/Y coordinates.

```css
.flying-bird {
    /* Define an SVG path string */
    offset-path: path("M 10 80 C 40 10, 65 10, 95 80 S 150 150, 180 80");
    animation: fly 2s linear infinite;
}

@keyframes fly {
    from { offset-distance: 0%; }
    to { offset-distance: 100%; }
}
```
