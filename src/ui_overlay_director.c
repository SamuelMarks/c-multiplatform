/**
 * @file ui_overlay_director.c
 * @brief Implementation of the UI overlay director.
 * @details Manages modals, popovers, and floating menus mounted onto a root
 * overlay layer.
 */
/* clang-format off */
#include "ui_overlay_director.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @struct ui_overlay
 * @brief Internal record of an active overlay mount.
 */
struct ui_overlay {
  struct ui_component *component;   /**< The mounted UI component */
  struct ui_dom_node *wrapper_node; /**< Intermediate wrapper node */
  int z_index;                      /**< Stored Z-index level */
  struct ui_overlay *next;          /**< Next overlay in linked list */
};

/**
 * @struct ui_overlay_director
 * @brief Core engine managing multiple mounted overlays.
 */
struct ui_overlay_director {
  struct ui_dom_node *root_node;    /**< Target DOM node to mount over */
  struct ui_overlay *first_overlay; /**< Head of active overlays list */
};

/**
 * @brief Creates a new overlay director bound to a specific root DOM node.
 * @param[in,out] root_node The DOM node where overlays will be mounted.
 * @param[out] out_director Pointer to store the created director.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_overlay_director_create(struct ui_dom_node *root_node,
                           struct ui_overlay_director **out_director) {
  struct ui_overlay_director *d;

  if (!root_node || !out_director) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  d = (struct ui_overlay_director *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_overlay_director));
  if (!d) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  d->root_node = root_node;
  d->first_overlay = NULL;

  *out_director = d;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys an overlay director and unmounts all active overlays.
 * @param[in,out] director The director to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_overlay_director_destroy(struct ui_overlay_director *director) {
  struct ui_overlay *current;
  struct ui_overlay *next;

  if (!director) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = director->first_overlay;
  while (current) {
    ui_error_t unmount_rc;
    next = current->next;
    /* Unmounting automatically cleans up the wrapper_node and detaches from
     * root */
    unmount_rc = ui_overlay_director_unmount(director, current);
    {
      (void)unmount_rc;
    }
    current = next;
  }

  C_MULTIPLATFORM_FREE(director);
  return UI_ERROR_NONE;
}

/**
 * @brief Mounts a UI component as an overlay on top of the current view.
 * @param[in,out] director The overlay director.
 * @param[in,out] component The component to mount.
 * @param[in] z_index The CSS z-index for the overlay.
 * @param[out] out_overlay Pointer to store the resulting overlay handle.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_overlay_director_mount_component(struct ui_overlay_director *director,
                                    struct ui_component *component, int z_index,
                                    struct ui_overlay **out_overlay) {
  struct ui_overlay *overlay = NULL;
  struct ui_dom_node *wrapper = NULL;
  ui_error_t err;
  char style_buf[128];

  if (!director || !component || !out_overlay) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  overlay =
      (struct ui_overlay *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_overlay));
  if (!overlay) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &wrapper);
  if (err != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(overlay);
    return err;
  }

  err = ui_dom_node_set_tag_name(wrapper, "div");
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(wrapper);
    C_MULTIPLATFORM_FREE(overlay);
    return err;
  }

#if defined(_MSC_VER)
  sprintf_s(style_buf, sizeof(style_buf), "position: absolute; z-index: %d;",
            z_index);
#else
  sprintf(style_buf, "position: absolute; z-index: %d;", z_index);
#endif

  err = ui_dom_node_set_attribute(wrapper, "style", style_buf);
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(wrapper);
    C_MULTIPLATFORM_FREE(overlay);
    return err;
  }

  /* Set data-overlay attribute for debugging/identification */
  err = ui_dom_node_set_attribute(wrapper, "data-overlay", "true");
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(wrapper);
    C_MULTIPLATFORM_FREE(overlay);
    return err;
  }

  (void)ui_dom_node_append_child(director->root_node, wrapper);
  (void)ui_component_mount(component, wrapper);

  overlay->component = component;
  overlay->wrapper_node = wrapper;
  overlay->z_index = z_index;
  overlay->next = director->first_overlay;
  director->first_overlay = overlay;

  *out_overlay = overlay;
  return UI_ERROR_NONE;
}

/**
 * @brief Unmounts a specific overlay.
 * @param[in,out] director The overlay director.
 * @param[in,out] overlay The overlay handle to unmount.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if the overlay
 * doesn't exist.
 */
ui_error_t ui_overlay_director_unmount(struct ui_overlay_director *director,
                                       struct ui_overlay *overlay) {
  struct ui_overlay *current;
  struct ui_overlay *prev = NULL;
  struct ui_dom_node *p;

  if (!director || !overlay) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = director->first_overlay;
  while (current) {
    if (current == overlay) {
      if (prev) {
        prev->next = current->next;
      } else {
        director->first_overlay = current->next;
      }
      break;
    }
    prev = current;
    current = current->next;
  }

  if (!current) {
    return UI_ERROR_NOT_FOUND;
  }

  /* Detach wrapper from parent before destroying to avoid dangling pointers */
  p = overlay->wrapper_node->parent;
  if (overlay->wrapper_node->previous_sibling) {
    overlay->wrapper_node->previous_sibling->next_sibling =
        overlay->wrapper_node->next_sibling;
  } else {
    p->first_child = overlay->wrapper_node->next_sibling;
  }
  if (overlay->wrapper_node->next_sibling) {
    overlay->wrapper_node->next_sibling->previous_sibling =
        overlay->wrapper_node->previous_sibling;
  } else {
    p->last_child = overlay->wrapper_node->previous_sibling;
  }
  overlay->wrapper_node->parent = NULL;
  overlay->wrapper_node->previous_sibling = NULL;
  overlay->wrapper_node->next_sibling = NULL;
  (void)ui_dom_node_destroy(overlay->wrapper_node);

  C_MULTIPLATFORM_FREE(overlay);
  return UI_ERROR_NONE;
}
