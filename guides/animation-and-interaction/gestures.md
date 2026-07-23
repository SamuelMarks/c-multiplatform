# Gestures & Multi-touch

Handling raw pointer events (mouse down, touch move, mouse up) can be tedious when implementing complex interactions. The `ui_gesture.h` module provides higher-level Gesture Recognizers that distill raw event streams into semantic actions like Pans, Pinches, and Swipes.

## The Gesture Recognizer Pattern

A gesture recognizer is attached to a UI component. It listens to the pointer events hitting that component, analyzes the motion (and the number of active touch points), and fires a callback when a specific pattern is detected.

## Implementing a Pinch-to-Zoom

Pinch-to-zoom requires at least two simultaneous touch points. The recognizer handles the complex math of calculating the centroid and scale delta.

```c
#include "ui_gesture.h"

void on_pinch_gesture(ui_gesture_recognizer_t* recognizer, ui_gesture_event_t* event) {
    ui_component_t* target = recognizer->target;

    if (event->state == UI_GESTURE_STATE_BEGAN || event->state == UI_GESTURE_STATE_CHANGED) {
        // scale_delta represents the relative change since the last frame
        float current_scale = ui_css_get_transform_scale_x(target);
        float new_scale = current_scale * event->data.pinch.scale_delta;

        // Clamp scale between 0.5x and 3.0x
        new_scale = fmax(0.5f, fmin(new_scale, 3.0f));

        ui_css_set_transform_scale(target, new_scale, new_scale);
    }
}

// Attach the recognizer to an image component
ui_component_t* my_image = ui_image_create(arena);
ui_gesture_recognizer_t* pinch_rec = ui_gesture_pinch_create(arena, on_pinch_gesture);
ui_component_add_gesture(my_image, pinch_rec);
```

## Available Recognizers

The framework provides several built-in recognizers:
*   `ui_gesture_tap_create`: Detects quick taps/clicks (configurable for double-taps).
*   `ui_gesture_long_press_create`: Fires after the pointer is held stationary for a specific duration.
*   `ui_gesture_pan_create`: Tracks X/Y drag movement. Useful for scrolling or dragging elements.
*   `ui_gesture_swipe_create`: Detects fast, directional flings.
*   `ui_gesture_pinch_create`: Tracks the distance between two touch points for scaling.
*   `ui_gesture_rotate_create`: Tracks the orbital rotation of two touch points.

## Gesture Resolution (Simultaneous Gestures)

By default, if a component has multiple recognizers (e.g., Pan and Pinch), only one can "win" and claim the touch sequence to prevent conflicting interactions.

If you want a component to be pannable *and* pinchable at the exact same time, you configure the recognizers to allow simultaneous recognition:

```c
ui_gesture_set_simultaneous(pan_rec, true);
ui_gesture_set_simultaneous(pinch_rec, true);
```
