/*
 * \file ui_gesture.c
 * \brief Implementation of pointer gesture recognition.
 */
/* clang-format off */
#include "ui_gesture.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

/*
 * \def UI_TAP_MAX_DURATION_MS
 * \brief Maximum duration for a tap gesture.
 */
#define UI_TAP_MAX_DURATION_MS 300.0
/*
 * \def UI_TAP_MAX_DISTANCE
 * \brief Maximum distance allowed for a tap gesture.
 */
#define UI_TAP_MAX_DISTANCE 10.0
/*
 * \def UI_LONG_PRESS_MIN_DURATION_MS
 * \brief Minimum duration for a long press gesture.
 */
#define UI_LONG_PRESS_MIN_DURATION_MS 500.0
/*
 * \def UI_PAN_MIN_DISTANCE
 * \brief Minimum distance required to trigger a pan gesture.
 */
#define UI_PAN_MIN_DISTANCE 10.0
/*
 * \def UI_SWIPE_MIN_VELOCITY
 * \brief Minimum velocity required to trigger a swipe gesture.
 */
#define UI_SWIPE_MIN_VELOCITY 500.0 /* pixels per second */

/**
 * @struct ui_gesture_recognizer
 * \struct ui_gesture_recognizer
 * \brief Tracks pointer state to recognize gestures.
 */
struct ui_gesture_recognizer {
  int is_tracking;      /**< is_tracking */
  int start_x;          /**< start_x */
  int start_y;          /**< start_y */
  double start_time_ms; /**< start_time_ms */
  int last_x;           /**< last_x */
  int last_y;           /**< last_y */
  double last_time_ms;  /**< last_time_ms */

  int has_moved_significantly; /**< has_moved_significantly */
  int long_press_triggered;    /**< long_press_triggered */

  /* Velocity tracking */
  float velocity_x; /**< velocity_x */
  float velocity_y; /**< velocity_y */
  /* Multi-touch state */
  float initial_distance; /**< initial_distance */
  float initial_angle;    /**< initial_angle */
  float last_scale;       /**< last_scale */
  float last_rotation;    /**< last_rotation */
};

/*
 * \brief Creates a new gesture recognizer.
 * \param[out] out_recognizer Pointer to store the created recognizer.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_gesture_recognizer_create(struct ui_gesture_recognizer **out_recognizer) {
  struct ui_gesture_recognizer *r;
  if (!out_recognizer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  r = (struct ui_gesture_recognizer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_gesture_recognizer));
  if (!r) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  r->is_tracking = 0;
  r->start_x = 0;
  r->start_y = 0;
  r->start_time_ms = 0.0;
  r->last_x = 0;
  r->last_y = 0;
  r->last_time_ms = 0.0;
  r->has_moved_significantly = 0;
  r->long_press_triggered = 0;
  r->velocity_x = 0.0f;
  r->velocity_y = 0.0f;
  r->initial_distance = 0.0f;
  r->initial_angle = 0.0f;
  r->last_scale = 1.0f;
  r->last_rotation = 0.0f;

  *out_recognizer = r;
  return UI_ERROR_NONE;
}

/*
 * \brief Destroys a gesture recognizer.
 * \param[in,out] recognizer The recognizer to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_gesture_recognizer_destroy(struct ui_gesture_recognizer *recognizer) {
  if (!recognizer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(recognizer);
  return UI_ERROR_NONE;
}

/*
 * \brief Resets the internal state of the gesture recognizer.
 * \param[in,out] r The recognizer to reset.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t ui_gesture_reset(struct ui_gesture_recognizer *r) {
  r->is_tracking = 0;
  r->has_moved_significantly = 0;
  r->long_press_triggered = 0;
  r->velocity_x = 0.0f;
  r->velocity_y = 0.0f;
  r->initial_distance = 0.0f;
  r->initial_angle = 0.0f;
  r->last_scale = 1.0f;
  r->last_rotation = 0.0f;
  return UI_ERROR_NONE;
}

/*
 * \brief Processes a pointer event and updates gesture state.
 * \param[in,out] r The gesture recognizer.
 * \param[in] event The pointer event to process.
 * \param[in] timestamp_ms The timestamp of the event.
 * \param[out] out_gesture_event Pointer to store any recognized gesture event.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_gesture_recognizer_process_event(
    struct ui_gesture_recognizer *r, const struct ui_event *event,
    double timestamp_ms, struct ui_gesture_event *out_gesture_event) {
  int current_x = 0;
  int current_y = 0;
  int is_down = 0;
  int is_move = 0;
  int is_up = 0;
  int is_cancel = 0;
  float dx, dy, dist_sq;
  double dt;
  double duration;
  float speed_sq;
  float dx2, dy2, current_distance, current_angle;

  if (!r || !event || !out_gesture_event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(out_gesture_event, 0, sizeof(*out_gesture_event));
  out_gesture_event->type = UI_GESTURE_NONE;
  out_gesture_event->state = UI_GESTURE_STATE_POSSIBLE;

  /* Map inputs to generic pointer logic */
  switch (event->type) {
  case UI_EVENT_MOUSE_DOWN:
    if (event->event_data.mouse.button == 0) { /* Left click */
      current_x = event->event_data.mouse.x;
      current_y = event->event_data.mouse.y;
      is_down = 1;
    }
    break;
  case UI_EVENT_MOUSE_MOVE:
    current_x = event->event_data.mouse.x;
    current_y = event->event_data.mouse.y;
    is_move = 1;
    break;
  case UI_EVENT_MOUSE_UP:
    if (event->event_data.mouse.button == 0) {
      current_x = event->event_data.mouse.x;
      current_y = event->event_data.mouse.y;
      is_up = 1;
    }
    break;
  case UI_EVENT_TOUCH_START:
    if (event->event_data.touch.num_points > 0) {
      current_x = event->event_data.touch.points[0].x;
      current_y = event->event_data.touch.points[0].y;
      is_down = 1; /* For single touch point tracking */
    }
    break;
  case UI_EVENT_TOUCH_MOVE:
    if (event->event_data.touch.num_points > 0) {
      current_x = event->event_data.touch.points[0].x;
      current_y = event->event_data.touch.points[0].y;
      is_move = 1;
    }
    break;
  case UI_EVENT_TOUCH_END:
    is_up = 1;
    if (event->event_data.touch.num_points > 0) {
      current_x = event->event_data.touch.points[0].x;
      current_y = event->event_data.touch.points[0].y;
    } else {
      current_x = r->last_x;
      current_y = r->last_y;
    }
    break;
  case UI_EVENT_TOUCH_CANCEL:
    is_cancel = 1;
    break;
  default:
    return UI_ERROR_NONE; /* Not a pointer event */
  }

  if (is_down) {
    r->is_tracking = 1;
    r->start_x = current_x;
    r->start_y = current_y;
    r->start_time_ms = timestamp_ms;
    r->last_x = current_x;
    r->last_y = current_y;
    r->last_time_ms = timestamp_ms;
    r->has_moved_significantly = 0;
    r->long_press_triggered = 0;
    r->velocity_x = 0.0f;
    r->velocity_y = 0.0f;
  } else if (event->type == UI_EVENT_TOUCH_MOVE &&
             event->event_data.touch.num_points >= 2 && r->is_tracking) {
    dx2 = (float)(event->event_data.touch.points[1].x -
                  event->event_data.touch.points[0].x);
    dy2 = (float)(event->event_data.touch.points[1].y -
                  event->event_data.touch.points[0].y);
    current_distance = (float)sqrt(dx2 * dx2 + dy2 * dy2);
    current_angle = (float)atan2(dy2, dx2);
    if (r->initial_distance == 0.0f) {
      r->initial_distance = current_distance;
      r->initial_angle = current_angle;
      out_gesture_event->type = UI_GESTURE_PINCH;
      out_gesture_event->state = UI_GESTURE_STATE_BEGAN;
      out_gesture_event->scale = 1.0f;
      out_gesture_event->rotation = 0.0f;
      out_gesture_event->x = (event->event_data.touch.points[0].x +
                              event->event_data.touch.points[1].x) /
                             2;
      out_gesture_event->y = (event->event_data.touch.points[0].y +
                              event->event_data.touch.points[1].y) /
                             2;
    } else {
      float scale = current_distance / r->initial_distance;
      float rotation = current_angle - r->initial_angle;
      if (fabs(scale - r->last_scale) > 0.05f) {
        out_gesture_event->type = UI_GESTURE_PINCH;
        out_gesture_event->state = UI_GESTURE_STATE_CHANGED;
        out_gesture_event->scale = scale;
        out_gesture_event->rotation = rotation;
        out_gesture_event->x = (event->event_data.touch.points[0].x +
                                event->event_data.touch.points[1].x) /
                               2;
        out_gesture_event->y = (event->event_data.touch.points[0].y +
                                event->event_data.touch.points[1].y) /
                               2;
        r->last_scale = scale;
      } else if (fabs(rotation - r->last_rotation) > 0.05f) {
        out_gesture_event->type = UI_GESTURE_ROTATION;
        out_gesture_event->state = UI_GESTURE_STATE_CHANGED;
        out_gesture_event->scale = scale;
        out_gesture_event->rotation = rotation;
        out_gesture_event->x = (event->event_data.touch.points[0].x +
                                event->event_data.touch.points[1].x) /
                               2;
        out_gesture_event->y = (event->event_data.touch.points[0].y +
                                event->event_data.touch.points[1].y) /
                               2;
        r->last_rotation = rotation;
      }
    }
  } else if (is_move && r->is_tracking) {
    dx = (float)(current_x - r->start_x);
    dy = (float)(current_y - r->start_y);
    dist_sq = dx * dx + dy * dy;
    dt = (timestamp_ms - r->last_time_ms) / 1000.0;

    if (dt > 0.0) {
      r->velocity_x = (float)(current_x - r->last_x) / (float)dt;
      r->velocity_y = (float)(current_y - r->last_y) / (float)dt;
    }

    if (!r->has_moved_significantly &&
        dist_sq > UI_PAN_MIN_DISTANCE * UI_PAN_MIN_DISTANCE) {
      r->has_moved_significantly = 1;

      /* If long press hasn't fired, start pan */
      if (!r->long_press_triggered) {
        out_gesture_event->type = UI_GESTURE_PAN;
        out_gesture_event->state = UI_GESTURE_STATE_BEGAN;
        out_gesture_event->x = current_x;
        out_gesture_event->y = current_y;
        out_gesture_event->delta_x = 0.0f;
        out_gesture_event->delta_y = 0.0f;
        out_gesture_event->velocity_x = r->velocity_x;
        out_gesture_event->velocity_y = r->velocity_y;
      }
    } else if (r->has_moved_significantly && !r->long_press_triggered) {
      /* Continue pan */
      out_gesture_event->type = UI_GESTURE_PAN;
      out_gesture_event->state = UI_GESTURE_STATE_CHANGED;
      out_gesture_event->x = current_x;
      out_gesture_event->y = current_y;
      out_gesture_event->delta_x = (float)(current_x - r->last_x);
      out_gesture_event->delta_y = (float)(current_y - r->last_y);
      out_gesture_event->velocity_x = r->velocity_x;
      out_gesture_event->velocity_y = r->velocity_y;
    }

    r->last_x = current_x;
    r->last_y = current_y;
    r->last_time_ms = timestamp_ms;
  } else if (is_up && r->is_tracking) {
    duration = timestamp_ms - r->start_time_ms;
    speed_sq = r->velocity_x * r->velocity_x + r->velocity_y * r->velocity_y;

    if (!r->has_moved_significantly) {
      if (!r->long_press_triggered && duration <= UI_TAP_MAX_DURATION_MS) {
        /* Tap */
        out_gesture_event->type = UI_GESTURE_TAP;
        out_gesture_event->state = UI_GESTURE_STATE_ENDED;
        out_gesture_event->x = current_x;
        out_gesture_event->y = current_y;
      } else if (r->long_press_triggered) {
        /* End long press */
        out_gesture_event->type = UI_GESTURE_LONG_PRESS;
        out_gesture_event->state = UI_GESTURE_STATE_ENDED;
        out_gesture_event->x = current_x;
        out_gesture_event->y = current_y;
      }
    } else {
      if (!r->long_press_triggered) {
        if (speed_sq > UI_SWIPE_MIN_VELOCITY * UI_SWIPE_MIN_VELOCITY) {
          /* Swipe */
          out_gesture_event->type = UI_GESTURE_SWIPE;
          out_gesture_event->state = UI_GESTURE_STATE_ENDED;
          out_gesture_event->x = current_x;
          out_gesture_event->y = current_y;
          out_gesture_event->delta_x = (float)(current_x - r->last_x);
          out_gesture_event->delta_y = (float)(current_y - r->last_y);
          out_gesture_event->velocity_x = r->velocity_x;
          out_gesture_event->velocity_y = r->velocity_y;
        } else {
          /* End Pan */
          out_gesture_event->type = UI_GESTURE_PAN;
          out_gesture_event->state = UI_GESTURE_STATE_ENDED;
          out_gesture_event->x = current_x;
          out_gesture_event->y = current_y;
          out_gesture_event->delta_x = (float)(current_x - r->last_x);
          out_gesture_event->delta_y = (float)(current_y - r->last_y);
          out_gesture_event->velocity_x = r->velocity_x;
          out_gesture_event->velocity_y = r->velocity_y;
        }
      }
    }
    {
      ui_error_t reset_rc = ui_gesture_reset(r);
      (void)reset_rc;
    }
  } else if (is_cancel && r->is_tracking) {
    if (r->has_moved_significantly && !r->long_press_triggered) {
      out_gesture_event->type = UI_GESTURE_PAN;
      out_gesture_event->state = UI_GESTURE_STATE_CANCELLED;
      out_gesture_event->x = r->last_x;
      out_gesture_event->y = r->last_y;
    } else if (r->long_press_triggered) {
      out_gesture_event->type = UI_GESTURE_LONG_PRESS;
      out_gesture_event->state = UI_GESTURE_STATE_CANCELLED;
      out_gesture_event->x = r->last_x;
      out_gesture_event->y = r->last_y;
    }
    {
      ui_error_t reset_rc = ui_gesture_reset(r);
      (void)reset_rc;
    }
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Updates the gesture recognizer (e.g., checking timeouts for long
 * press).
 * \param[in,out] r The gesture recognizer.
 * \param[in] timestamp_ms The current timestamp.
 * \param[out] out_gesture_event Pointer to store any recognized gesture event.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_gesture_recognizer_update(struct ui_gesture_recognizer *r,
                             double timestamp_ms,
                             struct ui_gesture_event *out_gesture_event) {
  if (!r || !out_gesture_event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(out_gesture_event, 0, sizeof(*out_gesture_event));
  out_gesture_event->type = UI_GESTURE_NONE;
  out_gesture_event->state = UI_GESTURE_STATE_POSSIBLE;

  if (r->is_tracking && !r->has_moved_significantly &&
      !r->long_press_triggered) {
    if (timestamp_ms - r->start_time_ms >= UI_LONG_PRESS_MIN_DURATION_MS) {
      r->long_press_triggered = 1;
      out_gesture_event->type = UI_GESTURE_LONG_PRESS;
      out_gesture_event->state = UI_GESTURE_STATE_BEGAN;
      out_gesture_event->x = r->last_x;
      out_gesture_event->y = r->last_y;
    }
  }

  return UI_ERROR_NONE;
}
