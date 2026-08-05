/* clang-format off */
#include "ui_speed_dial_base.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

ui_error_t ui_speed_dial_base_init(struct ui_speed_dial_base *speed_dial,
                                   struct ui_component *component) {
  if (!speed_dial || !component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  speed_dial->component = component;
  speed_dial->is_open = 0;
  speed_dial->actions = NULL;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_speed_dial_base_add_action(struct ui_speed_dial_base *speed_dial,
                                         int id, struct ui_fab_base *fab) {
  struct ui_speed_dial_action *action;

  if (!speed_dial || !fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  action =
      (struct ui_speed_dial_action *)C_MULTIPLATFORM_MALLOC(sizeof(*action));
  if (!action) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  action->id = id;
  action->fab = fab;
  action->next = speed_dial->actions;
  speed_dial->actions = action;

  return UI_ERROR_NONE;
}

ui_error_t ui_speed_dial_base_toggle(struct ui_speed_dial_base *speed_dial) {
  if (!speed_dial)
    return UI_ERROR_INVALID_ARGUMENT;
  if (speed_dial) {
    speed_dial->is_open = !speed_dial->is_open;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_speed_dial_base_cleanup(struct ui_speed_dial_base *speed_dial) {
  struct ui_speed_dial_action *curr;
  struct ui_speed_dial_action *next_action;

  if (!speed_dial) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = speed_dial->actions;
  while (curr) {
    next_action = curr->next;
    C_MULTIPLATFORM_FREE(curr);
    curr = next_action;
  }

  speed_dial->actions = NULL;
  speed_dial->is_open = 0;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_speed_dial_base_bind_disabled(struct ui_speed_dial_base *widget,
                                            struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

ui_error_t ui_speed_dial_base_bind_text(struct ui_speed_dial_base *widget,
                                        struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}
