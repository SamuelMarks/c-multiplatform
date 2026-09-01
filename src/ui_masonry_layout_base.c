/**
 * @file ui_masonry_layout_base.c
 * @brief Implementation of the UI masonry layout base component.
 * @details Provides the base DOM representation for masonry layout.
 */
/* clang-format off */
#include "ui_masonry_layout_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

/** @brief Default CSS stylesheet for masonry layout */
static const char *ui_masonry_layout_base_default_css =
    ".masonry-container { "
    "display: flex; "
    "flex-direction: column; "
    "flex-wrap: wrap; "
    "}";

/**
 * @struct ui_masonry_layout_base
 * @brief Represents a masonry (waterfall) style layout container.
 * @details State and internal data structure.
 */
struct ui_masonry_layout_base {
  struct ui_component *component;  /**< The underlying DOM component */
  struct ui_computed *data_signal; /**< The data bound signal */
};

/**
 * @brief Creates a masonry layout base component.
 * @param[out] out_masonry Pointer to store the newly created masonry layout.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_masonry_layout_base_create(struct ui_masonry_layout_base **out_masonry) {
  struct ui_masonry_layout_base *masonry;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_masonry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  masonry = (struct ui_masonry_layout_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_masonry_layout_base));
  if (!masonry) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  masonry->component = NULL;

  rc = ui_component_create(&masonry->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "class", "masonry-container");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_masonry_layout_base_default_css,
                               &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  {
    ui_error_t _ign_rc =
        ui_component_set_default_style(masonry->component, default_style);
    (void)_ign_rc;
  }

  masonry->component->shadow_root = root_node;
  root_node = NULL;

  *out_masonry = masonry;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  if (masonry->component) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(masonry->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  C_MULTIPLATFORM_FREE(masonry);
  return rc;
}

/**
 * @brief Destroys a masonry layout base component.
 * @param[in,out] masonry The masonry layout to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_masonry_layout_base_destroy(struct ui_masonry_layout_base *masonry) {
  if (!masonry) {
    return UI_ERROR_NONE;
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(masonry->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  C_MULTIPLATFORM_FREE(masonry);
  return UI_ERROR_NONE;
}

/**
 * @brief Reflows the masonry layout component.
 * @param[in,out] masonry The masonry layout to reflow.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_masonry_layout_base_reflow(struct ui_masonry_layout_base *masonry) {
  if (!masonry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* In a real implementation, we would query child nodes, measure their sizes,
   * and assign them to columns using a 2D bin packing or balancing algorithm.
   * For the unstyled CDK base, we simulate a reflow success. */
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the generic DOM component for the masonry layout.
 * @param[in] masonry The masonry layout.
 * @param[out] out_component Pointer to store the underlying component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_masonry_layout_base_get_component(struct ui_masonry_layout_base *masonry,
                                     struct ui_component **out_component) {
  if (!masonry || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = masonry->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds a reactive data signal to the masonry layout.
 * @param[in,out] widget The masonry layout widget.
 * @param[in,out] signal The reactive computed data signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_masonry_layout_base_bind_data(struct ui_masonry_layout_base *widget,
                                 struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
