/**
 * @file cupertino_gesture.c
 * @brief Implementation of Cupertino edge swipe back gesture and
 * accessibility two-finger "Z" scrub gesture.
 */

/* clang-format off */
#include "cupertino/cupertino_gesture.h"
#include <math.h>
#include <string.h>
/* clang-format on */

ui_error_t cupertino_pop_gesture_init(struct cupertino_pop_gesture *gesture,
                                      float screen_width,
                                      enum ui_bidi_direction direction) {
  if (!gesture || screen_width <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (direction != UI_BIDI_DIR_LTR && direction != UI_BIDI_DIR_RTL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(gesture, 0, sizeof(*gesture));
  gesture->state = CUPERTINO_POP_GESTURE_INACTIVE;
  gesture->direction = direction;
  gesture->screen_width = screen_width;
  gesture->edge_slop = 20.0f; /* 20pt active edge margin */
  gesture->progress = 0.0f;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_pop_gesture_touch_down(struct cupertino_pop_gesture *gesture,
                                 float touch_x, int *out_began) {
  int in_edge = 0;

  if (!gesture || !out_began) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (gesture->direction == UI_BIDI_DIR_LTR) {
    /* Left edge in LTR */
    if (touch_x >= 0.0f && touch_x <= gesture->edge_slop) {
      in_edge = 1;
    }
  } else {
    /* Right edge in RTL */
    if (touch_x >= (gesture->screen_width - gesture->edge_slop) &&
        touch_x <= gesture->screen_width) {
      in_edge = 1;
    }
  }

  if (in_edge) {
    gesture->state = CUPERTINO_POP_GESTURE_TRACKING;
    gesture->start_x = touch_x;
    gesture->current_x = touch_x;
    gesture->translation_x = 0.0f;
    gesture->progress = 0.0f;
    *out_began = 1;
  } else {
    gesture->state = CUPERTINO_POP_GESTURE_INACTIVE;
    *out_began = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_pop_gesture_touch_move(struct cupertino_pop_gesture *gesture,
                                 float current_x, float *out_progress) {
  float delta;
  float prog;

  if (!gesture || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (gesture->state != CUPERTINO_POP_GESTURE_TRACKING) {
    *out_progress = 0.0f;
    return UI_ERROR_NONE;
  }

  gesture->current_x = current_x;

  if (gesture->direction == UI_BIDI_DIR_LTR) {
    delta = current_x - gesture->start_x;
    if (delta < 0.0f) {
      delta = 0.0f;
    }
  } else {
    delta = gesture->start_x - current_x;
    if (delta < 0.0f) {
      delta = 0.0f;
    }
  }

  gesture->translation_x = delta;
  prog = delta / gesture->screen_width;
  if (prog > 1.0f) {
    prog = 1.0f;
  }
  gesture->progress = prog;
  *out_progress = prog;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_pop_gesture_touch_up(struct cupertino_pop_gesture *gesture,
                                          float velocity_x,
                                          int *out_should_pop) {
  int should_pop = 0;

  if (!gesture || !out_should_pop) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (gesture->state != CUPERTINO_POP_GESTURE_TRACKING) {
    *out_should_pop = 0;
    return UI_ERROR_NONE;
  }

  gesture->velocity_x = velocity_x;

  /*
   * Apple HIG Pop release decision:
   * Pop if:
   *  1. Displacement progress >= 0.5 (50% across screen)
   *  2. OR velocity >= 500 pt/s in pop direction
   */
  if (gesture->direction == UI_BIDI_DIR_LTR) {
    if (gesture->progress >= 0.5f || velocity_x >= 500.0f) {
      should_pop = 1;
    }
  } else {
    if (gesture->progress >= 0.5f || velocity_x <= -500.0f) {
      should_pop = 1;
    }
  }

  if (should_pop) {
    gesture->state = CUPERTINO_POP_GESTURE_COMPLETED;
  } else {
    gesture->state = CUPERTINO_POP_GESTURE_CANCELLED;
  }

  *out_should_pop = should_pop;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_pop_gesture_get_underlying_offset(
    const struct cupertino_pop_gesture *gesture,
    float *out_underlying_offset_x) {
  float base_offset;
  float offset;

  if (!gesture || !out_underlying_offset_x) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Underlying view starts at -33% offset in LTR (+33% in RTL) */
  base_offset = 0.33f * gesture->screen_width;

  if (gesture->direction == UI_BIDI_DIR_LTR) {
    /* Starts at -base_offset and advances toward 0 */
    offset = -base_offset * (1.0f - gesture->progress);
  } else {
    /* Starts at +base_offset and advances toward 0 */
    offset = base_offset * (1.0f - gesture->progress);
  }

  *out_underlying_offset_x = offset;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_scrub_gesture_init(struct cupertino_scrub_gesture *scrub) {
  if (!scrub) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(scrub, 0, sizeof(*scrub));
  scrub->state = CUPERTINO_SCRUB_IDLE;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_scrub_gesture_touch_down(struct cupertino_scrub_gesture *scrub,
                                   int pointer_count, float x, float y) {
  if (!scrub || pointer_count < 1) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scrub->pointer_count = pointer_count;
  scrub->start_x = x;
  scrub->start_y = y;
  scrub->last_x = x;
  scrub->last_y = y;

  if (pointer_count == 2) {
    scrub->state = CUPERTINO_SCRUB_SEGMENT_1;
  } else {
    scrub->state = CUPERTINO_SCRUB_FAILED;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_scrub_gesture_touch_move(struct cupertino_scrub_gesture *scrub,
                                   float x, float y, int *out_recognized) {
  if (!scrub || !out_recognized) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /*
   * Two-finger "Z" shape:
   * Segment 1: Rightwards horizontal swipe (dx > 20, |dy| < 15)
   * Segment 2: Diagonal down-left swipe (dx < -15, dy > 15)
   * Segment 3: Rightwards horizontal swipe (dx > 20, |dy| < 15) -> RECOGNIZED!
   */
  switch (scrub->state) {
  case CUPERTINO_SCRUB_SEGMENT_1:
    if (x - scrub->start_x > 30.0f && (float)fabs(y - scrub->start_y) < 20.0f) {
      scrub->state = CUPERTINO_SCRUB_SEGMENT_2;
      scrub->last_x = x;
      scrub->last_y = y;
    }
    break;

  case CUPERTINO_SCRUB_SEGMENT_2:
    if ((x - scrub->last_x < -25.0f) && (y - scrub->last_y > 25.0f)) {
      /* Reached bottom-left vertex of Z */
      scrub->last_x = x;
      scrub->last_y = y;
      scrub->state = CUPERTINO_SCRUB_RECOGNIZED;
      *out_recognized = 1;
    }
    break;

  case CUPERTINO_SCRUB_RECOGNIZED:
    *out_recognized = 1;
    break;

  case CUPERTINO_SCRUB_IDLE:
  case CUPERTINO_SCRUB_FAILED:
  default:
    *out_recognized = 0;
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_scrub_gesture_reset(struct cupertino_scrub_gesture *scrub) {
  if (!scrub) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scrub->state = CUPERTINO_SCRUB_IDLE;
  scrub->pointer_count = 0;
  scrub->start_x = 0.0f;
  scrub->start_y = 0.0f;
  scrub->last_x = 0.0f;
  scrub->last_y = 0.0f;

  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_gesture_mock_touch_down_fail = 0;
int g_cupertino_gesture_mock_touch_move_fail = 0;
int g_cupertino_gesture_mock_touch_up_fail = 0;
int g_cupertino_gesture_mock_scrub_down_fail = 0;
int g_cupertino_gesture_mock_scrub_move_fail = 0;
int g_cupertino_gesture_mock_scrub_reset_fail = 0;

static ui_error_t
mock_cupertino_pop_gesture_touch_down(struct cupertino_pop_gesture *gesture,
                                      float touch_x, int *out_began) {
  if (g_cupertino_gesture_mock_touch_down_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return (cupertino_pop_gesture_touch_down)(gesture, touch_x, out_began);
}
#undef cupertino_pop_gesture_touch_down
/** @cond */
#define cupertino_pop_gesture_touch_down mock_cupertino_pop_gesture_touch_down
/** @endcond */

static ui_error_t
mock_cupertino_pop_gesture_touch_move(struct cupertino_pop_gesture *gesture,
                                      float current_x, float *out_progress) {
  if (g_cupertino_gesture_mock_touch_move_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return (cupertino_pop_gesture_touch_move)(gesture, current_x, out_progress);
}
#undef cupertino_pop_gesture_touch_move
/** @cond */
#define cupertino_pop_gesture_touch_move mock_cupertino_pop_gesture_touch_move
/** @endcond */

static ui_error_t
mock_cupertino_pop_gesture_touch_up(struct cupertino_pop_gesture *gesture,
                                    float velocity_x, int *out_should_pop) {
  if (g_cupertino_gesture_mock_touch_up_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return (cupertino_pop_gesture_touch_up)(gesture, velocity_x, out_should_pop);
}
#undef cupertino_pop_gesture_touch_up
/** @cond */
#define cupertino_pop_gesture_touch_up mock_cupertino_pop_gesture_touch_up
/** @endcond */

static ui_error_t
mock_cupertino_scrub_gesture_touch_down(struct cupertino_scrub_gesture *scrub,
                                        int pointer_count, float x, float y) {
  if (g_cupertino_gesture_mock_scrub_down_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return (cupertino_scrub_gesture_touch_down)(scrub, pointer_count, x, y);
}
#undef cupertino_scrub_gesture_touch_down
/** @cond */
#define cupertino_scrub_gesture_touch_down                                     \
  mock_cupertino_scrub_gesture_touch_down
/** @endcond */

static ui_error_t
mock_cupertino_scrub_gesture_touch_move(struct cupertino_scrub_gesture *scrub,
                                        float x, float y, int *out_recognized) {
  if (g_cupertino_gesture_mock_scrub_move_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return (cupertino_scrub_gesture_touch_move)(scrub, x, y, out_recognized);
}
#undef cupertino_scrub_gesture_touch_move
/** @cond */
#define cupertino_scrub_gesture_touch_move                                     \
  mock_cupertino_scrub_gesture_touch_move
/** @endcond */

static ui_error_t
mock_cupertino_scrub_gesture_reset(struct cupertino_scrub_gesture *scrub) {
  if (g_cupertino_gesture_mock_scrub_reset_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return (cupertino_scrub_gesture_reset)(scrub);
}
#undef cupertino_scrub_gesture_reset
/** @cond */
#define cupertino_scrub_gesture_reset mock_cupertino_scrub_gesture_reset
/** @endcond */

#endif /* UI_TEST_MOCK_ALLOC */

ui_error_t
cupertino_gesture_forward_event(struct cupertino_pop_gesture *pop_gesture,
                                struct cupertino_scrub_gesture *scrub_gesture,
                                const struct ui_event *event,
                                int *out_handled) {
  float x;
  float y;
  int began;
  float prog;
  int completed;
  int recognized;
  ui_error_t rc;

  if (!event || !out_handled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_handled = 0;

  switch (event->type) {
  case UI_EVENT_MOUSE_DOWN:
    x = (float)event->event_data.mouse.x;
    y = (float)event->event_data.mouse.y;
    if (pop_gesture) {
      rc = cupertino_pop_gesture_touch_down(pop_gesture, x, &began);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      if (began) {
        *out_handled = 1;
      }
    }
    if (scrub_gesture) {
      rc = cupertino_scrub_gesture_touch_down(scrub_gesture, 1, x, y);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    break;

  case UI_EVENT_MOUSE_MOVE:
    x = (float)event->event_data.mouse.x;
    y = (float)event->event_data.mouse.y;
    if (pop_gesture && pop_gesture->state == CUPERTINO_POP_GESTURE_TRACKING) {
      rc = cupertino_pop_gesture_touch_move(pop_gesture, x, &prog);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      *out_handled = 1;
    }
    if (scrub_gesture && scrub_gesture->state != CUPERTINO_SCRUB_IDLE) {
      rc = cupertino_scrub_gesture_touch_move(scrub_gesture, x, y, &recognized);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      if (recognized) {
        *out_handled = 1;
      }
    }
    break;

  case UI_EVENT_MOUSE_UP:
    if (pop_gesture && pop_gesture->state == CUPERTINO_POP_GESTURE_TRACKING) {
      rc = cupertino_pop_gesture_touch_up(pop_gesture, 0.0f, &completed);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      *out_handled = 1;
    }
    if (scrub_gesture) {
      rc = cupertino_scrub_gesture_reset(scrub_gesture);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    break;

  case UI_EVENT_TOUCH_START:
    if (event->event_data.touch.num_points > 0) {
      x = (float)event->event_data.touch.points[0].x;
      y = (float)event->event_data.touch.points[0].y;
      if (pop_gesture) {
        rc = cupertino_pop_gesture_touch_down(pop_gesture, x, &began);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
        if (began) {
          *out_handled = 1;
        }
      }
      if (scrub_gesture) {
        rc = cupertino_scrub_gesture_touch_down(
            scrub_gesture, event->event_data.touch.num_points, x, y);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
    }
    break;

  case UI_EVENT_TOUCH_MOVE:
    if (event->event_data.touch.num_points > 0) {
      x = (float)event->event_data.touch.points[0].x;
      y = (float)event->event_data.touch.points[0].y;
      if (pop_gesture && pop_gesture->state == CUPERTINO_POP_GESTURE_TRACKING) {
        rc = cupertino_pop_gesture_touch_move(pop_gesture, x, &prog);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
        *out_handled = 1;
      }
      if (scrub_gesture && scrub_gesture->state != CUPERTINO_SCRUB_IDLE) {
        rc = cupertino_scrub_gesture_touch_move(scrub_gesture, x, y,
                                                &recognized);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
        if (recognized) {
          *out_handled = 1;
        }
      }
    }
    break;

  case UI_EVENT_TOUCH_END:
  case UI_EVENT_TOUCH_CANCEL:
    if (pop_gesture && pop_gesture->state == CUPERTINO_POP_GESTURE_TRACKING) {
      rc = cupertino_pop_gesture_touch_up(pop_gesture, 0.0f, &completed);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      *out_handled = 1;
    }
    if (scrub_gesture) {
      rc = cupertino_scrub_gesture_reset(scrub_gesture);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    break;

  default:
    break;
  }

  return UI_ERROR_NONE;
}
