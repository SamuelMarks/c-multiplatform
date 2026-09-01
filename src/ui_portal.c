/**
 * @file ui_portal.c
 * @brief Implementation of DOM portaling (mounting content in a different part
 * of the tree).
 */

/* clang-format off */
#include "ui_portal.h"
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct ui_portal
 * @brief Maintains state for portaling a DOM node to a different target.
 */
struct ui_portal {
  struct ui_dom_node *physical_target; /**< The physical target node where
                                          content is appended. */
  struct ui_dom_node *content_node; /**< The current content node portaled. */
};

/**
 * @brief Creates a new portal anchored to a specific physical target node.
 * @param[out] out_portal Pointer to store the created portal.
 * @param[in,out] physical_target The destination node in the DOM tree.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_portal_create(struct ui_portal **out_portal,
                            struct ui_dom_node *physical_target) {
  struct ui_portal *portal;

  if (!out_portal || !physical_target) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  portal = (struct ui_portal *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_portal));
  if (!portal) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  portal->physical_target = physical_target;
  portal->content_node = NULL;

  *out_portal = portal;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a portal and unmounts its content.
 * @param[in,out] portal The portal to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_portal_destroy(struct ui_portal *portal) {
  if (!portal) {
    return UI_ERROR_NONE;
  }

  /* Safely cleanup orphaned content to prevent memory leaks */
  if (portal->content_node) {
    /* Unmount from physical target if attached */
    if (portal->content_node->parent == portal->physical_target) {
      {
        ui_error_t rc_cleanup = ui_dom_node_remove_child(
            portal->physical_target, portal->content_node);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(portal->content_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  C_MULTIPLATFORM_FREE(portal);
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the content node to be portaled to the target.
 * @param[in,out] portal The portal.
 * @param[in,out] content_node The DOM node to portal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_portal_set_content(struct ui_portal *portal,
                                 struct ui_dom_node *content_node) {
  ui_error_t rc;

  if (!portal || !content_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (portal->content_node) {
    if (portal->content_node->parent == portal->physical_target) {
      {
        ui_error_t rc_cleanup = ui_dom_node_remove_child(
            portal->physical_target, portal->content_node);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(portal->content_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  portal->content_node = content_node;

  rc = ui_dom_node_append_child(portal->physical_target, portal->content_node);
  if (rc != UI_ERROR_NONE) {
    /* Revert content_node since append failed */
    portal->content_node = NULL;
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Gets the current content node portaled by this portal.
 * @param[in] portal The portal.
 * @param[out] out_content Pointer to store the content node.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_portal_get_content(const struct ui_portal *portal,
                                 struct ui_dom_node **out_content) {
  if (!portal || !out_content) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_content = portal->content_node;
  return UI_ERROR_NONE;
}
