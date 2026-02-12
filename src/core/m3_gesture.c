#include "m3/m3_gesture.h"

#include <string.h>

#ifdef M3_TESTING
static M3Bool g_m3_gesture_test_force_contains_error = M3_FALSE;
static M3Bool g_m3_gesture_test_force_reset_fail =
    M3_FALSE; /* GCOVR_EXCL_LINE */
static M3Bool g_m3_gesture_test_force_velocity_error = M3_FALSE;
static M3Bool g_m3_gesture_test_force_distance_error = M3_FALSE;
static M3Bool g_m3_gesture_test_force_config_init_fail = M3_FALSE;
static int g_m3_gesture_test_distance_error_after = 0;
#endif

static int m3_gesture_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_gesture_validate_config(const M3GestureConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->tap_max_distance < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (config->double_tap_max_distance < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (config->drag_start_distance < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (config->fling_min_velocity < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_gesture_distance_sq(M3Scalar x0, M3Scalar y0, M3Scalar x1,
                                  M3Scalar y1, M3Scalar *out_value) {
  M3Scalar dx;
  M3Scalar dy;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING /* GCOVR_EXCL_LINE */
  if (g_m3_gesture_test_force_distance_error == M3_TRUE) {
    g_m3_gesture_test_force_distance_error = M3_FALSE;
    return M3_ERR_UNKNOWN;
  }
  if (g_m3_gesture_test_distance_error_after > 0) {
    g_m3_gesture_test_distance_error_after -= 1;
    if (g_m3_gesture_test_distance_error_after == 0) {
      return M3_ERR_UNKNOWN;
    }
  }
#endif

  dx = x1 - x0;
  dy = y1 - y0;
  *out_value = dx * dx + dy * dy;
  return M3_OK;
}

static int m3_gesture_time_delta(m3_u32 start, m3_u32 end, m3_u32 *out_delta) {
  if (out_delta == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (end < start) {
    return M3_ERR_RANGE;
  }
  *out_delta = end - start;
  return M3_OK;
}

static int m3_gesture_velocity(M3Scalar x0, M3Scalar y0, M3Scalar x1,
                               M3Scalar y1, m3_u32 dt_ms, M3Scalar *out_vx,
                               M3Scalar *out_vy) {
  M3Scalar scale;

  if (out_vx == NULL || out_vy == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_m3_gesture_test_force_velocity_error == M3_TRUE) {
    g_m3_gesture_test_force_velocity_error = M3_FALSE;
    return M3_ERR_UNKNOWN;
  }
#endif

  if (dt_ms == 0u) {
    *out_vx = 0.0f;
    *out_vy = 0.0f;
    return M3_OK;
  }

  scale = 1000.0f / (M3Scalar)dt_ms;
  *out_vx = (x1 - x0) * scale;
  *out_vy = (y1 - y0) * scale;
  return M3_OK;
}

static int m3_gesture_hit_test(const M3RenderNode *node, M3Scalar x, M3Scalar y,
                               M3Widget **out_widget) {
  M3Bool contains;
  m3_usize i;
  int rc; /* GCOVR_EXCL_LINE */

  if (node == NULL || out_widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_widget = NULL;

  if (node->widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (node->widget->flags & M3_WIDGET_FLAG_HIDDEN) {
    return M3_OK;
  }
  if (node->widget->flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  rc = m3_gesture_validate_rect(&node->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  contains = M3_FALSE;
  rc = m3_rect_contains_point(&node->bounds, x, y, &contains);
#ifdef M3_TESTING
  if (g_m3_gesture_test_force_contains_error == M3_TRUE) {
    g_m3_gesture_test_force_contains_error = M3_FALSE;
    rc = M3_ERR_RANGE;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }
  if (contains == M3_FALSE) {
    return M3_OK;
  }

  if (node->child_count > 0 && node->children == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (i = node->child_count; i > 0; --i) {
    M3RenderNode *child = node->children[i - 1];

    if (child == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_gesture_hit_test(child, x, y, out_widget);
    if (rc != M3_OK) {
      return rc;
    }
    if (*out_widget != NULL) {
      return M3_OK;
    }
  }

  *out_widget = node->widget;
  return M3_OK;
}

static int m3_gesture_dispatch_to_widget(M3Widget *widget,
                                         const M3InputEvent *event,
                                         M3Bool *out_handled) {
  M3Bool handled;
  int rc; /* GCOVR_EXCL_LINE */

  if (out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;

  if (widget == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (widget->vtable == NULL || widget->vtable->event == NULL) {
    return M3_OK;
  }

  handled = M3_FALSE;
  rc = widget->vtable->event(widget->ctx, event, &handled);
  if (rc != M3_OK) {
    return rc;
  }

  *out_handled = handled;
  return M3_OK;
}

static int m3_gesture_emit(M3Widget *widget, const M3InputEvent *source,
                           m3_u32 type, const M3GestureEvent *gesture,
                           M3Bool *out_handled) {
  M3InputEvent event; /* GCOVR_EXCL_LINE */
  int rc;

  if (out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;

  if (widget == NULL) {
    return M3_OK;
  }
  if (source == NULL || gesture == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(&event, 0, sizeof(event));
  event.type = type;
  event.modifiers = source->modifiers;
  event.time_ms = source->time_ms;
  event.window = source->window;
  event.data.gesture = *gesture;

  rc = m3_gesture_dispatch_to_widget(widget, &event, out_handled);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_gesture_reset_pointer(M3GestureDispatcher *dispatcher) {
  if (dispatcher == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

#ifdef M3_TESTING
  if (g_m3_gesture_test_force_reset_fail == M3_TRUE) {
    g_m3_gesture_test_force_reset_fail = M3_FALSE;
    return M3_ERR_UNKNOWN;
  }
#endif

  dispatcher->active_widget = NULL;
  dispatcher->pointer_active = M3_FALSE;
  dispatcher->active_pointer = 0;
  dispatcher->drag_active = M3_FALSE;
  dispatcher->down_time = 0u;
  dispatcher->last_time = 0u;
  dispatcher->down_x = 0.0f;
  dispatcher->down_y = 0.0f;
  dispatcher->last_x = 0.0f;
  dispatcher->last_y = 0.0f;
  dispatcher->velocity_x = 0.0f;
  dispatcher->velocity_y = 0.0f;
  return M3_OK;
}

static int m3_gesture_clear_state(M3GestureDispatcher *dispatcher) {
  int rc;

  if (dispatcher == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_gesture_reset_pointer(dispatcher);
  if (rc != M3_OK) {
    return rc;
  }

  dispatcher->has_last_tap = M3_FALSE;
  dispatcher->last_tap_time = 0u;
  dispatcher->last_tap_x = 0.0f;
  dispatcher->last_tap_y = 0.0f;
  dispatcher->last_tap_widget = NULL;
  return M3_OK;
}

int M3_CALL m3_gesture_config_init(M3GestureConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_m3_gesture_test_force_config_init_fail == M3_TRUE) {
    g_m3_gesture_test_force_config_init_fail = M3_FALSE;
    return M3_ERR_UNKNOWN;
  }
#endif

  config->tap_max_ms = M3_GESTURE_DEFAULT_TAP_MAX_MS;
  config->tap_max_distance = M3_GESTURE_DEFAULT_TAP_MAX_DISTANCE;
  config->double_tap_max_ms = M3_GESTURE_DEFAULT_DOUBLE_TAP_MAX_MS;
  config->double_tap_max_distance = M3_GESTURE_DEFAULT_DOUBLE_TAP_MAX_DISTANCE;
  config->long_press_ms = M3_GESTURE_DEFAULT_LONG_PRESS_MS;
  config->drag_start_distance = M3_GESTURE_DEFAULT_DRAG_START_DISTANCE;
  config->fling_min_velocity = M3_GESTURE_DEFAULT_FLING_MIN_VELOCITY;

  return m3_gesture_validate_config(config);
}

int M3_CALL m3_gesture_dispatcher_init(M3GestureDispatcher *dispatcher,
                                       const M3GestureConfig *config) {
  M3GestureConfig defaults; /* GCOVR_EXCL_LINE */
  int rc;

  if (dispatcher == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  if (config == NULL) {
    rc = m3_gesture_config_init(&defaults);
    if (rc != M3_OK) {
      return rc;
    }
    dispatcher->config = defaults;
  } else {
    rc = m3_gesture_validate_config(config);
    if (rc != M3_OK) {
      return rc;
    }
    dispatcher->config = *config;
  }

  dispatcher->initialized = M3_TRUE;
  rc = m3_gesture_clear_state(dispatcher);
  if (rc != M3_OK) {
    dispatcher->initialized = M3_FALSE;
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_gesture_dispatcher_shutdown(M3GestureDispatcher *dispatcher) {
  int rc;

  if (dispatcher == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  rc = m3_gesture_clear_state(dispatcher);
  if (rc != M3_OK) {
    return rc;
  }

  dispatcher->initialized = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_gesture_dispatcher_reset(M3GestureDispatcher *dispatcher) {
  if (dispatcher == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  return m3_gesture_clear_state(dispatcher);
}

int M3_CALL m3_gesture_dispatcher_set_config(M3GestureDispatcher *dispatcher,
                                             const M3GestureConfig *config) {
  int rc;

  if (dispatcher == NULL || config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  rc = m3_gesture_validate_config(config);
  if (rc != M3_OK) {
    return rc;
  }

  dispatcher->config = *config;
  return M3_OK;
}

int M3_CALL m3_gesture_dispatcher_get_config(
    const M3GestureDispatcher *dispatcher, M3GestureConfig *out_config) {
  if (dispatcher == NULL || out_config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  *out_config = dispatcher->config;
  return M3_OK;
}

int M3_CALL m3_gesture_dispatch(M3GestureDispatcher *dispatcher,
                                const M3RenderNode *root,
                                const M3InputEvent *event,
                                M3Widget **out_target, M3Bool *out_handled) {
  M3Widget *target;
  M3Bool handled;
  M3Bool handled_any;
  M3Scalar x; /* GCOVR_EXCL_LINE */
  M3Scalar y;
  M3Scalar dist_sq;
  M3Scalar drag_threshold_sq; /* GCOVR_EXCL_LINE */
  M3Scalar tap_threshold_sq;
  M3Scalar double_tap_threshold_sq;
  M3Scalar delta_x;
  M3Scalar delta_y;
  M3Scalar total_x;
  M3Scalar total_y;
  M3Scalar velocity_x;
  M3Scalar velocity_y;
  M3Scalar speed_sq;
  M3Scalar fling_threshold_sq;
  m3_u32 dt;
  m3_u32 duration;
  int rc;

  if (dispatcher == NULL || root == NULL || event == NULL ||
      out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  *out_handled = M3_FALSE;
  if (out_target != NULL) {
    *out_target = NULL;
  }

  handled_any = M3_FALSE;
  target = NULL;

  switch (event->type) {
  case M3_INPUT_POINTER_DOWN:
    if (dispatcher->pointer_active) {
      if (event->data.pointer.pointer_id == dispatcher->active_pointer) {
        return M3_ERR_STATE;
      }
      return M3_ERR_BUSY;
    }
    x = (M3Scalar)event->data.pointer.x;
    y = (M3Scalar)event->data.pointer.y;
    rc = m3_gesture_hit_test(root, x, y, &target);
    if (rc != M3_OK) {
      return rc;
    }
    if (target == NULL) {
      return M3_OK;
    }

    dispatcher->active_widget = target;
    dispatcher->pointer_active = M3_TRUE;
    dispatcher->active_pointer = event->data.pointer.pointer_id;
    dispatcher->drag_active = M3_FALSE;
    dispatcher->down_time = event->time_ms;
    dispatcher->last_time = event->time_ms;
    dispatcher->down_x = x;
    dispatcher->down_y = y;
    dispatcher->last_x = x;
    dispatcher->last_y = y;
    dispatcher->velocity_x = 0.0f;
    dispatcher->velocity_y = 0.0f;
    if (out_target != NULL) {
      *out_target = target;
    }
    return M3_OK;
  case M3_INPUT_POINTER_MOVE:
    if (!dispatcher->pointer_active) {
      return M3_OK;
    }
    if (dispatcher->active_widget == NULL) {
      return M3_ERR_STATE;
    }
    if (event->data.pointer.pointer_id != dispatcher->active_pointer) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    rc = m3_gesture_time_delta(dispatcher->last_time, event->time_ms, &dt);
    if (rc != M3_OK) {
      return rc;
    }
    x = (M3Scalar)event->data.pointer.x;
    y = (M3Scalar)event->data.pointer.y;
    rc = m3_gesture_velocity(dispatcher->last_x, dispatcher->last_y, x, y, dt,
                             &velocity_x, &velocity_y);
    if (rc != M3_OK) {
      return rc;
    }
    dispatcher->velocity_x = velocity_x;
    dispatcher->velocity_y = velocity_y;

    rc = m3_gesture_distance_sq(dispatcher->down_x, dispatcher->down_y, x, y,
                                &dist_sq);
    if (rc != M3_OK) {
      return rc;
    }
    drag_threshold_sq = dispatcher->config.drag_start_distance *
                        dispatcher->config.drag_start_distance;

    if (!dispatcher->drag_active && dist_sq >= drag_threshold_sq) {
      M3GestureEvent gesture;

      dispatcher->drag_active = M3_TRUE;
      delta_x = x - dispatcher->last_x;
      delta_y = y - dispatcher->last_y;
      total_x = x - dispatcher->down_x;
      total_y = y - dispatcher->down_y;

      gesture.pointer_id = dispatcher->active_pointer;
      gesture.x = x;
      gesture.y = y;
      gesture.start_x = dispatcher->down_x;
      gesture.start_y = dispatcher->down_y;
      gesture.delta_x = delta_x;
      gesture.delta_y = delta_y;
      gesture.total_x = total_x;
      gesture.total_y = total_y;
      gesture.velocity_x = dispatcher->velocity_x;
      gesture.velocity_y = dispatcher->velocity_y;
      gesture.tap_count = 0u;

      dispatcher->last_x = x;
      dispatcher->last_y = y;
      dispatcher->last_time = event->time_ms;

      handled = M3_FALSE;
      rc = m3_gesture_emit(dispatcher->active_widget, event,
                           M3_INPUT_GESTURE_DRAG_START, &gesture, &handled);
      if (rc != M3_OK) {
        return rc;
      }
      handled_any = (handled_any || handled) ? M3_TRUE : M3_FALSE;
      if (out_target != NULL) {
        *out_target = dispatcher->active_widget;
      }
      *out_handled = handled_any;
      return M3_OK;
    }

    if (dispatcher->drag_active) {
      M3GestureEvent gesture;

      delta_x = x - dispatcher->last_x;
      delta_y = y - dispatcher->last_y;
      total_x = x - dispatcher->down_x;
      total_y = y - dispatcher->down_y;

      gesture.pointer_id = dispatcher->active_pointer;
      gesture.x = x;
      gesture.y = y;
      gesture.start_x = dispatcher->down_x;
      gesture.start_y = dispatcher->down_y;
      gesture.delta_x = delta_x;
      gesture.delta_y = delta_y;
      gesture.total_x = total_x;
      gesture.total_y = total_y;
      gesture.velocity_x = dispatcher->velocity_x;
      gesture.velocity_y = dispatcher->velocity_y;
      gesture.tap_count = 0u;

      dispatcher->last_x = x;
      dispatcher->last_y = y;
      dispatcher->last_time = event->time_ms;

      handled = M3_FALSE;
      rc = m3_gesture_emit(dispatcher->active_widget, event,
                           M3_INPUT_GESTURE_DRAG_UPDATE, &gesture, &handled);
      if (rc != M3_OK) {
        return rc;
      }
      handled_any = (handled_any || handled) ? M3_TRUE : M3_FALSE;
      if (out_target != NULL) {
        *out_target = dispatcher->active_widget;
      }
      *out_handled = handled_any;
      return M3_OK;
    }

    dispatcher->last_x = x;
    dispatcher->last_y = y;
    dispatcher->last_time = event->time_ms;
    return M3_OK;
  case M3_INPUT_POINTER_UP:
    if (!dispatcher->pointer_active) {
      return M3_OK;
    }
    if (dispatcher->active_widget == NULL) {
      return M3_ERR_STATE;
    }
    if (event->data.pointer.pointer_id != dispatcher->active_pointer) {
      return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_gesture_time_delta(dispatcher->last_time, event->time_ms, &dt);
    if (rc != M3_OK) {
      return rc;
    }
    rc =
        m3_gesture_time_delta(dispatcher->down_time, event->time_ms, &duration);
    if (rc != M3_OK) {
      return rc;
    }

    x = (M3Scalar)event->data.pointer.x;
    y = (M3Scalar)event->data.pointer.y;

    rc = m3_gesture_velocity(dispatcher->last_x, dispatcher->last_y, x, y, dt,
                             &velocity_x, &velocity_y);
    if (rc != M3_OK) {
      return rc;
    }

    dispatcher->velocity_x = velocity_x;
    dispatcher->velocity_y = velocity_y;

    rc = m3_gesture_distance_sq(dispatcher->down_x, dispatcher->down_y, x, y,
                                &dist_sq);
    if (rc != M3_OK) {
      return rc;
    }

    tap_threshold_sq = dispatcher->config.tap_max_distance *
                       dispatcher->config.tap_max_distance;
    double_tap_threshold_sq = dispatcher->config.double_tap_max_distance *
                              dispatcher->config.double_tap_max_distance;

    if (dispatcher->drag_active) {
      M3GestureEvent gesture;

      delta_x = x - dispatcher->last_x;
      delta_y = y - dispatcher->last_y;
      total_x = x - dispatcher->down_x;
      total_y = y - dispatcher->down_y;

      gesture.pointer_id = dispatcher->active_pointer;
      gesture.x = x;
      gesture.y = y;
      gesture.start_x = dispatcher->down_x;
      gesture.start_y = dispatcher->down_y;
      gesture.delta_x = delta_x;
      gesture.delta_y = delta_y;
      gesture.total_x = total_x;
      gesture.total_y = total_y;
      gesture.velocity_x = dispatcher->velocity_x;
      gesture.velocity_y = dispatcher->velocity_y;
      gesture.tap_count = 0u;

      handled = M3_FALSE;
      rc = m3_gesture_emit(dispatcher->active_widget, event,
                           M3_INPUT_GESTURE_DRAG_END, &gesture, &handled);
      if (rc != M3_OK) {
        return rc;
      }
      handled_any = (handled_any || handled) ? M3_TRUE : M3_FALSE;

      fling_threshold_sq = dispatcher->config.fling_min_velocity *
                           dispatcher->config.fling_min_velocity;
      speed_sq = dispatcher->velocity_x * dispatcher->velocity_x +
                 dispatcher->velocity_y * dispatcher->velocity_y;
      if (speed_sq >= fling_threshold_sq) {
        gesture.delta_x = 0.0f;
        gesture.delta_y = 0.0f;
        gesture.velocity_x = dispatcher->velocity_x;
        gesture.velocity_y = dispatcher->velocity_y;
        gesture.tap_count = 0u;
        handled = M3_FALSE;
        rc = m3_gesture_emit(dispatcher->active_widget, event,
                             M3_INPUT_GESTURE_FLING, &gesture, &handled);
        if (rc != M3_OK) {
          return rc;
        }
        handled_any = (handled_any || handled) ? M3_TRUE : M3_FALSE;
      }

      dispatcher->has_last_tap = M3_FALSE;
    } else if (duration >= dispatcher->config.long_press_ms &&
               dist_sq <= tap_threshold_sq) {
      M3GestureEvent gesture;

      gesture.pointer_id = dispatcher->active_pointer;
      gesture.x = x;
      gesture.y = y;
      gesture.start_x = dispatcher->down_x;
      gesture.start_y = dispatcher->down_y;
      gesture.delta_x = 0.0f;
      gesture.delta_y = 0.0f;
      gesture.total_x = 0.0f;
      gesture.total_y = 0.0f;
      gesture.velocity_x = 0.0f;
      gesture.velocity_y = 0.0f;
      gesture.tap_count = 0u;

      handled = M3_FALSE;
      rc = m3_gesture_emit(dispatcher->active_widget, event,
                           M3_INPUT_GESTURE_LONG_PRESS, &gesture, &handled);
      if (rc != M3_OK) {
        return rc;
      }
      handled_any = (handled_any || handled) ? M3_TRUE : M3_FALSE;
      dispatcher->has_last_tap = M3_FALSE;
    } else if (duration <= dispatcher->config.tap_max_ms &&
               dist_sq <= tap_threshold_sq) {
      M3Bool double_tap;
      M3Scalar tap_dist_sq;
      m3_u32 tap_dt;

      double_tap = M3_FALSE;
      if (dispatcher->has_last_tap &&
          dispatcher->last_tap_widget == dispatcher->active_widget) {
        if (event->time_ms >= dispatcher->last_tap_time) {
          tap_dt = event->time_ms - dispatcher->last_tap_time;
          rc = m3_gesture_distance_sq(dispatcher->last_tap_x,
                                      dispatcher->last_tap_y, x, y,
                                      &tap_dist_sq);
          if (rc != M3_OK) {
            return rc;
          }
          if (tap_dt <= dispatcher->config.double_tap_max_ms &&
              tap_dist_sq <= double_tap_threshold_sq) {
            double_tap = M3_TRUE;
          }
        } else {
          dispatcher->has_last_tap = M3_FALSE;
        }
      }

      if (double_tap) {
        M3GestureEvent gesture;

        gesture.pointer_id = dispatcher->active_pointer;
        gesture.x = x;
        gesture.y = y;
        gesture.start_x = dispatcher->down_x;
        gesture.start_y = dispatcher->down_y;
        gesture.delta_x = 0.0f;
        gesture.delta_y = 0.0f;
        gesture.total_x = 0.0f;
        gesture.total_y = 0.0f;
        gesture.velocity_x = 0.0f;
        gesture.velocity_y = 0.0f;
        gesture.tap_count = 2u;

        handled = M3_FALSE;
        rc = m3_gesture_emit(dispatcher->active_widget, event,
                             M3_INPUT_GESTURE_DOUBLE_TAP, &gesture, &handled);
        if (rc != M3_OK) {
          return rc;
        }
        handled_any = (handled_any || handled) ? M3_TRUE : M3_FALSE;
        dispatcher->has_last_tap = M3_FALSE;
      } else {
        M3GestureEvent gesture;

        gesture.pointer_id = dispatcher->active_pointer;
        gesture.x = x;
        gesture.y = y;
        gesture.start_x = dispatcher->down_x;
        gesture.start_y = dispatcher->down_y;
        gesture.delta_x = 0.0f;
        gesture.delta_y = 0.0f;
        gesture.total_x = 0.0f;
        gesture.total_y = 0.0f;
        gesture.velocity_x = 0.0f;
        gesture.velocity_y = 0.0f;
        gesture.tap_count = 1u;

        handled = M3_FALSE;
        rc = m3_gesture_emit(dispatcher->active_widget, event,
                             M3_INPUT_GESTURE_TAP, &gesture, &handled);
        if (rc != M3_OK) {
          return rc;
        }
        handled_any = (handled_any || handled) ? M3_TRUE : M3_FALSE;
        dispatcher->has_last_tap = M3_TRUE;
        dispatcher->last_tap_time = event->time_ms;
        dispatcher->last_tap_x = x;
        dispatcher->last_tap_y = y;
        dispatcher->last_tap_widget = dispatcher->active_widget;
      }
    }

    if (out_target != NULL) {
      *out_target = dispatcher->active_widget;
    }
    *out_handled = handled_any;
    rc = m3_gesture_reset_pointer(dispatcher);
    if (rc != M3_OK) {
      return rc;
    }
    return M3_OK;
  case M3_INPUT_POINTER_SCROLL:
    return M3_OK;
  default:
    return M3_ERR_INVALID_ARGUMENT;
  }
}

#ifdef M3_TESTING
int M3_CALL m3_gesture_test_set_contains_error(M3Bool enable) {
  g_m3_gesture_test_force_contains_error = enable;
  return M3_OK;
}

int M3_CALL m3_gesture_test_set_reset_fail(M3Bool enable) {
  g_m3_gesture_test_force_reset_fail = enable;
  return M3_OK;
}

int M3_CALL m3_gesture_test_set_velocity_error(M3Bool enable) {
  g_m3_gesture_test_force_velocity_error = enable;
  return M3_OK;
}

int M3_CALL m3_gesture_test_set_distance_error(M3Bool enable) {
  g_m3_gesture_test_force_distance_error = enable;
  return M3_OK;
}

int M3_CALL m3_gesture_test_set_distance_error_after(m3_i32 calls) {
  if (calls < 0) {
    return M3_ERR_RANGE;
  }
  g_m3_gesture_test_distance_error_after = (int)calls;
  return M3_OK;
}

int M3_CALL m3_gesture_test_set_config_init_fail(M3Bool enable) {
  g_m3_gesture_test_force_config_init_fail = enable;
  return M3_OK;
}

int M3_CALL m3_gesture_test_validate_rect(const M3Rect *rect) {
  return m3_gesture_validate_rect(rect);
}

int M3_CALL m3_gesture_test_distance_sq(M3Scalar x0, M3Scalar y0, M3Scalar x1,
                                        M3Scalar y1, M3Scalar *out_value) {
  return m3_gesture_distance_sq(x0, y0, x1, y1, out_value);
}

int M3_CALL m3_gesture_test_time_delta(m3_u32 start, m3_u32 end,
                                       m3_u32 *out_delta) {
  return m3_gesture_time_delta(start, end, out_delta);
}

int M3_CALL m3_gesture_test_velocity(M3Scalar x0, M3Scalar y0, M3Scalar x1,
                                     M3Scalar y1, m3_u32 dt_ms,
                                     M3Scalar *out_vx, M3Scalar *out_vy) {
  return m3_gesture_velocity(x0, y0, x1, y1, dt_ms, out_vx, out_vy);
}

int M3_CALL m3_gesture_test_dispatch_to_widget(M3Widget *widget,
                                               const M3InputEvent *event,
                                               M3Bool *out_handled) {
  return m3_gesture_dispatch_to_widget(widget, event, out_handled);
}

int M3_CALL m3_gesture_test_emit(M3Widget *widget, const M3InputEvent *source,
                                 m3_u32 type, const M3GestureEvent *gesture,
                                 M3Bool *out_handled) {
  return m3_gesture_emit(widget, source, type, gesture, out_handled);
}

int M3_CALL m3_gesture_test_reset_pointer(M3GestureDispatcher *dispatcher) {
  return m3_gesture_reset_pointer(dispatcher);
}

int M3_CALL m3_gesture_test_clear_state(M3GestureDispatcher *dispatcher) {
  return m3_gesture_clear_state(dispatcher);
}

int M3_CALL m3_gesture_test_validate_config(const M3GestureConfig *config) {
  return m3_gesture_validate_config(config);
}

int M3_CALL m3_gesture_test_hit_test(const M3RenderNode *node, M3Scalar x,
                                     M3Scalar y, M3Widget **out_widget) {
  return m3_gesture_hit_test(node, x, y, out_widget);
}
#endif
