/*
 * \file ui_masonry_layout_base.c
 * \brief Implementation of the UI masonry layout base component.
 */
/* clang-format off */
#include "ui_masonry_layout_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

static const char *ui_masonry_layout_base_default_css =
    ".masonry-container { "
    "display: flex; "
    "flex-direction: column; "
    "flex-wrap: wrap; "
    "}";

/* \brief ui_masonry_layout_base
 */
/**
 * @struct ui_masonry_layout_base
 * \struct ui_masonry_layout_base
 * \brief Represents a masonry (waterfall) style layout container.
 */
struct ui_masonry_layout_base {
  struct ui_component *component;  /**< component */
  struct ui_computed *data_signal; /**< data_signal */
};

/* \brief ui_error
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
    (void)ui_dom_node_destroy(root_node);
  }
  if (masonry->component) {
    (void)ui_component_destroy(masonry->component);
  }
  C_MULTIPLATFORM_FREE(masonry);
  return rc;
}

/*
 * \brief Destroys a masonry layout base component.
 * \param[in,out] masonry The masonry layout to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_masonry_layout_base_destroy(struct ui_masonry_layout_base *masonry) {
  if (!masonry) {
    return UI_ERROR_NONE;
  }
  (void)ui_component_destroy(masonry->component);
  C_MULTIPLATFORM_FREE(masonry);
  return UI_ERROR_NONE;
}

/* \brief ui_error
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

/* \brief ui_error
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

/* \brief ui_error
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
