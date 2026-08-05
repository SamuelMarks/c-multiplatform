/* clang-format off */
#include "ui_swipe_action_base.h"
#include <math.h>
/* clang-format on */

ui_error_t ui_swipe_action_base_init(struct ui_swipe_action_base *swipe_action,
                                     struct ui_component *component) {
  if (!swipe_action || !component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  swipe_action->component = component;
  swipe_action->state = UI_SWIPE_ACTION_IDLE;
  swipe_action->offset_x = 0.0f;
  swipe_action->threshold = 60.0f; /* default swipe threshold */

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_swipe_action_base_update(struct ui_swipe_action_base *swipe_action,
                            float delta_x) {
  if (!swipe_action) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (swipe_action->state == UI_SWIPE_ACTION_IDLE ||
      swipe_action->state == UI_SWIPE_ACTION_REVEALED_LEFT ||
      swipe_action->state == UI_SWIPE_ACTION_REVEALED_RIGHT) {
    swipe_action->state = UI_SWIPE_ACTION_SWIPING;
  }

  swipe_action->offset_x += delta_x;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_swipe_action_base_commit(struct ui_swipe_action_base *swipe_action) {
  if (!swipe_action) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (swipe_action->offset_x > swipe_action->threshold) {
    swipe_action->state = UI_SWIPE_ACTION_REVEALED_LEFT;
  } else if (swipe_action->offset_x < -swipe_action->threshold) {
    swipe_action->state = UI_SWIPE_ACTION_REVEALED_RIGHT;
  } else {
    swipe_action->state = UI_SWIPE_ACTION_IDLE;
    swipe_action->offset_x = 0.0f;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_swipe_action_base_reset(struct ui_swipe_action_base *swipe_action) {
  if (!swipe_action)
    return UI_ERROR_INVALID_ARGUMENT;
  if (swipe_action) {
    swipe_action->state = UI_SWIPE_ACTION_IDLE;
    swipe_action->offset_x = 0.0f;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_swipe_action_base_bind_disabled(struct ui_swipe_action_base *widget,
                                   struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_swipe_action_base_bind_text(struct ui_swipe_action_base *widget,
                                          struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}
