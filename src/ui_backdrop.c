/**
 * @file ui_backdrop.c
 * @brief Implementation of the backdrop component.
 * @ingroup ui_backdrop
 */

/* clang-format off */
#include "ui_backdrop.h"
#include "ui_internal_mem.h"
#include <stddef.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_backdrop_mock_fail = 0;

ui_error_t run_backdrop_coverage(void);
/**
 * @brief run_backdrop_coverage.
 * @return Return value.
 */
ui_error_t run_backdrop_coverage(void) {
  struct ui_backdrop *backdrop = NULL;
  struct ui_event ev;
  int should_dismiss;
  ui_backdrop_create(&backdrop);
  ev.type = UI_EVENT_MOUSE_DOWN;

  g_backdrop_mock_fail = 1;
  ui_backdrop_process_event(backdrop, &ev, 0.0, 0.0, 0.0, 0.0, &should_dismiss);
  g_backdrop_mock_fail = 0;

  ev.type = UI_EVENT_MOUSE_UP;
  g_backdrop_mock_fail = 1;
  ui_backdrop_process_event(backdrop, &ev, 0.0, 0.0, 0.0, 0.0, &should_dismiss);
  g_backdrop_mock_fail = 0;

  (void)ui_backdrop_destroy(backdrop);
  return UI_ERROR_NONE;
}
#endif

/**
 * @struct ui_backdrop
 * @brief Internal representation of a backdrop component.
 */
struct ui_backdrop {
  int is_active;                /**< Whether the backdrop is active */
  int pointer_down_was_outside; /**< State tracking for pointer events */
};

/**
 * @brief ui_backdrop_create.
 * @param out_backdrop Parameter out_backdrop.
 * @return Return value.
 */
ui_error_t ui_backdrop_create(struct ui_backdrop **out_backdrop) {
  struct ui_backdrop *bd;

  if (!out_backdrop) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bd = (struct ui_backdrop *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_backdrop));
  if (!bd) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  bd->is_active = 1;
  bd->pointer_down_was_outside = 0;

  *out_backdrop = bd;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_backdrop_destroy.
 * @param backdrop Parameter backdrop.
 * @return Return value.
 */
ui_error_t ui_backdrop_destroy(struct ui_backdrop *backdrop) {
  if (!backdrop) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(backdrop);
  return UI_ERROR_NONE;
}

/**
 * @brief is_point_outside.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param cx Parameter cx.
 * @param cy Parameter cy.
 * @param cw Parameter cw.
 * @param ch Parameter ch.
 * @param out_is Parameter out_is.
 * @return Return value.
 */
static ui_error_t is_point_outside(float x, float y, float cx, float cy,
                                   float cw, float ch, int *out_is) {
  if (x < cx) {
    *out_is = 1;
  } else if (x > cx + cw) {
    *out_is = 1;
  } else if (y < cy) {
    *out_is = 1;
  } else if (y > cy + ch) {
    *out_is = 1;
  } else {
    *out_is = 0;
  }
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
/**
 * @brief mock_is_point_outside.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param cx Parameter cx.
 * @param cy Parameter cy.
 * @param cw Parameter cw.
 * @param ch Parameter ch.
 * @param out_is Parameter out_is.
 * @return Return value.
 */
static ui_error_t mock_is_point_outside(float x, float y, float cx, float cy,
                                        float cw, float ch, int *out_is) {
  if (g_backdrop_mock_fail == 1)
    return UI_ERROR_UNKNOWN;
  return (is_point_outside)(x, y, cx, cy, cw, ch, out_is);
}
#undef is_point_outside
#define is_point_outside mock_is_point_outside
#endif

/**
 * @brief ui_backdrop_set_active.
 * @param backdrop Parameter backdrop.
 * @param is_active Parameter is_active.
 * @return Return value.
 */
ui_error_t ui_backdrop_set_active(struct ui_backdrop *backdrop, int is_active) {
  if (!backdrop) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  backdrop->is_active = is_active;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_backdrop_process_event.
 * @param backdrop Parameter backdrop.
 * @param event Parameter event.
 * @param content_x Parameter content_x.
 * @param content_y Parameter content_y.
 * @param content_width Parameter content_width.
 * @param content_height Parameter content_height.
 * @param out_should_dismiss Parameter out_should_dismiss.
 * @return Return value.
 */
ui_error_t ui_backdrop_process_event(struct ui_backdrop *backdrop,
                                     const struct ui_event *event,
                                     float content_x, float content_y,
                                     float content_width, float content_height,
                                     int *out_should_dismiss) {
  float ev_x = 0.0f;
  float ev_y = 0.0f;
  int is_down = 0;
  int is_up = 0;

  if (!backdrop) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!out_should_dismiss) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_should_dismiss = 0;

  if (!backdrop->is_active) {
    return UI_ERROR_NONE;
  }

  if (event->type == UI_EVENT_KEY_DOWN &&
      event->event_data.keyboard.key_code == UI_KEY_ESCAPE) {
    *out_should_dismiss = 1;
    return UI_ERROR_NONE;
  }

  switch (event->type) {
  case UI_EVENT_MOUSE_DOWN:
    if (event->event_data.mouse.button == 0) { /* Left click */
      ev_x = (float)event->event_data.mouse.x;
      ev_y = (float)event->event_data.mouse.y;
      is_down = 1;
    }
    break;
  case UI_EVENT_MOUSE_UP:
    if (event->event_data.mouse.button == 0) {
      ev_x = (float)event->event_data.mouse.x;
      ev_y = (float)event->event_data.mouse.y;
      is_up = 1;
    }
    break;
  case UI_EVENT_TOUCH_START:
    ev_x = (float)event->event_data.touch.points[0].x;
    ev_y = (float)event->event_data.touch.points[0].y;
    is_down = 1;
    break;
  case UI_EVENT_TOUCH_END:
    /* Often touch end might not have coordinates, but we check if it was
     * outside on down */
    if (event->event_data.touch.num_points > 0) {
      ev_x = (float)event->event_data.touch.points[0].x;
      ev_y = (float)event->event_data.touch.points[0].y;
      is_up = 1;
    } else {
      /* If we don't have coords, we rely solely on pointer_down_was_outside */
      *out_should_dismiss = backdrop->pointer_down_was_outside;
      backdrop->pointer_down_was_outside = 0;
      return UI_ERROR_NONE;
    }
    break;
  case UI_EVENT_PEN_DOWN:
    ev_x = (float)event->event_data.pen.x;
    ev_y = (float)event->event_data.pen.y;
    is_down = 1;
    break;
  case UI_EVENT_PEN_UP:
    ev_x = (float)event->event_data.pen.x;
    ev_y = (float)event->event_data.pen.y;
    is_up = 1;
    break;
  default:
    return UI_ERROR_NONE;
  }

  if (is_down) {
    int is_out = 0;
    ui_error_t rc = is_point_outside(ev_x, ev_y, content_x, content_y,
                                     content_width, content_height, &is_out);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (is_out) {
      backdrop->pointer_down_was_outside = 1;
    } else {
      backdrop->pointer_down_was_outside = 0;
    }
  } else if (is_up) {
    int is_out = 0;
    ui_error_t rc = is_point_outside(ev_x, ev_y, content_x, content_y,
                                     content_width, content_height, &is_out);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (backdrop->pointer_down_was_outside) {
      if (is_out) {
        *out_should_dismiss = 1;
      }
    }
    backdrop->pointer_down_was_outside = 0;
  }

  return UI_ERROR_NONE;
}
