#include "cmpc/cmp_gesture.h"

#include <string.h>

#ifdef CMP_TESTING
static CMPBool g_cmp_gesture_test_force_contains_error = CMP_FALSE;
static CMPBool g_cmp_gesture_test_force_reset_fail =
    CMP_FALSE; /* GCOVR_EXCL_LINE */
static CMPBool g_cmp_gesture_test_force_velocity_error = CMP_FALSE;
static CMPBool g_cmp_gesture_test_force_distance_error = CMP_FALSE;
static CMPBool g_cmp_gesture_test_force_config_init_fail = CMP_FALSE;
static int g_cmp_gesture_test_distance_error_after = 0;
#endif

static int cmp_gesture_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_gesture_validate_config(const CMPGestureConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->tap_max_distance < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (config->double_tap_max_distance < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (config->drag_start_distance < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (config->fling_min_velocity < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_gesture_distance_sq(CMPScalar x0, CMPScalar y0, CMPScalar x1,
                                   CMPScalar y1, CMPScalar *out_value) {
  CMPScalar dx;
  CMPScalar dy;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (g_cmp_gesture_test_force_distance_error == CMP_TRUE) {
    g_cmp_gesture_test_force_distance_error = CMP_FALSE;
    return CMP_ERR_UNKNOWN;
  }
  if (g_cmp_gesture_test_distance_error_after > 0) {
    g_cmp_gesture_test_distance_error_after -= 1;
    if (g_cmp_gesture_test_distance_error_after == 0) {
      return CMP_ERR_UNKNOWN;
    }
  }
#endif

  dx = x1 - x0;
  dy = y1 - y0;
  *out_value = dx * dx + dy * dy;
  return CMP_OK;
}

static int cmp_gesture_time_delta(cmp_u32 start, cmp_u32 end,
                                  cmp_u32 *out_delta) {
  if (out_delta == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (end < start) {
    return CMP_ERR_RANGE;
  }
  *out_delta = end - start;
  return CMP_OK;
}

static int cmp_gesture_velocity(CMPScalar x0, CMPScalar y0, CMPScalar x1,
                                CMPScalar y1, cmp_u32 dt_ms, CMPScalar *out_vx,
                                CMPScalar *out_vy) {
  CMPScalar scale;

  if (out_vx == NULL || out_vy == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_gesture_test_force_velocity_error == CMP_TRUE) {
    g_cmp_gesture_test_force_velocity_error = CMP_FALSE;
    return CMP_ERR_UNKNOWN;
  }
#endif

  if (dt_ms == 0u) {
    *out_vx = 0.0f;
    *out_vy = 0.0f;
    return CMP_OK;
  }

  scale = 1000.0f / (CMPScalar)dt_ms;
  *out_vx = (x1 - x0) * scale;
  *out_vy = (y1 - y0) * scale;
  return CMP_OK;
}

static int cmp_gesture_hit_test(const CMPRenderNode *node, CMPScalar x,
                                CMPScalar y, CMPWidget **out_widget) {
  CMPBool contains;
  cmp_usize i;
  int rc; /* GCOVR_EXCL_LINE */

  if (node == NULL || out_widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_widget = NULL;

  if (node->widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (node->widget->flags & CMP_WIDGET_FLAG_HIDDEN) {
    return CMP_OK;
  }
  if (node->widget->flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  rc = cmp_gesture_validate_rect(&node->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  contains = CMP_FALSE;
  rc = cmp_rect_contains_point(&node->bounds, x, y, &contains);
#ifdef CMP_TESTING
  if (g_cmp_gesture_test_force_contains_error == CMP_TRUE) {
    g_cmp_gesture_test_force_contains_error = CMP_FALSE;
    rc = CMP_ERR_RANGE;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  if (contains == CMP_FALSE) {
    return CMP_OK;
  }

  if (node->child_count > 0 && node->children == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = node->child_count; i > 0; --i) {
    CMPRenderNode *child = node->children[i - 1];

    if (child == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }

    rc = cmp_gesture_hit_test(child, x, y, out_widget);
    if (rc != CMP_OK) {
      return rc;
    }
    if (*out_widget != NULL) {
      return CMP_OK;
    }
  }

  *out_widget = node->widget;
  return CMP_OK;
}

static int cmp_gesture_dispatch_to_widget(CMPWidget *widget,
                                          const CMPInputEvent *event,
                                          CMPBool *out_handled) {
  CMPBool handled;
  int rc; /* GCOVR_EXCL_LINE */

  if (out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  if (widget == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (widget->vtable == NULL || widget->vtable->event == NULL) {
    return CMP_OK;
  }

  handled = CMP_FALSE;
  rc = widget->vtable->event(widget->ctx, event, &handled);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_handled = handled;
  return CMP_OK;
}

static int cmp_gesture_emit(CMPWidget *widget, const CMPInputEvent *source,
                            cmp_u32 type, const CMPGestureEvent *gesture,
                            CMPBool *out_handled) {
  CMPInputEvent event; /* GCOVR_EXCL_LINE */
  int rc;

  if (out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  if (widget == NULL) {
    return CMP_OK;
  }
  if (source == NULL || gesture == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(&event, 0, sizeof(event));
  event.type = type;
  event.modifiers = source->modifiers;
  event.time_ms = source->time_ms;
  event.window = source->window;
  event.data.gesture = *gesture;

  rc = cmp_gesture_dispatch_to_widget(widget, &event, out_handled);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int cmp_gesture_reset_pointer(CMPGestureDispatcher *dispatcher) {
  if (dispatcher == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#ifdef CMP_TESTING
  if (g_cmp_gesture_test_force_reset_fail == CMP_TRUE) {
    g_cmp_gesture_test_force_reset_fail = CMP_FALSE;
    return CMP_ERR_UNKNOWN;
  }
#endif

  dispatcher->active_widget = NULL;
  dispatcher->pointer_active = CMP_FALSE;
  dispatcher->active_pointer = 0;
  dispatcher->drag_active = CMP_FALSE;
  dispatcher->down_time = 0u;
  dispatcher->last_time = 0u;
  dispatcher->down_x = 0.0f;
  dispatcher->down_y = 0.0f;
  dispatcher->last_x = 0.0f;
  dispatcher->last_y = 0.0f;
  dispatcher->velocity_x = 0.0f;
  dispatcher->velocity_y = 0.0f;
  return CMP_OK;
}

static int cmp_gesture_clear_state(CMPGestureDispatcher *dispatcher) {
  int rc;

  if (dispatcher == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_gesture_reset_pointer(dispatcher);
  if (rc != CMP_OK) {
    return rc;
  }

  dispatcher->has_last_tap = CMP_FALSE;
  dispatcher->last_tap_time = 0u;
  dispatcher->last_tap_x = 0.0f;
  dispatcher->last_tap_y = 0.0f;
  dispatcher->last_tap_widget = NULL;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_config_init(CMPGestureConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_gesture_test_force_config_init_fail == CMP_TRUE) {
    g_cmp_gesture_test_force_config_init_fail = CMP_FALSE;
    return CMP_ERR_UNKNOWN;
  }
#endif

  config->tap_max_ms = CMP_GESTURE_DEFAULT_TAP_MAX_MS;
  config->tap_max_distance = CMP_GESTURE_DEFAULT_TAP_MAX_DISTANCE;
  config->double_tap_max_ms = CMP_GESTURE_DEFAULT_DOUBLE_TAP_MAX_MS;
  config->double_tap_max_distance = CMP_GESTURE_DEFAULT_DOUBLE_TAP_MAX_DISTANCE;
  config->long_press_ms = CMP_GESTURE_DEFAULT_LONG_PRESS_MS;
  config->drag_start_distance = CMP_GESTURE_DEFAULT_DRAG_START_DISTANCE;
  config->fling_min_velocity = CMP_GESTURE_DEFAULT_FLING_MIN_VELOCITY;

  return cmp_gesture_validate_config(config);
}

int CMP_CALL cmp_gesture_dispatcher_init(CMPGestureDispatcher *dispatcher,
                                         const CMPGestureConfig *config) {
  CMPGestureConfig defaults; /* GCOVR_EXCL_LINE */
  int rc;

  if (dispatcher == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (dispatcher->initialized) {
    return CMP_ERR_STATE;
  }

  if (config == NULL) {
    rc = cmp_gesture_config_init(&defaults);
    if (rc != CMP_OK) {
      return rc;
    }
    dispatcher->config = defaults;
  } else {
    rc = cmp_gesture_validate_config(config);
    if (rc != CMP_OK) {
      return rc;
    }
    dispatcher->config = *config;
  }

  dispatcher->initialized = CMP_TRUE;
  rc = cmp_gesture_clear_state(dispatcher);
  if (rc != CMP_OK) {
    dispatcher->initialized = CMP_FALSE;
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_gesture_dispatcher_shutdown(CMPGestureDispatcher *dispatcher) {
  int rc;

  if (dispatcher == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_gesture_clear_state(dispatcher);
  if (rc != CMP_OK) {
    return rc;
  }

  dispatcher->initialized = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_dispatcher_reset(CMPGestureDispatcher *dispatcher) {
  if (dispatcher == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return CMP_ERR_STATE;
  }

  return cmp_gesture_clear_state(dispatcher);
}

int CMP_CALL cmp_gesture_dispatcher_set_config(CMPGestureDispatcher *dispatcher,
                                               const CMPGestureConfig *config) {
  int rc;

  if (dispatcher == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_gesture_validate_config(config);
  if (rc != CMP_OK) {
    return rc;
  }

  dispatcher->config = *config;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_dispatcher_get_config(
    const CMPGestureDispatcher *dispatcher, CMPGestureConfig *out_config) {
  if (dispatcher == NULL || out_config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return CMP_ERR_STATE;
  }

  *out_config = dispatcher->config;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_dispatch(CMPGestureDispatcher *dispatcher,
                                  const CMPRenderNode *root,
                                  const CMPInputEvent *event,
                                  CMPWidget **out_target,
                                  CMPBool *out_handled) {
  CMPWidget *target;
  CMPBool handled;
  CMPBool handled_any;
  CMPScalar x; /* GCOVR_EXCL_LINE */
  CMPScalar y;
  CMPScalar dist_sq;
  CMPScalar drag_threshold_sq; /* GCOVR_EXCL_LINE */
  CMPScalar tap_threshold_sq;
  CMPScalar double_tap_threshold_sq;
  CMPScalar delta_x;
  CMPScalar delta_y;
  CMPScalar total_x;
  CMPScalar total_y;
  CMPScalar velocity_x;
  CMPScalar velocity_y;
  CMPScalar speed_sq;
  CMPScalar fling_threshold_sq;
  cmp_u32 dt;
  cmp_u32 duration;
  int rc;

  if (dispatcher == NULL || root == NULL || event == NULL ||
      out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return CMP_ERR_STATE;
  }

  *out_handled = CMP_FALSE;
  if (out_target != NULL) {
    *out_target = NULL;
  }

  handled_any = CMP_FALSE;
  target = NULL;

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
    if (dispatcher->pointer_active) {
      if (event->data.pointer.pointer_id == dispatcher->active_pointer) {
        return CMP_ERR_STATE;
      }
      return CMP_ERR_BUSY;
    }
    x = (CMPScalar)event->data.pointer.x;
    y = (CMPScalar)event->data.pointer.y;
    rc = cmp_gesture_hit_test(root, x, y, &target);
    if (rc != CMP_OK) {
      return rc;
    }
    if (target == NULL) {
      return CMP_OK;
    }

    dispatcher->active_widget = target;
    dispatcher->pointer_active = CMP_TRUE;
    dispatcher->active_pointer = event->data.pointer.pointer_id;
    dispatcher->drag_active = CMP_FALSE;
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
    return CMP_OK;
  case CMP_INPUT_POINTER_MOVE:
    if (!dispatcher->pointer_active) {
      return CMP_OK;
    }
    if (dispatcher->active_widget == NULL) {
      return CMP_ERR_STATE;
    }
    if (event->data.pointer.pointer_id != dispatcher->active_pointer) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    rc = cmp_gesture_time_delta(dispatcher->last_time, event->time_ms, &dt);
    if (rc != CMP_OK) {
      return rc;
    }
    x = (CMPScalar)event->data.pointer.x;
    y = (CMPScalar)event->data.pointer.y;
    rc = cmp_gesture_velocity(dispatcher->last_x, dispatcher->last_y, x, y, dt,
                              &velocity_x, &velocity_y);
    if (rc != CMP_OK) {
      return rc;
    }
    dispatcher->velocity_x = velocity_x;
    dispatcher->velocity_y = velocity_y;

    rc = cmp_gesture_distance_sq(dispatcher->down_x, dispatcher->down_y, x, y,
                                 &dist_sq);
    if (rc != CMP_OK) {
      return rc;
    }
    drag_threshold_sq = dispatcher->config.drag_start_distance *
                        dispatcher->config.drag_start_distance;

    if (!dispatcher->drag_active && dist_sq >= drag_threshold_sq) {
      CMPGestureEvent gesture;

      dispatcher->drag_active = CMP_TRUE;
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

      handled = CMP_FALSE;
      rc = cmp_gesture_emit(dispatcher->active_widget, event,
                            CMP_INPUT_GESTURE_DRAG_START, &gesture, &handled);
      if (rc != CMP_OK) {
        return rc;
      }
      handled_any = (handled_any || handled) ? CMP_TRUE : CMP_FALSE;
      if (out_target != NULL) {
        *out_target = dispatcher->active_widget;
      }
      *out_handled = handled_any;
      return CMP_OK;
    }

    if (dispatcher->drag_active) {
      CMPGestureEvent gesture;

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

      handled = CMP_FALSE;
      rc = cmp_gesture_emit(dispatcher->active_widget, event,
                            CMP_INPUT_GESTURE_DRAG_UPDATE, &gesture, &handled);
      if (rc != CMP_OK) {
        return rc;
      }
      handled_any = (handled_any || handled) ? CMP_TRUE : CMP_FALSE;
      if (out_target != NULL) {
        *out_target = dispatcher->active_widget;
      }
      *out_handled = handled_any;
      return CMP_OK;
    }

    dispatcher->last_x = x;
    dispatcher->last_y = y;
    dispatcher->last_time = event->time_ms;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:
    if (!dispatcher->pointer_active) {
      return CMP_OK;
    }
    if (dispatcher->active_widget == NULL) {
      return CMP_ERR_STATE;
    }
    if (event->data.pointer.pointer_id != dispatcher->active_pointer) {
      return CMP_ERR_INVALID_ARGUMENT;
    }

    rc = cmp_gesture_time_delta(dispatcher->last_time, event->time_ms, &dt);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_gesture_time_delta(dispatcher->down_time, event->time_ms,
                                &duration);
    if (rc != CMP_OK) {
      return rc;
    }

    x = (CMPScalar)event->data.pointer.x;
    y = (CMPScalar)event->data.pointer.y;

    rc = cmp_gesture_velocity(dispatcher->last_x, dispatcher->last_y, x, y, dt,
                              &velocity_x, &velocity_y);
    if (rc != CMP_OK) {
      return rc;
    }

    dispatcher->velocity_x = velocity_x;
    dispatcher->velocity_y = velocity_y;

    rc = cmp_gesture_distance_sq(dispatcher->down_x, dispatcher->down_y, x, y,
                                 &dist_sq);
    if (rc != CMP_OK) {
      return rc;
    }

    tap_threshold_sq = dispatcher->config.tap_max_distance *
                       dispatcher->config.tap_max_distance;
    double_tap_threshold_sq = dispatcher->config.double_tap_max_distance *
                              dispatcher->config.double_tap_max_distance;

    if (dispatcher->drag_active) {
      CMPGestureEvent gesture;

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

      handled = CMP_FALSE;
      rc = cmp_gesture_emit(dispatcher->active_widget, event,
                            CMP_INPUT_GESTURE_DRAG_END, &gesture, &handled);
      if (rc != CMP_OK) {
        return rc;
      }
      handled_any = (handled_any || handled) ? CMP_TRUE : CMP_FALSE;

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
        handled = CMP_FALSE;
        rc = cmp_gesture_emit(dispatcher->active_widget, event,
                              CMP_INPUT_GESTURE_FLING, &gesture, &handled);
        if (rc != CMP_OK) {
          return rc;
        }
        handled_any = (handled_any || handled) ? CMP_TRUE : CMP_FALSE;
      }

      dispatcher->has_last_tap = CMP_FALSE;
    } else if (duration >= dispatcher->config.long_press_ms &&
               dist_sq <= tap_threshold_sq) {
      CMPGestureEvent gesture;

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

      handled = CMP_FALSE;
      rc = cmp_gesture_emit(dispatcher->active_widget, event,
                            CMP_INPUT_GESTURE_LONG_PRESS, &gesture, &handled);
      if (rc != CMP_OK) {
        return rc;
      }
      handled_any = (handled_any || handled) ? CMP_TRUE : CMP_FALSE;
      dispatcher->has_last_tap = CMP_FALSE;
    } else if (duration <= dispatcher->config.tap_max_ms &&
               dist_sq <= tap_threshold_sq) {
      CMPBool double_tap;
      CMPScalar tap_dist_sq;
      cmp_u32 tap_dt;

      double_tap = CMP_FALSE;
      if (dispatcher->has_last_tap &&
          dispatcher->last_tap_widget == dispatcher->active_widget) {
        if (event->time_ms >= dispatcher->last_tap_time) {
          tap_dt = event->time_ms - dispatcher->last_tap_time;
          rc = cmp_gesture_distance_sq(dispatcher->last_tap_x,
                                       dispatcher->last_tap_y, x, y,
                                       &tap_dist_sq);
          if (rc != CMP_OK) {
            return rc;
          }
          if (tap_dt <= dispatcher->config.double_tap_max_ms &&
              tap_dist_sq <= double_tap_threshold_sq) {
            double_tap = CMP_TRUE;
          }
        } else {
          dispatcher->has_last_tap = CMP_FALSE;
        }
      }

      if (double_tap) {
        CMPGestureEvent gesture;

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

        handled = CMP_FALSE;
        rc = cmp_gesture_emit(dispatcher->active_widget, event,
                              CMP_INPUT_GESTURE_DOUBLE_TAP, &gesture, &handled);
        if (rc != CMP_OK) {
          return rc;
        }
        handled_any = (handled_any || handled) ? CMP_TRUE : CMP_FALSE;
        dispatcher->has_last_tap = CMP_FALSE;
      } else {
        CMPGestureEvent gesture;

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

        handled = CMP_FALSE;
        rc = cmp_gesture_emit(dispatcher->active_widget, event,
                              CMP_INPUT_GESTURE_TAP, &gesture, &handled);
        if (rc != CMP_OK) {
          return rc;
        }
        handled_any = (handled_any || handled) ? CMP_TRUE : CMP_FALSE;
        dispatcher->has_last_tap = CMP_TRUE;
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
    rc = cmp_gesture_reset_pointer(dispatcher);
    if (rc != CMP_OK) {
      return rc;
    }
    return CMP_OK;
  case CMP_INPUT_POINTER_SCROLL:
    return CMP_OK;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

#ifdef CMP_TESTING
int CMP_CALL cmp_gesture_test_set_contains_error(CMPBool enable) {
  g_cmp_gesture_test_force_contains_error = enable;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_test_set_reset_fail(CMPBool enable) {
  g_cmp_gesture_test_force_reset_fail = enable;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_test_set_velocity_error(CMPBool enable) {
  g_cmp_gesture_test_force_velocity_error = enable;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_test_set_distance_error(CMPBool enable) {
  g_cmp_gesture_test_force_distance_error = enable;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_test_set_distance_error_after(cmp_i32 calls) {
  if (calls < 0) {
    return CMP_ERR_RANGE;
  }
  g_cmp_gesture_test_distance_error_after = (int)calls;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_test_set_config_init_fail(CMPBool enable) {
  g_cmp_gesture_test_force_config_init_fail = enable;
  return CMP_OK;
}

int CMP_CALL cmp_gesture_test_validate_rect(const CMPRect *rect) {
  return cmp_gesture_validate_rect(rect);
}

int CMP_CALL cmp_gesture_test_distance_sq(CMPScalar x0, CMPScalar y0,
                                          CMPScalar x1, CMPScalar y1,
                                          CMPScalar *out_value) {
  return cmp_gesture_distance_sq(x0, y0, x1, y1, out_value);
}

int CMP_CALL cmp_gesture_test_time_delta(cmp_u32 start, cmp_u32 end,
                                         cmp_u32 *out_delta) {
  return cmp_gesture_time_delta(start, end, out_delta);
}

int CMP_CALL cmp_gesture_test_velocity(CMPScalar x0, CMPScalar y0, CMPScalar x1,
                                       CMPScalar y1, cmp_u32 dt_ms,
                                       CMPScalar *out_vx, CMPScalar *out_vy) {
  return cmp_gesture_velocity(x0, y0, x1, y1, dt_ms, out_vx, out_vy);
}

int CMP_CALL cmp_gesture_test_dispatch_to_widget(CMPWidget *widget,
                                                 const CMPInputEvent *event,
                                                 CMPBool *out_handled) {
  return cmp_gesture_dispatch_to_widget(widget, event, out_handled);
}

int CMP_CALL cmp_gesture_test_emit(CMPWidget *widget,
                                   const CMPInputEvent *source, cmp_u32 type,
                                   const CMPGestureEvent *gesture,
                                   CMPBool *out_handled) {
  return cmp_gesture_emit(widget, source, type, gesture, out_handled);
}

int CMP_CALL cmp_gesture_test_reset_pointer(CMPGestureDispatcher *dispatcher) {
  return cmp_gesture_reset_pointer(dispatcher);
}

int CMP_CALL cmp_gesture_test_clear_state(CMPGestureDispatcher *dispatcher) {
  return cmp_gesture_clear_state(dispatcher);
}

int CMP_CALL cmp_gesture_test_validate_config(const CMPGestureConfig *config) {
  return cmp_gesture_validate_config(config);
}

int CMP_CALL cmp_gesture_test_hit_test(const CMPRenderNode *node, CMPScalar x,
                                       CMPScalar y, CMPWidget **out_widget) {
  return cmp_gesture_hit_test(node, x, y, out_widget);
}
#endif
