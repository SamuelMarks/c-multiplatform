#include "m3/m3_event.h"

#ifdef M3_TESTING
static M3Bool g_m3_event_force_contains_error = M3_FALSE;
#endif
static int m3_event_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static M3Bool m3_event_widget_focusable(const M3Widget *widget) {
  if (widget == NULL) {
    return M3_FALSE;
  }
  if (widget->flags & M3_WIDGET_FLAG_HIDDEN) {
    return M3_FALSE;
  }
  if (widget->flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_FALSE;
  }
  if (widget->flags & M3_WIDGET_FLAG_FOCUSABLE) {
    return M3_TRUE;
  }
  return M3_FALSE;
}

static int m3_event_hit_test(const M3RenderNode *node, M3Scalar x, M3Scalar y,
                             M3Widget **out_widget) {
  M3Bool contains;
  m3_usize i;
  int rc;

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

  rc = m3_event_validate_rect(&node->bounds);
  if (rc != M3_OK) {
    return rc;
  }

#ifdef M3_TESTING
  if (g_m3_event_force_contains_error == M3_TRUE) {
    rc = m3_rect_contains_point(&node->bounds, x, y, NULL);
  } else {
    rc = m3_rect_contains_point(&node->bounds, x, y, &contains);
  }
#else
  rc = m3_rect_contains_point(&node->bounds, x, y, &contains);
#endif
  if (rc != M3_OK) {
    return rc;
  }
  if (!contains) {
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

    rc = m3_event_hit_test(child, x, y, out_widget);
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

static int m3_event_dispatch_to_widget(M3Widget *widget,
                                       const M3InputEvent *event,
                                       M3Bool *out_handled) {
  M3Bool handled;

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
  {
    int rc = widget->vtable->event(widget->ctx, event, &handled);
    if (rc != M3_OK) {
      return rc;
    }
  }

  *out_handled = handled;
  return M3_OK;
}

int M3_CALL m3_event_dispatcher_init(M3EventDispatcher *dispatcher) {
  if (dispatcher == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  dispatcher->initialized = M3_TRUE;
  dispatcher->focused = NULL;
  dispatcher->focus_visible = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_event_dispatcher_shutdown(M3EventDispatcher *dispatcher) {
  if (dispatcher == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  dispatcher->initialized = M3_FALSE;
  dispatcher->focused = NULL;
  dispatcher->focus_visible = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_event_dispatcher_get_focus(const M3EventDispatcher *dispatcher,
                                          M3Widget **out_widget) {
  if (dispatcher == NULL || out_widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  *out_widget = dispatcher->focused;
  return M3_OK;
}

int M3_CALL m3_event_dispatcher_set_focus(M3EventDispatcher *dispatcher,
                                          M3Widget *widget) {
  if (dispatcher == NULL || widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }
  if (!m3_event_widget_focusable(widget)) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  dispatcher->focused = widget;
  return M3_OK;
}

int M3_CALL m3_event_dispatcher_clear_focus(M3EventDispatcher *dispatcher) {
  if (dispatcher == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  dispatcher->focused = NULL;
  dispatcher->focus_visible = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_event_dispatcher_get_focus_visible(
    const M3EventDispatcher *dispatcher, M3Bool *out_visible) {
  if (dispatcher == NULL || out_visible == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  *out_visible = dispatcher->focus_visible;
  return M3_OK;
}

int M3_CALL m3_event_dispatcher_set_focus_visible(M3EventDispatcher *dispatcher,
                                                  M3Bool visible) {
  if (dispatcher == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  dispatcher->focus_visible = visible ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_event_dispatch(M3EventDispatcher *dispatcher,
                              const M3RenderNode *root,
                              const M3InputEvent *event, M3Widget **out_target,
                              M3Bool *out_handled) {
  M3Widget *target;
  M3Bool handled;
  int rc;

  if (dispatcher == NULL || root == NULL || event == NULL ||
      out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!dispatcher->initialized) {
    return M3_ERR_STATE;
  }

  target = NULL;
  *out_handled = M3_FALSE;
  if (out_target != NULL) {
    *out_target = NULL;
  }

  switch (event->type) {
  case M3_INPUT_POINTER_DOWN:
  case M3_INPUT_POINTER_UP:
  case M3_INPUT_POINTER_MOVE:
  case M3_INPUT_POINTER_SCROLL:
    rc = m3_event_hit_test(root, (M3Scalar)event->data.pointer.x,
                           (M3Scalar)event->data.pointer.y, &target);
    if (rc != M3_OK) {
      return rc;
    }
    if (target == NULL) {
      if (event->type == M3_INPUT_POINTER_DOWN) {
        dispatcher->focused = NULL;
        dispatcher->focus_visible = M3_FALSE;
      }
      return M3_OK;
    }

    handled = M3_FALSE;
    rc = m3_event_dispatch_to_widget(target, event, &handled);
    if (rc != M3_OK) {
      return rc;
    }

    if (event->type == M3_INPUT_POINTER_DOWN) {
      if (m3_event_widget_focusable(target)) {
        dispatcher->focused = target;
      } else {
        dispatcher->focused = NULL;
      }
      dispatcher->focus_visible = M3_FALSE;
    }

    if (out_target != NULL) {
      *out_target = target;
    }
    *out_handled = handled;
    return M3_OK;
  case M3_INPUT_KEY_DOWN:
  case M3_INPUT_KEY_UP:
  case M3_INPUT_TEXT:
  case M3_INPUT_TEXT_UTF8:
  case M3_INPUT_TEXT_EDIT:
    if (dispatcher->focused == NULL) {
      return M3_OK;
    }
    handled = M3_FALSE;
    rc = m3_event_dispatch_to_widget(dispatcher->focused, event, &handled);
    if (rc != M3_OK) {
      return rc;
    }
    dispatcher->focus_visible = M3_TRUE;
    if (out_target != NULL) {
      *out_target = dispatcher->focused;
    }
    *out_handled = handled;
    return M3_OK;
  case M3_INPUT_WINDOW_RESIZE:
  case M3_INPUT_WINDOW_CLOSE:
  case M3_INPUT_WINDOW_FOCUS:
  case M3_INPUT_WINDOW_BLUR:
    handled = M3_FALSE;
    rc = m3_event_dispatch_to_widget(root->widget, event, &handled);
    if (event->type == M3_INPUT_WINDOW_BLUR) {
      dispatcher->focused = NULL;
      dispatcher->focus_visible = M3_FALSE;
    }
    if (rc != M3_OK) {
      return rc;
    }
    if (out_target != NULL) {
      *out_target = root->widget;
    }
    *out_handled = handled;
    return M3_OK;
  default:
    return M3_ERR_INVALID_ARGUMENT;
  }
}

#ifdef M3_TESTING
int M3_CALL m3_event_test_validate_rect(const M3Rect *rect) {
  return m3_event_validate_rect(rect);
}

int M3_CALL m3_event_test_widget_focusable(const M3Widget *widget,
                                           M3Bool *out_focusable) {
  if (out_focusable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_focusable = m3_event_widget_focusable(widget);
  return M3_OK;
}

int M3_CALL m3_event_test_hit_test(const M3RenderNode *node, M3Scalar x,
                                   M3Scalar y, M3Widget **out_widget) {
  return m3_event_hit_test(node, x, y, out_widget);
}

int M3_CALL m3_event_test_dispatch_to_widget(M3Widget *widget,
                                             const M3InputEvent *event,
                                             M3Bool *out_handled) {
  return m3_event_dispatch_to_widget(widget, event, out_handled);
}

int M3_CALL m3_event_test_set_force_contains_error(M3Bool enable) {
  if (enable != M3_FALSE && enable != M3_TRUE) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  g_m3_event_force_contains_error = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}
#endif
