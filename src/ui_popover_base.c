/**
 * \file ui_popover_base.c
 * \brief Implementation of the UI popover base component.
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

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/**
 * \struct ui_popover_base
 * \brief State and DOM mapping for a popover widget (menus, tooltips, dialogs).
 */
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

/**
 * \brief Creates a new popover base widget.
 * \param[out] out_popover Pointer to store the created widget.
 * \return UI_ERROR_NONE on success.
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
    (void)ui_component_destroy(popover->overlay_component);
    C_MULTIPLATFORM_FREE(popover);
    return rc;
  }

  *out_popover = popover;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a popover base widget.
 * \param[in,out] popover The widget to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_destroy(struct ui_popover_base *popover) {
  if (!popover)
    return UI_ERROR_NONE;

  if (popover->is_open) {
    ui_error_t rc = ui_popover_base_close(popover);
  }

  (void)ui_backdrop_destroy(popover->backdrop);
  (void)ui_component_destroy(popover->overlay_component);
  C_MULTIPLATFORM_FREE(popover);
  return UI_ERROR_NONE;
}

/**
 * \brief Opens the popover, anchoring it to a specific layout node.
 * \param[in,out] popover The popover widget.
 * \param[in] content The DOM node containing the popover's visual content.
 * \param[in,out] director The overlay director to mount the popover into.
 * \param[in,out] focus_mgr Optional focus manager to trap focus within the
 * popover.
 * \param[in] trigger_layout The layout node of the trigger element.
 * \param[in] anchor_config Configuration for anchoring the popover.
 * \param[in] viewport_width Width of the viewport.
 * \param[in] viewport_height Height of the viewport.
 * \return UI_ERROR_NONE on success.
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
  struct ui_layout_node overlay_layout;
  char style_buf[256];
  struct ui_dom_node *root_node = NULL;

  if (!popover || !content || !director || !trigger_layout || !anchor_config) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (popover->is_open) {
    rc = ui_popover_base_close(popover);
  }

  /* Compute anchor position.
     In a real layout pass, width/height would come from content layout.
     We use a mock size for computation here as typical in headless
     abstractions. */
  overlay_layout.x = 0;
  overlay_layout.y = 0;
  overlay_layout.width = 200.0f;
  overlay_layout.height = 150.0f;

  {

    ui_error_t _ign_rc = ui_geometry_anchor_compute(
        trigger_layout, &overlay_layout, anchor_config, viewport_width,
        viewport_height, &x, &y);

    (void)_ign_rc;
  }

  /* Cache coordinates for backdrop click-outside hit testing */
  popover->current_x = x;
  popover->current_y = y;
  popover->current_width = overlay_layout.width;
  popover->current_height = overlay_layout.height;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);

  /* Standard W3C mapping for popovers / dialogs */
  {
    ui_error_t attr_rc = ui_dom_node_set_attribute(root_node, "role", "dialog");
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }
  {
    ui_error_t attr_rc =
        ui_dom_node_set_attribute(root_node, "aria-modal", "true");
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }

#if defined(_MSC_VER)
  sprintf_s(style_buf, sizeof(style_buf),
            "position: absolute; left: %fpx; top: %fpx; z-index: 10000;", x, y);
#else
  sprintf(style_buf,
          "position: absolute; left: %fpx; top: %fpx; z-index: 10000;", x, y);
#endif
  {
    ui_error_t attr_rc =
        ui_dom_node_set_attribute(root_node, "style", style_buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }

  rc = ui_dom_node_append_child(root_node, content);
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    return rc;
  }

  popover->overlay_component->shadow_root = root_node;

  rc = ui_overlay_director_mount_component(director, popover->overlay_component,
                                           10000, &popover->active_overlay);
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rem_rc = ui_dom_node_remove_child(root_node, content);
    }
    popover->overlay_component->shadow_root = NULL;
    (void)ui_dom_node_destroy(root_node);
    return rc;
  }

  if (focus_mgr) {
    rc = ui_focus_manager_push_trap(focus_mgr, root_node);
    if (rc != UI_ERROR_NONE) {
      ui_error_t unmount_rc =
          ui_overlay_director_unmount(director, popover->active_overlay);
      popover->active_overlay = NULL;
      { ui_error_t rem_rc = ui_dom_node_remove_child(root_node, content); }
      popover->overlay_component->shadow_root = NULL;
      (void)ui_dom_node_destroy(root_node);
      return rc;
    }
    popover->active_focus_mgr = focus_mgr;
  }

  popover->active_director = director;
  popover->is_open = 1;

  return UI_ERROR_NONE;
}

/**
 * \brief Processes an incoming UI event (like clicks outside) for the popover.
 * \param[in,out] popover The popover widget.
 * \param[in] event The UI event to process.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_process_event(struct ui_popover_base *popover,
                                         const struct ui_event *event) {
  int should_dismiss = 0;

  if (!popover || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!popover->is_open)
    return UI_ERROR_NONE;

  {

    ui_error_t _ign_rc = ui_backdrop_process_event(
        popover->backdrop, event, popover->current_x, popover->current_y,
        popover->current_width, popover->current_height, &should_dismiss);

    (void)_ign_rc;
  }

  if (should_dismiss) {
    return ui_popover_base_close(popover);
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Closes the popover and cleans up its overlay/focus state.
 * \param[in,out] popover The popover widget.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_close(struct ui_popover_base *popover) {
  struct ui_dom_node *root;

  if (!popover)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!popover->is_open)
    return UI_ERROR_NONE;

  if (popover->active_focus_mgr) {
    {
      ui_error_t pop_rc = ui_focus_manager_pop_trap(popover->active_focus_mgr);
    }
    popover->active_focus_mgr = NULL;
  }

  {
    ui_error_t un_rc = ui_overlay_director_unmount(popover->active_director,
                                                   popover->active_overlay);
  }

  /* Unlink the content node to prevent its destruction */
  root = popover->overlay_component->shadow_root;
  {
    ui_error_t _ign_rc = ui_dom_node_remove_child(root, root->first_child);
    (void)_ign_rc;
  }
  (void)ui_dom_node_destroy(root);
  popover->overlay_component->shadow_root = NULL;

  popover->active_overlay = NULL;
  popover->active_director = NULL;
  popover->is_open = 0;

  return UI_ERROR_NONE;
}

/**
 * \brief Checks if the popover is currently open.
 * \param[in] popover The popover widget.
 * \param[out] out_is_open Set to 1 if open, 0 otherwise.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_is_open(const struct ui_popover_base *popover,
                                   int *out_is_open) {
  if (!popover || !out_is_open)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_is_open = popover->is_open;
  return UI_ERROR_NONE;
}

/**
 * \brief Binds the open state of the popover to a reactive signal.
 * \param[in,out] widget The popover widget.
 * \param[in,out] open_signal The signal representing the open state.
 * \return UI_ERROR_NONE on success.
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
 * \brief Gets the computed signal indicating if the popover is currently
 * animating.
 * \param[in,out] widget The popover widget.
 * \param[out] out_animating Pointer to store the computed signal.
 * \return UI_ERROR_NONE on success.
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
