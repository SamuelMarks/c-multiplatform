/**
 * @file ui_tooltip_base.c
 * @brief Implementation of the tooltip base component.
 */
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

#ifdef UI_TEST_MOCK_ALLOC
int g_tooltip_mock_fail = 0;

/**
 * @brief mock_tooltip_dom_node_destroy.
 * @param node Node.
 * @return Return value.
 */
static ui_error_t mock_tooltip_dom_node_destroy(struct ui_dom_node *node) {
  if (g_tooltip_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_destroy(node);
}
/** @cond */
#define ui_dom_node_destroy mock_tooltip_dom_node_destroy
/** @endcond */

/**
 * @brief mock_tooltip_overlay_director_mount_component.
 * @param director Director.
 * @param comp Component.
 * @param layer Layer.
 * @param out_overlay Output overlay.
 * @return Return value.
 */
static ui_error_t mock_tooltip_overlay_director_mount_component(
    struct ui_overlay_director *director, struct ui_component *comp, int layer,
    struct ui_overlay **out_overlay) {
  if (g_tooltip_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_overlay_director_mount_component(director, comp, layer,
                                             out_overlay);
}
/** @cond */
#define ui_overlay_director_mount_component                                    \
  mock_tooltip_overlay_director_mount_component
/** @endcond */

/**
 * @brief mock_tooltip_overlay_director_unmount.
 * @param director Director.
 * @param overlay Overlay.
 * @return Return value.
 */
static ui_error_t
mock_tooltip_overlay_director_unmount(struct ui_overlay_director *director,
                                      struct ui_overlay *overlay) {
  if (g_tooltip_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_overlay_director_unmount(director, overlay);
}
/** @cond */
#define ui_overlay_director_unmount mock_tooltip_overlay_director_unmount
/** @endcond */

/**
 * @brief mock_tooltip_component_destroy.
 * @param comp Component.
 * @return Return value.
 */
static ui_error_t mock_tooltip_component_destroy(struct ui_component *comp) {
  if (g_tooltip_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(comp);
}
/** @cond */
#define ui_component_destroy mock_tooltip_component_destroy
/** @endcond */

/**
 * @brief mock_tooltip_dom_node_append_child.
 * @param parent Parent node.
 * @param child Child node.
 * @return Return value.
 */
static ui_error_t
mock_tooltip_dom_node_append_child(struct ui_dom_node *parent,
                                   struct ui_dom_node *child) {
  if (g_tooltip_mock_fail == 6) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_append_child(parent, child);
}
/** @cond */
#define ui_dom_node_append_child mock_tooltip_dom_node_append_child
/** @endcond */

/**
 * @brief mock_tooltip_geometry_anchor_compute.
 * @param target Target.
 * @param overlay Overlay.
 * @param config Config.
 * @param viewport_width Viewport width.
 * @param viewport_height Viewport height.
 * @param out_x Out x.
 * @param out_y Out y.
 * @return Return value.
 */
static ui_error_t mock_tooltip_geometry_anchor_compute(
    const struct ui_layout_node *target, const struct ui_layout_node *overlay,
    const struct ui_anchor_config *config, float viewport_width,
    float viewport_height, float *out_x, float *out_y) {
  if (g_tooltip_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_geometry_anchor_compute(target, overlay, config, viewport_width,
                                    viewport_height, out_x, out_y);
}
/** @cond */
#define ui_geometry_anchor_compute mock_tooltip_geometry_anchor_compute
/** @endcond */
#endif

/**
 * @enum ui_tooltip_state
 * @brief Internal state machine for the tooltip.
 */
enum ui_tooltip_state {
  /** @brief Tooltip is hidden and inactive. */
  UI_TOOLTIP_STATE_IDLE,
  /** @brief Waiting for hover delay to expire. */
  UI_TOOLTIP_STATE_HOVER_DELAY,
  /** @brief Waiting for focus delay to expire. */
  UI_TOOLTIP_STATE_FOCUS_DELAY,
  /** @brief Waiting for touch-and-hold delay to expire. */
  UI_TOOLTIP_STATE_TOUCH_HOLD_DELAY,
  /** @brief Tooltip is fully visible. */
  UI_TOOLTIP_STATE_VISIBLE,
  /** @brief Waiting for hide delay to expire. */
  UI_TOOLTIP_STATE_HIDE_DELAY
};

/**
 * @struct ui_tooltip_base
 * @struct ui_tooltip_base
 * @brief Internal implementation of the tooltip component.
 */
struct ui_tooltip_base {
  /** @brief The tooltip configuration. */
  struct ui_tooltip_config config; /**< config */
  /** @brief The current state. */
  enum ui_tooltip_state state; /**< state */
  /** @brief Time when the current state was entered. */
  double state_enter_time; /**< state_enter_time */
  /** @brief The tooltip text. */
  char *text; /**< text */

  /** @brief The overlay component for rendering. */
  struct ui_component *overlay_component; /**< overlay_component */
  /** @brief The active overlay instance. */
  struct ui_overlay *active_overlay; /**< active_overlay */
  /** @brief Signal for the open state. */
  struct ui_signal *open_signal; /**< open_signal */
  /** @brief Signal for the animating state. */
  struct ui_computed *animating_signal; /**< animating_signal */
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
  ui_error_t rc = UI_ERROR_NONE;
  if (!tooltip)
    return UI_ERROR_NONE;
  if (tooltip->text)
    C_MULTIPLATFORM_FREE(tooltip->text);
  if (tooltip->overlay_component) {
    ui_error_t rc_cleanup = ui_component_destroy(tooltip->overlay_component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  /* Note: active_overlay lifecycle is managed by overlay_director unmount */
  C_MULTIPLATFORM_FREE(tooltip);
  return rc;
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

/**
 * @brief Helper function to transition the tooltip state machine.
 * @param tooltip The tooltip instance.
 * @param new_state The new state to transition to.
 * @param time_secs The current time in seconds.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t transition_state(struct ui_tooltip_base *tooltip,
                                   enum ui_tooltip_state new_state,
                                   double time_secs) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_tooltip_mock_fail == 10) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  tooltip->state = new_state;
  tooltip->state_enter_time = time_secs;
  return UI_ERROR_NONE;
}

ui_error_t ui_tooltip_base_handle_event(struct ui_tooltip_base *tooltip,
                                        const struct ui_event *event,
                                        double current_time_secs) {
  ui_error_t rc;

  if (!tooltip || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  switch (event->type) {
  case UI_EVENT_MOUSE_DOWN:
  case UI_EVENT_KEY_DOWN: /* Dismiss on key or click */
    if (tooltip->state != UI_TOOLTIP_STATE_IDLE) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_IDLE, current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_EVENT_MOUSE_MOVE:
    /* In a real system, we'd check intersection with the trigger rect.
       For this primitive, we assume the event router only sends us relevant
       events. */
    if (tooltip->state == UI_TOOLTIP_STATE_IDLE) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_HOVER_DELAY,
                            current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_EVENT_TOUCH_START:
    if (tooltip->state == UI_TOOLTIP_STATE_IDLE) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_TOUCH_HOLD_DELAY,
                            current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_EVENT_TOUCH_END:
  case UI_EVENT_TOUCH_CANCEL:
  case UI_EVENT_WINDOW_RESIZE:
    if (tooltip->state != UI_TOOLTIP_STATE_IDLE &&
        tooltip->state != UI_TOOLTIP_STATE_HIDE_DELAY) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_HIDE_DELAY,
                            current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  /* Treat focus (simulated here) as needing a delay */
  case UI_EVENT_PEN_DOWN: /* Re-using PEN_DOWN as focus for primitive mock */
    if (tooltip->state == UI_TOOLTIP_STATE_IDLE) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_FOCUS_DELAY,
                            current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_EVENT_PEN_UP: /* Blur */
    if (tooltip->state != UI_TOOLTIP_STATE_IDLE &&
        tooltip->state != UI_TOOLTIP_STATE_HIDE_DELAY) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_HIDE_DELAY,
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
  ui_error_t rc;

  if (!tooltip)
    return UI_ERROR_INVALID_ARGUMENT;

  elapsed = current_time_secs - tooltip->state_enter_time;

  switch (tooltip->state) {
  case UI_TOOLTIP_STATE_HOVER_DELAY:
    if (elapsed >= tooltip->config.hover_delay_secs) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_VISIBLE,
                            current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_TOOLTIP_STATE_FOCUS_DELAY:
    if (elapsed >= tooltip->config.focus_delay_secs) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_VISIBLE,
                            current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_TOOLTIP_STATE_TOUCH_HOLD_DELAY:
    if (elapsed >= tooltip->config.touch_hold_delay_secs) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_VISIBLE,
                            current_time_secs);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    break;

  case UI_TOOLTIP_STATE_HIDE_DELAY:
    if (elapsed >= tooltip->config.hide_delay_secs) {
      rc = transition_state(tooltip, UI_TOOLTIP_STATE_IDLE, current_time_secs);
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

#ifdef UI_TEST_MOCK_ALLOC
/**
 * @brief mock_tooltip_base_is_visible.
 * @param tooltip Tooltip.
 * @param out_is_visible Output pointer.
 * @return Return value.
 */
static ui_error_t
mock_tooltip_base_is_visible(const struct ui_tooltip_base *tooltip,
                             int *out_is_visible) {
  if (g_tooltip_mock_fail == 7) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_tooltip_base_is_visible(tooltip, out_is_visible);
}
/** @cond */
#define ui_tooltip_base_is_visible mock_tooltip_base_is_visible
/** @endcond */
#endif

ui_error_t ui_tooltip_base_render(struct ui_tooltip_base *tooltip,
                                  struct ui_overlay_director *director,
                                  const struct ui_layout_node *trigger_layout,
                                  const struct ui_anchor_config *anchor_config,
                                  float viewport_width, float viewport_height) {

  float x = 0.0f, y = 0.0f;
  struct ui_layout_node overlay_layout;
  char style_buf[256];
  struct ui_dom_node *root_node = NULL;
  struct ui_dom_node *text_node = NULL;
  int is_visible = 0;
  ui_error_t rc;
  ui_error_t rc_cleanup;

  if (!tooltip || !director || !trigger_layout || !anchor_config)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = ui_tooltip_base_is_visible(tooltip, &is_visible);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (!is_visible) {
    if (tooltip->active_overlay) {
      rc = ui_overlay_director_unmount(director, tooltip->active_overlay);
      tooltip->active_overlay = NULL;
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    return UI_ERROR_NONE;
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

  rc =
      ui_geometry_anchor_compute(trigger_layout, &overlay_layout, anchor_config,
                                 viewport_width, viewport_height, &x, &y);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Build component DOM */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_dom_node_set_attribute(root_node, "role", "tooltip");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root_node);
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(style_buf, sizeof(style_buf),
            "position: absolute; left: %fpx; top: %fpx; z-index: 9999;", x, y);
#else
  sprintf(style_buf,
          "position: absolute; left: %fpx; top: %fpx; z-index: 9999;", x, y);
#endif
  rc = ui_dom_node_set_attribute(root_node, "style", style_buf);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root_node);
    return rc;
  }

  if (tooltip->text) {
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (rc != UI_ERROR_NONE) {
      ui_dom_node_destroy(root_node);
      return rc;
    }

    {
      /* Direct member access for text content to simulate standard DOM text
       * node logic */
      size_t len = strlen(tooltip->text);
      text_node->text_content = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
      if (!text_node->text_content) {
        ui_dom_node_destroy(text_node);
        ui_dom_node_destroy(root_node);
        return UI_ERROR_OUT_OF_MEMORY;
      }
#if defined(_MSC_VER)
      strcpy_s(text_node->text_content, len + 1, tooltip->text);
#else
      strcpy(text_node->text_content, tooltip->text);
#endif
    }
    rc = ui_dom_node_append_child(root_node, text_node);
    if (rc != UI_ERROR_NONE) {
      ui_dom_node_destroy(text_node);
      ui_dom_node_destroy(root_node);
      return rc;
    }
  }

  if (tooltip->overlay_component->shadow_root) {
    rc_cleanup = ui_dom_node_destroy(tooltip->overlay_component->shadow_root);
    if (rc_cleanup != UI_ERROR_NONE) {
      ui_dom_node_destroy(root_node);
      return rc_cleanup;
    }
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

ui_error_t
ui_tooltip_base_get_animating_signal(struct ui_tooltip_base *widget,
                                     struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}
