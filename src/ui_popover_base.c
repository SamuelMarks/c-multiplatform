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

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

struct ui_popover_base {
  int is_open;

  struct ui_component *overlay_component;
  struct ui_overlay *active_overlay;
  struct ui_backdrop *backdrop;
  struct ui_focus_manager *active_focus_mgr;
  struct ui_overlay_director *active_director;

  float current_x;
  float current_y;
  float current_width;
  float current_height;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

enum ui_error ui_popover_base_create(struct ui_popover_base **out_popover) {
  struct ui_popover_base *popover;
  enum ui_error rc;

  if (!out_popover)
    return UI_ERROR_INVALID_ARGUMENT;

  popover = (struct ui_popover_base *)UI_MALLOC(sizeof(struct ui_popover_base));
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
    UI_FREE(popover);
    return rc;
  }

  rc = ui_backdrop_create(&popover->backdrop);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(popover->overlay_component);
    UI_FREE(popover);
    return rc;
  }

  *out_popover = popover;
  return UI_ERROR_NONE;
}

void ui_popover_base_destroy(struct ui_popover_base *popover) {
  if (!popover)
    return;

  if (popover->is_open) {
    ui_popover_base_close(popover);
  }

  ui_backdrop_destroy(popover->backdrop);
  ui_component_destroy(popover->overlay_component);
  UI_FREE(popover);
}

enum ui_error ui_popover_base_open(struct ui_popover_base *popover,
                                   struct ui_dom_node *content,
                                   struct ui_overlay_director *director,
                                   struct ui_focus_manager *focus_mgr,
                                   const struct ui_layout_node *trigger_layout,
                                   const struct ui_anchor_config *anchor_config,
                                   float viewport_width,
                                   float viewport_height) {

  float x = 0.0f, y = 0.0f;
  enum ui_error rc;
  struct ui_layout_node overlay_layout;
  char style_buf[256];
  struct ui_dom_node *root_node = NULL;

  if (!popover || !content || !director || !trigger_layout || !anchor_config) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (popover->is_open) {
    ui_popover_base_close(popover);
  }

  /* Compute anchor position.
     In a real layout pass, width/height would come from content layout.
     We use a mock size for computation here as typical in headless
     abstractions. */
  overlay_layout.x = 0;
  overlay_layout.y = 0;
  overlay_layout.width = 200.0f;
  overlay_layout.height = 150.0f;

  (void)ui_geometry_anchor_compute(trigger_layout, &overlay_layout,
                                   anchor_config, viewport_width,
                                   viewport_height, &x, &y);

  /* Cache coordinates for backdrop click-outside hit testing */
  popover->current_x = x;
  popover->current_y = y;
  popover->current_width = overlay_layout.width;
  popover->current_height = overlay_layout.height;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Standard W3C mapping for popovers / dialogs */
  ui_dom_node_set_attribute(root_node, "role", "dialog");
  ui_dom_node_set_attribute(root_node, "aria-modal", "true");

#if defined(_MSC_VER)
  sprintf_s(style_buf, sizeof(style_buf),
            "position: absolute; left: %fpx; top: %fpx; z-index: 10000;", x, y);
#else
  sprintf(style_buf,
          "position: absolute; left: %fpx; top: %fpx; z-index: 10000;", x, y);
#endif
  ui_dom_node_set_attribute(root_node, "style", style_buf);

  rc = ui_dom_node_append_child(root_node, content);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root_node);
    return rc;
  }

  popover->overlay_component->shadow_root = root_node;

  rc = ui_overlay_director_mount_component(director, popover->overlay_component,
                                           10000, &popover->active_overlay);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_remove_child(root_node, content);
    popover->overlay_component->shadow_root = NULL;
    ui_dom_node_destroy(root_node);
    return rc;
  }

  if (focus_mgr) {
    rc = ui_focus_manager_push_trap(focus_mgr, root_node);
    if (rc != UI_ERROR_NONE) {
      ui_overlay_director_unmount(director, popover->active_overlay);
      popover->active_overlay = NULL;
      ui_dom_node_remove_child(root_node, content);
      popover->overlay_component->shadow_root = NULL;
      ui_dom_node_destroy(root_node);
      return rc;
    }
    popover->active_focus_mgr = focus_mgr;
  }

  popover->active_director = director;
  popover->is_open = 1;

  return UI_ERROR_NONE;
}

enum ui_error ui_popover_base_process_event(struct ui_popover_base *popover,
                                            const struct ui_event *event) {
  int should_dismiss = 0;

  if (!popover || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!popover->is_open)
    return UI_ERROR_NONE;

  (void)ui_backdrop_process_event(popover->backdrop, event, popover->current_x,
                                  popover->current_y, popover->current_width,
                                  popover->current_height, &should_dismiss);

  if (should_dismiss) {
    return ui_popover_base_close(popover);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_popover_base_close(struct ui_popover_base *popover) {
  struct ui_dom_node *root;

  if (!popover)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!popover->is_open)
    return UI_ERROR_NONE;

  if (popover->active_focus_mgr) {
    ui_focus_manager_pop_trap(popover->active_focus_mgr);
    popover->active_focus_mgr = NULL;
  }

  ui_overlay_director_unmount(popover->active_director,
                              popover->active_overlay);

  /* Unlink the content node to prevent its destruction */
  root = popover->overlay_component->shadow_root;
  (void)ui_dom_node_remove_child(root, root->first_child);
  ui_dom_node_destroy(root);
  popover->overlay_component->shadow_root = NULL;

  popover->active_overlay = NULL;
  popover->active_director = NULL;
  popover->is_open = 0;

  return UI_ERROR_NONE;
}

enum ui_error ui_popover_base_is_open(const struct ui_popover_base *popover,
                                      int *out_is_open) {
  if (!popover || !out_is_open)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_is_open = popover->is_open;
  return UI_ERROR_NONE;
}

enum ui_error ui_popover_base_bind_open(struct ui_popover_base *widget,
                                        struct ui_signal *open_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_popover_base_get_animating_signal(struct ui_popover_base *widget,
                                     struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}
