/* clang-format off */
#include "ui_tooltip_base.h"
#include "ui_internal_mem.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
#include <string.h>
#include <stdio.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

enum ui_tooltip_state {
  UI_TOOLTIP_STATE_IDLE,
  UI_TOOLTIP_STATE_HOVER_DELAY,
  UI_TOOLTIP_STATE_FOCUS_DELAY,
  UI_TOOLTIP_STATE_TOUCH_HOLD_DELAY,
  UI_TOOLTIP_STATE_VISIBLE,
  UI_TOOLTIP_STATE_HIDE_DELAY
};

/** \brief ui_tooltip_base */
struct ui_tooltip_base {
  struct ui_tooltip_config config;
  enum ui_tooltip_state state;
  double state_enter_time;
  char *text;

  struct ui_component *overlay_component;
  struct ui_overlay *active_overlay;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

ui_error_t ui_tooltip_base_create(struct ui_tooltip_base **out_tooltip,
                                  const struct ui_tooltip_config *config) {
  struct ui_tooltip_base *tooltip;
  ui_error_t rc;

  if (!out_tooltip || !config)
    return UI_ERROR_INVALID_ARGUMENT;

  tooltip = (struct ui_tooltip_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_tooltip_base));
  if (!tooltip)
    return UI_ERROR_OUT_OF_MEMORY;

  tooltip->config = *config;
  tooltip->state = UI_TOOLTIP_STATE_IDLE;
  tooltip->state_enter_time = 0.0;
  tooltip->text = NULL;
  tooltip->active_overlay = NULL;

  rc = ui_component_create(&tooltip->overlay_component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tooltip);
    return rc;
  }

  *out_tooltip = tooltip;
  return UI_ERROR_NONE;
}

ui_error_t ui_tooltip_base_destroy(struct ui_tooltip_base *tooltip) {
  if (!tooltip)
    return UI_ERROR_NONE;
  if (tooltip->text)
    C_MULTIPLATFORM_FREE(tooltip->text);
  if (tooltip->overlay_component)
    (void)ui_component_destroy(tooltip->overlay_component);
  /* Note: active_overlay lifecycle is managed by overlay_director unmount */
  C_MULTIPLATFORM_FREE(tooltip);
  return UI_ERROR_NONE;
}

ui_error_t ui_tooltip_base_set_text(struct ui_tooltip_base *tooltip,
                                    const char *text) {
  size_t len;
  if (!tooltip)
    return UI_ERROR_INVALID_ARGUMENT;

  if (tooltip->text) {
    C_MULTIPLATFORM_FREE(tooltip->text);
    tooltip->text = NULL;
  }

  if (text) {
    len = strlen(text);
    tooltip->text = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
    if (!tooltip->text)
      return UI_ERROR_OUT_OF_MEMORY;
#if defined(_MSC_VER)
    strcpy_s(tooltip->text, len + 1, text);
#else
    strcpy(tooltip->text, text);
#endif
  }
  return UI_ERROR_NONE;
}

static ui_error_t transition_state(struct ui_tooltip_base *tooltip,
                                   enum ui_tooltip_state new_state,
                                   double time_secs) {
  tooltip->state = new_state;
  tooltip->state_enter_time = time_secs;
  return UI_ERROR_NONE;
}

ui_error_t ui_tooltip_base_handle_event(struct ui_tooltip_base *tooltip,
                                        const struct ui_event *event,
                                        double current_time_secs) {
  if (!tooltip || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  switch (event->type) {
  case UI_EVENT_MOUSE_DOWN:
  case UI_EVENT_KEY_DOWN: /* Dismiss on key or click */
    if (tooltip->state != UI_TOOLTIP_STATE_IDLE) {
      ui_error_t rc =
          transition_state(tooltip, UI_TOOLTIP_STATE_IDLE, current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_EVENT_MOUSE_MOVE:
    /* In a real system, we'd check intersection with the trigger rect.
       For this primitive, we assume the event router only sends us relevant
       events. */
    if (tooltip->state == UI_TOOLTIP_STATE_IDLE) {
      ui_error_t rc = transition_state(tooltip, UI_TOOLTIP_STATE_HOVER_DELAY,
                                       current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_EVENT_TOUCH_START:
    if (tooltip->state == UI_TOOLTIP_STATE_IDLE) {
      ui_error_t rc = transition_state(
          tooltip, UI_TOOLTIP_STATE_TOUCH_HOLD_DELAY, current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_EVENT_TOUCH_END:
  case UI_EVENT_TOUCH_CANCEL:
  case UI_EVENT_WINDOW_RESIZE:
    if (tooltip->state != UI_TOOLTIP_STATE_IDLE &&
        tooltip->state != UI_TOOLTIP_STATE_HIDE_DELAY) {
      ui_error_t rc = transition_state(tooltip, UI_TOOLTIP_STATE_HIDE_DELAY,
                                       current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  /* Treat focus (simulated here) as needing a delay */
  case UI_EVENT_PEN_DOWN: /* Re-using PEN_DOWN as focus for primitive mock */
    if (tooltip->state == UI_TOOLTIP_STATE_IDLE) {
      ui_error_t rc = transition_state(tooltip, UI_TOOLTIP_STATE_FOCUS_DELAY,
                                       current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_EVENT_PEN_UP: /* Blur */
    if (tooltip->state != UI_TOOLTIP_STATE_IDLE &&
        tooltip->state != UI_TOOLTIP_STATE_HIDE_DELAY) {
      ui_error_t rc = transition_state(tooltip, UI_TOOLTIP_STATE_HIDE_DELAY,
                                       current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  default:
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tooltip_base_tick(struct ui_tooltip_base *tooltip,
                                double current_time_secs) {
  double elapsed;
  if (!tooltip)
    return UI_ERROR_INVALID_ARGUMENT;

  elapsed = current_time_secs - tooltip->state_enter_time;

  switch (tooltip->state) {
  case UI_TOOLTIP_STATE_HOVER_DELAY:
    if (elapsed >= tooltip->config.hover_delay_secs) {
      ui_error_t rc = transition_state(tooltip, UI_TOOLTIP_STATE_VISIBLE,
                                       current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_TOOLTIP_STATE_FOCUS_DELAY:
    if (elapsed >= tooltip->config.focus_delay_secs) {
      ui_error_t rc = transition_state(tooltip, UI_TOOLTIP_STATE_VISIBLE,
                                       current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_TOOLTIP_STATE_TOUCH_HOLD_DELAY:
    if (elapsed >= tooltip->config.touch_hold_delay_secs) {
      ui_error_t rc = transition_state(tooltip, UI_TOOLTIP_STATE_VISIBLE,
                                       current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_TOOLTIP_STATE_HIDE_DELAY:
    if (elapsed >= tooltip->config.hide_delay_secs) {
      ui_error_t rc =
          transition_state(tooltip, UI_TOOLTIP_STATE_IDLE, current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  default:
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tooltip_base_is_visible(const struct ui_tooltip_base *tooltip,
                                      int *out_is_visible) {
  if (!tooltip || !out_is_visible)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_is_visible = tooltip->state == UI_TOOLTIP_STATE_VISIBLE ||
                    tooltip->state == UI_TOOLTIP_STATE_HIDE_DELAY;
  return UI_ERROR_NONE;
}

ui_error_t ui_tooltip_base_hide(struct ui_tooltip_base *tooltip) {
  if (!tooltip)
    return UI_ERROR_INVALID_ARGUMENT;
  /* Hard hide immediately */
  tooltip->state = UI_TOOLTIP_STATE_IDLE;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_tooltip_base_render(struct ui_tooltip_base *tooltip,
                                  struct ui_overlay_director *director,
                                  const struct ui_layout_node *trigger_layout,
                                  const struct ui_anchor_config *anchor_config,
                                  float viewport_width, float viewport_height) {

  float x = 0.0f, y = 0.0f;
  ui_error_t rc;
  struct ui_layout_node overlay_layout;
  char style_buf[256];
  struct ui_dom_node *root_node;
  struct ui_dom_node *text_node;

  if (!tooltip || !director || !trigger_layout || !anchor_config)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    int is_visible = 0;
    rc = ui_tooltip_base_is_visible(tooltip, &is_visible);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (!is_visible) {
      if (tooltip->active_overlay) {
        rc = ui_overlay_director_unmount(director, tooltip->active_overlay);
        if (rc != UI_ERROR_NONE)
          return rc;
        tooltip->active_overlay = NULL;
      }
      return UI_ERROR_NONE;
    }
  }

  if (tooltip->active_overlay) {
    /* Already rendered, maybe update position */
    return UI_ERROR_NONE;
  }

  /* Construct overlay layout node (mock sizes for computation) */
  overlay_layout.x = 0;
  overlay_layout.y = 0;
  overlay_layout.width = 100.0f; /* Approximated width for collision math */
  overlay_layout.height = 30.0f;

  {

    ui_error_t _ign_rc = ui_geometry_anchor_compute(
        trigger_layout, &overlay_layout, anchor_config, viewport_width,
        viewport_height, &x, &y);

    (void)_ign_rc;
  }

  /* Build component DOM */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_dom_node_set_attribute(root_node, "role", "tooltip");
  if (rc != UI_ERROR_NONE)
    return rc;

#if defined(_MSC_VER)
  sprintf_s(style_buf, sizeof(style_buf),
            "position: absolute; left: %fpx; top: %fpx; z-index: 9999;", x, y);
#else
  sprintf(style_buf,
          "position: absolute; left: %fpx; top: %fpx; z-index: 9999;", x, y);
#endif
  rc = ui_dom_node_set_attribute(root_node, "style", style_buf);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (tooltip->text) {
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (rc != UI_ERROR_NONE)
      return rc;

    /* Direct member access for text content to simulate standard DOM text
     * node logic */
    {
      size_t len = strlen(tooltip->text);
      text_node->text_content = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
      if (text_node->text_content) {
#if defined(_MSC_VER)
        strcpy_s(text_node->text_content, len + 1, tooltip->text);
#else
        strcpy(text_node->text_content, tooltip->text);
#endif
      }
    }
    rc = ui_dom_node_append_child(root_node, text_node);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (tooltip->overlay_component->shadow_root) {
    rc = ui_dom_node_destroy(tooltip->overlay_component->shadow_root);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  tooltip->overlay_component->shadow_root = root_node;

  /* Mount to director */
  rc = ui_overlay_director_mount_component(director, tooltip->overlay_component,
                                           9999, &tooltip->active_overlay);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tooltip_base_bind_open(struct ui_tooltip_base *widget,
                                     struct ui_signal *open_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_tooltip_base_get_animating_signal(struct ui_tooltip_base *widget,
                                     struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}
