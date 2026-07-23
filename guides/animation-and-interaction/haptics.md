# Haptics Engine

Providing physical, tactile feedback drastically improves the perceived quality and responsiveness of an application. The `ui_haptics.h` module abstracts platform-specific vibration APIs (like iOS CoreHaptics, Android Vibrator, or Gamepad rumble) into semantic feedback patterns.

## Semantic Feedback

Rather than specifying raw vibration durations (which feel inconsistent across different hardware), the framework encourages semantic triggers. The underlying engine translates these semantics into the most appropriate motor waveform for the device.

```c
#include "ui_haptics.h"

void on_switch_toggled(ui_component_t* toggle, bool is_on) {
    // A light, crisp click when toggling a setting
    ui_haptics_trigger(toggle->engine, UI_HAPTIC_SELECTION_CHANGE);
}

void on_payment_failed() {
    // A heavier, multi-pulse pattern to indicate an error
    ui_haptics_trigger(engine, UI_HAPTIC_NOTIFICATION_ERROR);
}

void on_payment_success() {
    // Two distinct, ascending pulses
    ui_haptics_trigger(engine, UI_HAPTIC_NOTIFICATION_SUCCESS);
}
```

## Continuous Physics & Gestures

Haptics are most powerful when tied directly to the physics of user interaction (e.g., pulling down to refresh, or dragging an item into a drop zone).

```c
void on_scroll_overshoot(ui_component_t* scroll_view, float overshoot_distance) {
    // Create a continuous rumbling effect that increases in intensity
    // the further the user pulls past the scroll boundary.
    float intensity = fmin(overshoot_distance / 100.0f, 1.0f);

    // Play a raw, continuous pattern (intensity from 0.0 to 1.0)
    ui_haptics_play_continuous(scroll_view->engine, intensity);
}

void on_scroll_release(ui_component_t* scroll_view) {
    // Stop the continuous rumble and give a final "snap" bump
    ui_haptics_stop_continuous(scroll_view->engine);
    ui_haptics_trigger(scroll_view->engine, UI_HAPTIC_IMPACT_MEDIUM);
}
```

## Platform Fallbacks

If the host platform does not have haptic hardware (e.g., most standard desktop Web browsers without a connected gamepad), calls to `ui_haptics_trigger` are safely ignored with zero performance overhead. You do not need to wrap them in feature-detection checks.
