/* clang-format off */
#include "ui_split_pane_base.h"
#include "ui_internal_mem.h"
#include "ui_drag_drop.h"
#include <stdlib.h>
/* clang-format on */

/**
 * \file ui_split_pane_base.c
 * \brief Split pane base component implementation.
 */

/**
 * \brief ui_split_pane_base structure.
 * \details Internal state for the split pane base component.
 */
struct ui_split_pane_base {
  enum ui_split_pane_orientation orientation;
  int position;
  int min_position;
  int max_position;

  int is_dragging;
  int drag_start_pos;
  int drag_start_coord;
  struct ui_signal *data_signal;
};

/**
 * \brief Creates a new split pane base component.
 * \param out_split_pane Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_pane_base_create(struct ui_split_pane_base **out_split_pane) {
  struct ui_split_pane_base *pane;
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_split_pane) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  pane = (struct ui_split_pane_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_split_pane_base));
  if (!pane) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  pane->orientation = UI_SPLIT_PANE_ORIENTATION_HORIZONTAL;
  pane->position = 50;
  pane->min_position = 0;
  pane->max_position = 10000;

  pane->is_dragging = 0;
  pane->drag_start_pos = 0;
  pane->drag_start_coord = 0;

  *out_split_pane = pane;

cleanup:
  return rc;
}

/**
 * \brief Destroys a split pane base component.
 * \param split_pane The component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_split_pane_base_destroy(struct ui_split_pane_base *split_pane) {
  if (!split_pane) {
    return UI_ERROR_NONE;
  }
  C_MULTIPLATFORM_FREE(split_pane);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the orientation of the split pane.
 * \param split_pane The split pane component.
 * \param orientation The orientation to set.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_pane_base_set_orientation(struct ui_split_pane_base *split_pane,
                                   enum ui_split_pane_orientation orientation) {
  if (!split_pane) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  split_pane->orientation = orientation;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current orientation.
 * \param split_pane The split pane component.
 * \param out_orientation Pointer to store the orientation.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_split_pane_base_get_orientation(
    const struct ui_split_pane_base *split_pane,
    enum ui_split_pane_orientation *out_orientation) {
  if (!split_pane || !out_orientation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_orientation = split_pane->orientation;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the position of the divider.
 * \param split_pane The split pane component.
 * \param position The position to set.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_pane_base_set_position(struct ui_split_pane_base *split_pane,
                                int position) {
  if (!split_pane) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (position < split_pane->min_position) {
    position = split_pane->min_position;
  }
  if (position > split_pane->max_position) {
    position = split_pane->max_position;
  }

  split_pane->position = position;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current position of the divider.
 * \param split_pane The split pane component.
 * \param out_position Pointer to store the position.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_pane_base_get_position(const struct ui_split_pane_base *split_pane,
                                int *out_position) {
  if (!split_pane || !out_position) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_position = split_pane->position;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the allowed bounds for the divider.
 * \param split_pane The split pane component.
 * \param min_position The minimum position.
 * \param max_position The maximum position.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_split_pane_base_set_bounds(struct ui_split_pane_base *split_pane,
                                         int min_position, int max_position) {
  if (!split_pane) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (min_position > max_position) {
    int temp = min_position;
    min_position = max_position;
    max_position = temp;
  }

  split_pane->min_position = min_position;
  split_pane->max_position = max_position;

  /* Re-clamp current position */
  return ui_split_pane_base_set_position(split_pane, split_pane->position);
}

/**
 * \brief Processes an event.
 * \param split_pane The split pane component.
 * \param event The event.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_pane_base_process_event(struct ui_split_pane_base *split_pane,
                                 const struct ui_event *event) {
  int coord = 0;
  int delta = 0;
  int new_pos;

  if (!split_pane || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (split_pane->orientation == UI_SPLIT_PANE_ORIENTATION_HORIZONTAL) {
    if (event->type == UI_EVENT_TOUCH_START ||
        event->type == UI_EVENT_TOUCH_MOVE ||
        event->type == UI_EVENT_TOUCH_END) {
      if (event->event_data.touch.num_points > 0) {
        coord = event->event_data.touch.points[0].x;
      }
    } else {
      coord = event->event_data.mouse.x;
    }
  } else {
    if (event->type == UI_EVENT_TOUCH_START ||
        event->type == UI_EVENT_TOUCH_MOVE ||
        event->type == UI_EVENT_TOUCH_END) {
      if (event->event_data.touch.num_points > 0) {
        coord = event->event_data.touch.points[0].y;
      }
    } else {
      coord = event->event_data.mouse.y;
    }
  }

  switch (event->type) {
  case UI_EVENT_MOUSE_DOWN:
  case UI_EVENT_TOUCH_START:
    if (event->type == UI_EVENT_TOUCH_START &&
        event->event_data.touch.num_points == 0) {
      break;
    }
    /* Assume the user clicked exactly on the resizer if this event is routed
     * here */
    split_pane->is_dragging = 1;
    split_pane->drag_start_coord = coord;
    split_pane->drag_start_pos = split_pane->position;
    break;

  case UI_EVENT_MOUSE_MOVE:
  case UI_EVENT_TOUCH_MOVE:
    if (event->type == UI_EVENT_TOUCH_MOVE &&
        event->event_data.touch.num_points == 0) {
      break;
    }
    if (split_pane->is_dragging) {
      delta = coord - split_pane->drag_start_coord;
      new_pos = split_pane->drag_start_pos + delta;
      (void)ui_split_pane_base_set_position(split_pane, new_pos);
    }
    break;

  case UI_EVENT_MOUSE_UP:
  case UI_EVENT_TOUCH_END:
    split_pane->is_dragging = 0;
    break;

  default:
    break;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Binds the data state to a signal.
 * \param widget The split pane component.
 * \param signal The signal to bind.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_split_pane_base_bind_data(struct ui_split_pane_base *widget,
                                        struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
