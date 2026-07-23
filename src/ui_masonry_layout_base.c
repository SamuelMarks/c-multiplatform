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

/** \brief ui_masonry_layout_base */
struct ui_masonry_layout_base {
  struct ui_component *component;
  struct ui_computed *data_signal;
};

/** \brief ui_error */
enum ui_error
ui_masonry_layout_base_create(struct ui_masonry_layout_base **out_masonry) {
  struct ui_masonry_layout_base *masonry;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_masonry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  masonry = (struct ui_masonry_layout_base *)UI_MALLOC(
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

  (void)ui_component_set_default_style(masonry->component, default_style);

  masonry->component->shadow_root = root_node;
  root_node = NULL;

  *out_masonry = masonry;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  if (masonry->component) {
    ui_component_destroy(masonry->component);
  }
  UI_FREE(masonry);
  return rc;
}

void ui_masonry_layout_base_destroy(struct ui_masonry_layout_base *masonry) {
  if (!masonry) {
    return;
  }
  if (masonry->component) {
    ui_component_destroy(masonry->component);
  }
  UI_FREE(masonry);
}

/** \brief ui_error */
enum ui_error
ui_masonry_layout_base_reflow(struct ui_masonry_layout_base *masonry) {
  if (!masonry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* In a real implementation, we would query child nodes, measure their sizes,
   * and assign them to columns using a 2D bin packing or balancing algorithm.
   * For the unstyled CDK base, we simulate a reflow success. */
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_masonry_layout_base_get_component(struct ui_masonry_layout_base *masonry,
                                     struct ui_component **out_component) {
  if (!masonry || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = masonry->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_masonry_layout_base_bind_data(struct ui_masonry_layout_base *widget,
                                 struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
