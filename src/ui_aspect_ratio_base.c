/* clang-format off */
#include "ui_aspect_ratio_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <stdio.h>
/* clang-format on */

static const char *ui_aspect_ratio_base_default_css =
    ".aspect-ratio-container { "
    "position: relative; "
    "width: 100%; "
    "} "
    ".aspect-ratio-container::before { "
    "content: ''; "
    "display: block; "
    "padding-bottom: var(--aspect-ratio-padding, 100%); "
    "} "
    ".aspect-ratio-content { "
    "position: absolute; "
    "top: 0; "
    "left: 0; "
    "width: 100%; "
    "height: 100%; "
    "}";

struct ui_aspect_ratio_base {
  struct ui_component *component;
  float ratio;
  struct ui_signal *ratio_signal;
};

/**
 * @brief Creates a new unstyled aspect ratio bounding box base component.
 *
 * @param out_aspect_ratio Pointer to receive the allocated component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_aspect_ratio_base_create(struct ui_aspect_ratio_base **out_aspect_ratio) {
  struct ui_aspect_ratio_base *ar;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_aspect_ratio) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ar = (struct ui_aspect_ratio_base *)UI_MALLOC(
      sizeof(struct ui_aspect_ratio_base));
  if (!ar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  ar->component = NULL;
  ar->ratio = 1.0f; /* Default 1:1 */

  rc = ui_component_create(&ar->component);
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

  rc = ui_dom_node_set_attribute(root_node, "class", "aspect-ratio-container");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc =
      ui_css_parse_stylesheet(ui_aspect_ratio_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  ui_component_set_default_style(ar->component, default_style);

  ar->component->shadow_root = root_node;
  root_node = NULL;

  *out_aspect_ratio = ar;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  if (ar->component) {
    ui_component_destroy(ar->component);
  }
  UI_FREE(ar);
  return rc;
}

/**
 * @brief Destroys an aspect ratio base component.
 *
 * @param aspect_ratio The component to destroy.
 */
void ui_aspect_ratio_base_destroy(struct ui_aspect_ratio_base *aspect_ratio) {
  if (!aspect_ratio) {
    return;
  }
  ui_component_destroy(aspect_ratio->component);
  UI_FREE(aspect_ratio);
}

/**
 * @brief Sets the aspect ratio (width / height).
 *
 * @param aspect_ratio The component.
 * @param ratio The aspect ratio float value (e.g., 16.0f / 9.0f).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_aspect_ratio_base_set_ratio(struct ui_aspect_ratio_base *aspect_ratio,
                               float ratio) {
  char buffer[64];
  float padding_bottom;
  enum ui_error rc;

  if (!aspect_ratio) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ratio <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  aspect_ratio->ratio = ratio;
  padding_bottom = (1.0f / ratio) * 100.0f;

#if defined(_MSC_VER)
  sprintf_s(buffer, sizeof(buffer), "%.2f%%", padding_bottom);
#else
  sprintf(buffer, "%.2f%%", padding_bottom);
#endif

  rc = ui_component_set_property(aspect_ratio->component,
                                 "--aspect-ratio-padding", buffer);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param aspect_ratio The aspect ratio component.
 * @return The underlying component.
 */
enum ui_error
ui_aspect_ratio_base_get_component(struct ui_aspect_ratio_base *aspect_ratio,
                                   struct ui_component **out_component) {
  if (!aspect_ratio || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = aspect_ratio->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the ratio property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_aspect_ratio_base_bind_ratio(struct ui_aspect_ratio_base *widget,
                                struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->ratio_signal = signal;
  return UI_ERROR_NONE;
}
