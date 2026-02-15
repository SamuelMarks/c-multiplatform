#include "cmpc/cmp_event.h"

#ifdef CMP_TESTING

static CMPBool g_cmp_event_force_contains_error = CMP_FALSE;

#endif

typedef struct CMPFocusContext {

  CMPWidget *target;

  CMPWidget *first;

  CMPWidget *last;

  CMPWidget *prev;

  CMPWidget *next;

  CMPWidget *last_visited;

  CMPBool found_target;

} CMPFocusContext;

static int cmp_event_validate_rect(const CMPRect *rect) {

  if (rect == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (rect->width < 0.0f || rect->height < 0.0f) {

    return CMP_ERR_RANGE;
  }

  return CMP_OK;
}

static CMPBool cmp_event_widget_focusable(const CMPWidget *widget) {

  if (widget == NULL) {

    return CMP_FALSE;
  }

  if (widget->flags & CMP_WIDGET_FLAG_HIDDEN) {

    return CMP_FALSE;
  }

  if (widget->flags & CMP_WIDGET_FLAG_DISABLED) {

    return CMP_FALSE;
  }

  if (widget->flags & CMP_WIDGET_FLAG_FOCUSABLE) {

    return CMP_TRUE;
  }

  return CMP_FALSE;
}

static void cmp_event_traverse_focus(const CMPRenderNode *node,

                                     CMPFocusContext *ctx) {

  cmp_usize i;

  if (node == NULL || node->widget == NULL) {

    return;
  }

  /* Visit node */

  if (cmp_event_widget_focusable(node->widget)) {

    if (ctx->first == NULL) {

      ctx->first = node->widget;
    }

    ctx->last = node->widget;

    if (ctx->found_target) {

      if (ctx->next == NULL) {

        ctx->next = node->widget;
      }
    }

    if (node->widget == ctx->target) {

      ctx->found_target = CMP_TRUE;

      ctx->prev = ctx->last_visited;
    }

    ctx->last_visited = node->widget;
  }

  /* Visit children */

  if (node->child_count > 0 && node->children != NULL) {

    for (i = 0; i < node->child_count; ++i) {

      cmp_event_traverse_focus(node->children[i], ctx);
    }
  }
}

static int cmp_event_hit_test(const CMPRenderNode *node, CMPScalar x,

                              CMPScalar y, CMPWidget **out_widget) {

  CMPBool contains = CMP_FALSE;

  cmp_usize i;

  int rc;

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

  rc = cmp_event_validate_rect(&node->bounds);

  if (rc != CMP_OK) {

    return rc;
  }

#ifdef CMP_TESTING

  if (g_cmp_event_force_contains_error == CMP_TRUE) {

    rc = cmp_rect_contains_point(&node->bounds, x, y, NULL);

  } else {

    rc = cmp_rect_contains_point(&node->bounds, x, y, &contains);
  }

#else

  rc = cmp_rect_contains_point(&node->bounds, x, y, &contains);

#endif

  if (rc != CMP_OK) {

    return rc;
  }

  if (!contains) {

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

    rc = cmp_event_hit_test(child, x, y, out_widget);

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

static int cmp_event_dispatch_to_widget(CMPWidget *widget,

                                        const CMPInputEvent *event,

                                        CMPBool *out_handled) {

  CMPBool handled;

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

  {

    int rc = widget->vtable->event(widget->ctx, event, &handled);

    if (rc != CMP_OK) {

      return rc;
    }
  }

  *out_handled = handled;

  return CMP_OK;
}

int CMP_CALL cmp_event_dispatcher_init(CMPEventDispatcher *dispatcher) {

  if (dispatcher == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (dispatcher->initialized) {

    return CMP_ERR_STATE;
  }

  dispatcher->initialized = CMP_TRUE;

  dispatcher->focused = NULL;

  dispatcher->focus_visible = CMP_FALSE;

  return CMP_OK;
}

int CMP_CALL cmp_event_dispatcher_shutdown(CMPEventDispatcher *dispatcher) {

  if (dispatcher == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!dispatcher->initialized) {

    return CMP_ERR_STATE;
  }

  dispatcher->initialized = CMP_FALSE;

  dispatcher->focused = NULL;

  dispatcher->focus_visible = CMP_FALSE;

  return CMP_OK;
}

int CMP_CALL cmp_event_dispatcher_get_focus(

    const CMPEventDispatcher *dispatcher, CMPWidget **out_widget) {

  if (dispatcher == NULL || out_widget == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!dispatcher->initialized) {

    return CMP_ERR_STATE;
  }

  *out_widget = dispatcher->focused;

  return CMP_OK;
}

int CMP_CALL cmp_event_dispatcher_set_focus(CMPEventDispatcher *dispatcher,

                                            CMPWidget *widget) {

  if (dispatcher == NULL || widget == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!dispatcher->initialized) {

    return CMP_ERR_STATE;
  }

  if (!cmp_event_widget_focusable(widget)) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  dispatcher->focused = widget;

  return CMP_OK;
}

int CMP_CALL cmp_event_dispatcher_clear_focus(CMPEventDispatcher *dispatcher) {

  if (dispatcher == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!dispatcher->initialized) {

    return CMP_ERR_STATE;
  }

  dispatcher->focused = NULL;

  dispatcher->focus_visible = CMP_FALSE;

  return CMP_OK;
}

int CMP_CALL cmp_event_dispatcher_get_focus_visible(

    const CMPEventDispatcher *dispatcher, CMPBool *out_visible) {

  if (dispatcher == NULL || out_visible == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!dispatcher->initialized) {

    return CMP_ERR_STATE;
  }

  *out_visible = dispatcher->focus_visible;

  return CMP_OK;
}

int CMP_CALL cmp_event_dispatcher_set_focus_visible(

    CMPEventDispatcher *dispatcher, CMPBool visible) {

  if (dispatcher == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!dispatcher->initialized) {

    return CMP_ERR_STATE;
  }

  dispatcher->focus_visible = visible ? CMP_TRUE : CMP_FALSE;

  return CMP_OK;
}

int CMP_CALL cmp_event_dispatch(CMPEventDispatcher *dispatcher,

                                const CMPRenderNode *root,

                                const CMPInputEvent *event,

                                CMPWidget **out_target, CMPBool *out_handled) {

  CMPWidget *target;

  CMPBool handled;

  int rc;

  if (dispatcher == NULL || root == NULL || event == NULL ||

      out_handled == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!dispatcher->initialized) {

    return CMP_ERR_STATE;
  }

  target = NULL;

  *out_handled = CMP_FALSE;

  if (out_target != NULL) {

    *out_target = NULL;
  }

  switch (event->type) {

  case CMP_INPUT_POINTER_DOWN:

  case CMP_INPUT_POINTER_UP:

  case CMP_INPUT_POINTER_MOVE:

  case CMP_INPUT_POINTER_SCROLL:

    rc = cmp_event_hit_test(root, (CMPScalar)event->data.pointer.x,

                            (CMPScalar)event->data.pointer.y, &target);

    if (rc != CMP_OK) {

      return rc;
    }

    if (target == NULL) {

      if (event->type == CMP_INPUT_POINTER_DOWN) {

        dispatcher->focused = NULL;

        dispatcher->focus_visible = CMP_FALSE;
      }

      return CMP_OK;
    }

    handled = CMP_FALSE;

    rc = cmp_event_dispatch_to_widget(target, event, &handled);

    if (rc != CMP_OK) {

      return rc;
    }

    if (event->type == CMP_INPUT_POINTER_DOWN) {

      if (cmp_event_widget_focusable(target)) {

        dispatcher->focused = target;

      } else {

        dispatcher->focused = NULL;
      }

      dispatcher->focus_visible = CMP_FALSE;
    }

    if (out_target != NULL) {

      *out_target = target;
    }

    *out_handled = handled;

    return CMP_OK;

  case CMP_INPUT_KEY_DOWN:

  case CMP_INPUT_KEY_UP:

  case CMP_INPUT_TEXT:

  case CMP_INPUT_TEXT_UTF8:

  case CMP_INPUT_TEXT_EDIT:

    /* Attempt to dispatch to the currently focused widget first */

    if (dispatcher->focused != NULL) {

      handled = CMP_FALSE;

      rc = cmp_event_dispatch_to_widget(dispatcher->focused, event, &handled);

      if (rc != CMP_OK) {

        return rc;
      }

      if (handled) {

        dispatcher->focus_visible = CMP_TRUE;

        if (out_target != NULL) {

          *out_target = dispatcher->focused;
        }

        *out_handled = CMP_TRUE;

        return CMP_OK;
      }
    }

    /* If unhandled, check for Tab navigation on Key Down */

    if (event->type == CMP_INPUT_KEY_DOWN && event->data.key.key_code == 9u) {

      CMPFocusContext ctx;

      CMPWidget *next_focus = NULL;

      CMPBool reverse;

      ctx.target = dispatcher->focused;

      ctx.first = NULL;

      ctx.last = NULL;

      ctx.prev = NULL;

      ctx.next = NULL;

      ctx.last_visited = NULL;

      ctx.found_target = CMP_FALSE;

      cmp_event_traverse_focus(root, &ctx);

      reverse = (event->modifiers & CMP_MOD_SHIFT) ? CMP_TRUE : CMP_FALSE;

      /* Handle Case: No current focus, select first/last */

      if (ctx.target == NULL && ctx.first != NULL) {

        if (reverse) {

          next_focus = ctx.last;

        } else {

          next_focus = ctx.first;
        }

      }

      /* Handle Case: Navigation from current focus */

      else {

        if (reverse) {

          if (ctx.prev != NULL) {

            next_focus = ctx.prev;

          } else {

            next_focus = ctx.last; /* Wrap around to end */
          }

        } else {

          if (ctx.next != NULL) {

            next_focus = ctx.next;

          } else {

            next_focus = ctx.first; /* Wrap around to start */
          }
        }
      }

      if (next_focus != NULL) {

        dispatcher->focused = next_focus;

        dispatcher->focus_visible = CMP_TRUE;

        if (out_target != NULL) {

          *out_target = next_focus;
        }

        *out_handled = CMP_TRUE;
      }
    }

    return CMP_OK;

  case CMP_INPUT_WINDOW_RESIZE:

  case CMP_INPUT_WINDOW_CLOSE:

  case CMP_INPUT_WINDOW_FOCUS:

  case CMP_INPUT_WINDOW_BLUR:

    handled = CMP_FALSE;

    rc = cmp_event_dispatch_to_widget(root->widget, event, &handled);

    if (event->type == CMP_INPUT_WINDOW_BLUR) {

      dispatcher->focused = NULL;

      dispatcher->focus_visible = CMP_FALSE;
    }

    if (rc != CMP_OK) {

      return rc;
    }

    if (out_target != NULL) {

      *out_target = root->widget;
    }

    *out_handled = handled;

    return CMP_OK;

  default:

    return CMP_ERR_INVALID_ARGUMENT;
  }
}

#ifdef CMP_TESTING

int CMP_CALL cmp_event_test_validate_rect(const CMPRect *rect) {

  return cmp_event_validate_rect(rect);
}

int CMP_CALL cmp_event_test_widget_focusable(const CMPWidget *widget,

                                             CMPBool *out_focusable) {

  if (out_focusable == NULL) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_focusable = cmp_event_widget_focusable(widget);

  return CMP_OK;
}

int CMP_CALL cmp_event_test_hit_test(const CMPRenderNode *node, CMPScalar x,

                                     CMPScalar y, CMPWidget **out_widget) {

  return cmp_event_hit_test(node, x, y, out_widget);
}

int CMP_CALL cmp_event_test_dispatch_to_widget(CMPWidget *widget,

                                               const CMPInputEvent *event,

                                               CMPBool *out_handled) {

  return cmp_event_dispatch_to_widget(widget, event, out_handled);
}

int CMP_CALL cmp_event_test_set_force_contains_error(CMPBool enable) {

  if (enable != CMP_FALSE && enable != CMP_TRUE) {

    return CMP_ERR_INVALID_ARGUMENT;
  }

  g_cmp_event_force_contains_error = enable ? CMP_TRUE : CMP_FALSE;

  return CMP_OK;
}

#endif