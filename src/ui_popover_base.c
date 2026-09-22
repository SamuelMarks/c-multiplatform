/**
 * @file ui_popover_base.c
 * @brief Implementation of the UI popover base component.
 */

/* clang-format off */
#include "ui_popover_base.h"
#include "ui_internal_mem.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
#include "ui_backdrop.h"
#include "ui_focus_manager.h"
#include "ui_dom_node.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_popover_mock_fail = 0;
int g_popover_backdrop_destroy_mock_fail = 0;
int g_popover_component_destroy_mock_fail = 0;
int g_popover_dom_node_destroy_mock_fail = 0;
int g_popover_remove_child_mock_fail = 0;
int g_popover_unmount_mock_fail = 0;

/**
 * @brief Mock implementation of ui_component_destroy for error injection.
 * @param comp Component to destroy.
 * @return Error code.
 */
static ui_error_t mock_popover_component_destroy(struct ui_component *comp) {
  if (g_popover_component_destroy_mock_fail > 0) {
    g_popover_component_destroy_mock_fail--;
    (ui_component_destroy)(comp);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_popover_component_destroy
/** @endcond */

/**
 * @brief Mock implementation of ui_backdrop_destroy for error injection.
 * @param backdrop Backdrop to destroy.
 * @return Error code.
 */
static ui_error_t mock_popover_backdrop_destroy(struct ui_backdrop *backdrop) {
  if (g_popover_backdrop_destroy_mock_fail > 0) {
    g_popover_backdrop_destroy_mock_fail--;
    (ui_backdrop_destroy)(backdrop);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_backdrop_destroy)(backdrop);
}
#undef ui_backdrop_destroy
/** @cond */
#define ui_backdrop_destroy mock_popover_backdrop_destroy
/** @endcond */

/**
 * @brief Mock implementation of ui_dom_node_destroy for error injection.
 * @param node Node to destroy.
 * @return Error code.
 */
static ui_error_t mock_popover_dom_node_destroy(struct ui_dom_node *node) {
  if (g_popover_dom_node_destroy_mock_fail > 0) {
    g_popover_dom_node_destroy_mock_fail--;
    (ui_dom_node_destroy)(node);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_destroy)(node);
}
#undef ui_dom_node_destroy
/** @cond */
#define ui_dom_node_destroy mock_popover_dom_node_destroy
/** @endcond */

/**
 * @brief Mock implementation of ui_backdrop_create for error injection.
 * @param out_backdrop Output backdrop pointer.
 * @return Error code.
 */
static ui_error_t
mock_popover_backdrop_create(struct ui_backdrop **out_backdrop) {
  if (g_popover_mock_fail == 1) {
    g_popover_mock_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_backdrop_create)(out_backdrop);
}
#undef ui_backdrop_create
/** @cond */
#define ui_backdrop_create mock_popover_backdrop_create
/** @endcond */

/**
 * @brief Mock implementation of ui_geometry_anchor_compute for error injection.
 * @param target Target layout node.
 * @param overlay Overlay layout node.
 * @param config Anchor configuration.
 * @param viewport_width Viewport width.
 * @param viewport_height Viewport height.
 * @param out_x Output x coordinate.
 * @param out_y Output y coordinate.
 * @return Error code.
 */
static ui_error_t mock_popover_geometry_anchor_compute(
    const struct ui_layout_node *target, const struct ui_layout_node *overlay,
    const struct ui_anchor_config *config, float viewport_width,
    float viewport_height, float *out_x, float *out_y) {
  if (g_popover_mock_fail == 2) {
    g_popover_mock_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_geometry_anchor_compute)(target, overlay, config, viewport_width,
                                      viewport_height, out_x, out_y);
}
#undef ui_geometry_anchor_compute
/** @cond */
#define ui_geometry_anchor_compute mock_popover_geometry_anchor_compute
/** @endcond */

/**
 * @brief Mock implementation of ui_dom_node_set_attribute for error injection.
 * @param node Target node.
 * @param name Attribute name.
 * @param value Attribute value.
 * @return Error code.
 */
static ui_error_t mock_popover_dom_node_set_attribute(struct ui_dom_node *node,
                                                      const char *name,
                                                      const char *value) {
  if (g_popover_mock_fail == 3) {
    g_popover_mock_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  if (g_popover_mock_fail == 4) {
    g_popover_mock_fail = 50;
  } else if (g_popover_mock_fail == 50) {
    g_popover_mock_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  if (g_popover_mock_fail == 5) {
    g_popover_mock_fail = 51;
  } else if (g_popover_mock_fail == 51) {
    g_popover_mock_fail = 52;
  } else if (g_popover_mock_fail == 52) {
    g_popover_mock_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_set_attribute)(node, name, value);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_popover_dom_node_set_attribute
/** @endcond */

/**
 * @brief Mock implementation of ui_overlay_director_mount_component for error
 * injection.
 * @param director Director.
 * @param component Component.
 * @param z_index Z-index.
 * @param out_overlay Output overlay.
 * @return Error code.
 */
static ui_error_t mock_popover_overlay_director_mount_component(
    struct ui_overlay_director *director, struct ui_component *component,
    int z_index, struct ui_overlay **out_overlay) {
  if (g_popover_mock_fail == 6) {
    g_popover_mock_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_overlay_director_mount_component)(director, component, z_index,
                                               out_overlay);
}
#undef ui_overlay_director_mount_component
/** @cond */
#define ui_overlay_director_mount_component                                    \
  mock_popover_overlay_director_mount_component
/** @endcond */

/**
 * @brief Mock implementation of ui_focus_manager_push_trap for error injection.
 * @param mgr Focus manager.
 * @param root Root node.
 * @return Error code.
 */
static ui_error_t
mock_popover_focus_manager_push_trap(struct ui_focus_manager *mgr,
                                     struct ui_dom_node *root) {
  if (g_popover_mock_fail == 7) {
    g_popover_mock_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_focus_manager_push_trap)(mgr, root);
}
#undef ui_focus_manager_push_trap
/** @cond */
#define ui_focus_manager_push_trap mock_popover_focus_manager_push_trap
/** @endcond */

/**
 * @brief Mock implementation of ui_backdrop_process_event for error injection.
 * @param backdrop Backdrop.
 * @param event Event.
 * @param x Current x.
 * @param y Current y.
 * @param width Current width.
 * @param height Current height.
 * @param out_should_dismiss Output dismiss flag.
 * @return Error code.
 */
static ui_error_t mock_popover_backdrop_process_event(
    struct ui_backdrop *backdrop, const struct ui_event *event, float x,
    float y, float width, float height, int *out_should_dismiss) {
  if (g_popover_mock_fail == 8) {
    g_popover_mock_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_backdrop_process_event)(backdrop, event, x, y, width, height,
                                     out_should_dismiss);
}
#undef ui_backdrop_process_event
/** @cond */
#define ui_backdrop_process_event mock_popover_backdrop_process_event
/** @endcond */

/**
 * @brief Mock implementation of ui_focus_manager_pop_trap for error injection.
 * @param mgr Focus manager.
 * @return Error code.
 */
static ui_error_t
mock_popover_focus_manager_pop_trap(struct ui_focus_manager *mgr) {
  if (g_popover_mock_fail == 9) {
    g_popover_mock_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_focus_manager_pop_trap)(mgr);
}
#undef ui_focus_manager_pop_trap
/** @cond */
#define ui_focus_manager_pop_trap mock_popover_focus_manager_pop_trap
/** @endcond */

/**
 * @brief Mock implementation of ui_overlay_director_unmount for error
 * injection.
 * @param director Director.
 * @param overlay Overlay.
 * @return Error code.
 */
static ui_error_t
mock_popover_overlay_director_unmount(struct ui_overlay_director *director,
                                      struct ui_overlay *overlay) {
  if (g_popover_unmount_mock_fail > 0) {
    g_popover_unmount_mock_fail--;
    (ui_overlay_director_unmount)(director, overlay);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_overlay_director_unmount)(director, overlay);
}
#undef ui_overlay_director_unmount
/** @cond */
#define ui_overlay_director_unmount mock_popover_overlay_director_unmount
/** @endcond */

/**
 * @brief Mock implementation of ui_dom_node_remove_child for error injection.
 * @param parent Parent node.
 * @param child Child node.
 * @return Error code.
 */
static ui_error_t
mock_popover_dom_node_remove_child(struct ui_dom_node *parent,
                                   struct ui_dom_node *child) {
  if (g_popover_remove_child_mock_fail > 0) {
    g_popover_remove_child_mock_fail--;
    (ui_dom_node_remove_child)(parent, child);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_remove_child)(parent, child);
}
#undef ui_dom_node_remove_child
/** @cond */
#define ui_dom_node_remove_child mock_popover_dom_node_remove_child
/** @endcond */
#endif

/**
 * @struct ui_popover_base
 * @brief State and DOM mapping for a popover widget (menus, tooltips, dialogs).
 */
struct ui_popover_base {
  int is_open; /**< Non-zero if the popover is currently open. */

  struct ui_component *overlay_component; /**< Overlay component. */
  struct ui_overlay *active_overlay;      /**< Active overlay instance. */
  struct ui_backdrop *backdrop; /**< Backdrop for click-outside detection. */
  struct ui_focus_manager
      *active_focus_mgr; /**< The focus manager for trapping focus. */
  struct ui_overlay_director
      *active_director; /**< The overlay director managing the popover. */

  float current_x;      /**< X coordinate of the popover. */
  float current_y;      /**< Y coordinate of the popover. */
  float current_width;  /**< Width of the popover. */
  float current_height; /**< Height of the popover. */
  struct ui_signal
      *open_signal; /**< Bound signal indicating if the popover is open. */
  struct ui_computed
      *animating_signal; /**< Computed signal tracking animation state. */
};

/**
 * @brief Creates a new popover base widget.
 * @param[out] out_popover Pointer to store the created widget.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_create(struct ui_popover_base **out_popover) {
  struct ui_popover_base *popover;
  ui_error_t rc;

  if (!out_popover)
    return UI_ERROR_INVALID_ARGUMENT;

  popover = (struct ui_popover_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_popover_base));
  if (!popover)
    return UI_ERROR_OUT_OF_MEMORY;

  popover->is_open = 0;
  popover->active_overlay = NULL;
  popover->active_focus_mgr = NULL;
  popover->active_director = NULL;
  popover->current_x = 0.0f;
  popover->current_y = 0.0f;
  popover->current_width = 0.0f;
  popover->current_height = 0.0f;

  rc = ui_component_create(&popover->overlay_component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(popover);
    return rc;
  }

  rc = ui_backdrop_create(&popover->backdrop);
  if (rc != UI_ERROR_NONE) {
    ui_error_t rc_cleanup = ui_component_destroy(popover->overlay_component);
    if (rc_cleanup != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(popover);
      return rc_cleanup;
    }
    C_MULTIPLATFORM_FREE(popover);
    return rc;
  }

  *out_popover = popover;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a popover base widget.
 * @param[in,out] popover The widget to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_destroy(struct ui_popover_base *popover) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t rc_cleanup;

  if (!popover)
    return UI_ERROR_NONE;

  if (popover->is_open) {
    rc_cleanup = ui_popover_base_close(popover);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }

  rc_cleanup = ui_backdrop_destroy(popover->backdrop);
  if (rc_cleanup != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
    rc = rc_cleanup;
  }

  rc_cleanup = ui_component_destroy(popover->overlay_component);
  if (rc_cleanup != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
    rc = rc_cleanup;
  }

  C_MULTIPLATFORM_FREE(popover);
  return rc;
}

/**
 * @brief Opens the popover, anchoring it to a specific layout node.
 * @param[in,out] popover The popover widget.
 * @param[in] content The DOM node containing the popover's visual content.
 * @param[in,out] director The overlay director to mount the popover into.
 * @param[in,out] focus_mgr Optional focus manager to trap focus within the
 * popover.
 * @param[in] trigger_layout The layout node of the trigger element.
 * @param[in] anchor_config Configuration for anchoring the popover.
 * @param[in] viewport_width Width of the viewport.
 * @param[in] viewport_height Height of the viewport.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_open(struct ui_popover_base *popover,
                                struct ui_dom_node *content,
                                struct ui_overlay_director *director,
                                struct ui_focus_manager *focus_mgr,
                                const struct ui_layout_node *trigger_layout,
                                const struct ui_anchor_config *anchor_config,
                                float viewport_width, float viewport_height) {

  float x = 0.0f, y = 0.0f;
  ui_error_t rc;
  ui_error_t rc_cleanup;
  struct ui_layout_node overlay_layout;
  char style_buf[256];
  struct ui_dom_node *root_node = NULL;

  if (!popover || !content || !director || !trigger_layout || !anchor_config) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (popover->is_open) {
    rc = ui_popover_base_close(popover);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  overlay_layout.x = 0;
  overlay_layout.y = 0;
  overlay_layout.width = 200.0f;
  overlay_layout.height = 150.0f;

  rc =
      ui_geometry_anchor_compute(trigger_layout, &overlay_layout, anchor_config,
                                 viewport_width, viewport_height, &x, &y);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* Cache coordinates for backdrop click-outside hit testing */
  popover->current_x = x;
  popover->current_y = y;
  popover->current_width = overlay_layout.width;
  popover->current_height = overlay_layout.height;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Standard W3C mapping for popovers / dialogs */
  rc = ui_dom_node_set_attribute(root_node, "role", "dialog");
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
    return rc;
  }

  rc = ui_dom_node_set_attribute(root_node, "aria-modal", "true");
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
    return rc;
  }

#if defined(_MSC_VER)
  sprintf_s(style_buf, sizeof(style_buf),
            "position: absolute; left: %fpx; top: %fpx; z-index: 10000;", x, y);
#else
  sprintf(style_buf,
          "position: absolute; left: %fpx; top: %fpx; z-index: 10000;", x, y);
#endif
  rc = ui_dom_node_set_attribute(root_node, "style", style_buf);
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
    return rc;
  }

  rc = ui_dom_node_append_child(root_node, content);
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
    return rc;
  }

  popover->overlay_component->shadow_root = root_node;

  rc = ui_overlay_director_mount_component(director, popover->overlay_component,
                                           10000, &popover->active_overlay);
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = ui_dom_node_remove_child(root_node, content);
    if (rc_cleanup != UI_ERROR_NONE) {
      popover->overlay_component->shadow_root = NULL;
      ui_dom_node_destroy(root_node);
      return rc_cleanup;
    }
    popover->overlay_component->shadow_root = NULL;
    rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
    return rc;
  }

  if (focus_mgr) {
    rc = ui_focus_manager_push_trap(focus_mgr, root_node);
    if (rc != UI_ERROR_NONE) {
      rc_cleanup =
          ui_overlay_director_unmount(director, popover->active_overlay);
      if (rc_cleanup != UI_ERROR_NONE) {
        popover->active_overlay = NULL;
        ui_dom_node_remove_child(root_node, content);
        popover->overlay_component->shadow_root = NULL;
        ui_dom_node_destroy(root_node);
        return rc_cleanup;
      }
      popover->active_overlay = NULL;
      rc_cleanup = ui_dom_node_remove_child(root_node, content);
      if (rc_cleanup != UI_ERROR_NONE) {
        popover->overlay_component->shadow_root = NULL;
        ui_dom_node_destroy(root_node);
        return rc_cleanup;
      }
      popover->overlay_component->shadow_root = NULL;
      rc_cleanup = ui_dom_node_destroy(root_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        return rc_cleanup;
      }
      return rc;
    }
    popover->active_focus_mgr = focus_mgr;
  }

  popover->active_director = director;
  popover->is_open = 1;

  return UI_ERROR_NONE;
}

/**
 * @brief Processes an incoming UI event (like clicks outside) for the popover.
 * @param[in,out] popover The popover widget.
 * @param[in] event The UI event to process.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_process_event(struct ui_popover_base *popover,
                                         const struct ui_event *event) {
  int should_dismiss = 0;
  ui_error_t rc;

  if (!popover || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!popover->is_open)
    return UI_ERROR_NONE;

  rc = ui_backdrop_process_event(popover->backdrop, event, popover->current_x,
                                 popover->current_y, popover->current_width,
                                 popover->current_height, &should_dismiss);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (should_dismiss) {
    return ui_popover_base_close(popover);
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Closes the popover and cleans up its overlay/focus state.
 * @param[in,out] popover The popover widget.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_close(struct ui_popover_base *popover) {
  struct ui_dom_node *root;
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t rc_cleanup;

  if (!popover)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!popover->is_open)
    return UI_ERROR_NONE;

  if (popover->active_focus_mgr) {
    rc_cleanup = ui_focus_manager_pop_trap(popover->active_focus_mgr);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
    popover->active_focus_mgr = NULL;
  }

  rc_cleanup = ui_overlay_director_unmount(popover->active_director,
                                           popover->active_overlay);
  if (rc_cleanup != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
    rc = rc_cleanup;
  }

  /* Unlink the content node to prevent its destruction */
  root = popover->overlay_component->shadow_root;
  if (root) {
    if (root->first_child) {
      rc_cleanup = ui_dom_node_remove_child(root, root->first_child);
      if (rc_cleanup != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
        rc = rc_cleanup;
      }
    }
    rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
    popover->overlay_component->shadow_root = NULL;
  }

  popover->active_overlay = NULL;
  popover->active_director = NULL;
  popover->is_open = 0;

  return rc;
}

/**
 * @brief Checks if the popover is currently open.
 * @param[in] popover The popover widget.
 * @param[out] out_is_open Set to 1 if open, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_is_open(const struct ui_popover_base *popover,
                                   int *out_is_open) {
  if (!popover || !out_is_open)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_is_open = popover->is_open;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the open state of the popover to a reactive signal.
 * @param[in,out] widget The popover widget.
 * @param[in,out] open_signal The signal representing the open state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_bind_open(struct ui_popover_base *widget,
                                     struct ui_signal *open_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the computed signal indicating if the popover is currently
 * animating.
 * @param[in,out] widget The popover widget.
 * @param[out] out_animating Pointer to store the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_popover_base_get_animating_signal(struct ui_popover_base *widget,
                                     struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
void ui_popover_base_test_clear_shadow_root(struct ui_popover_base *popover);
/**
 * @brief Test helper to clear shadow_root for coverage testing.
 * @param popover Target popover.
 */
void ui_popover_base_test_clear_shadow_root(struct ui_popover_base *popover) {
  popover->overlay_component->shadow_root = NULL;
}
#endif
