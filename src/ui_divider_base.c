/* clang-format off */
#include "ui_divider_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_divider_base_css =
    ":host { "
    "display: block; "
    "margin: 0; "
    "border-top-style: solid; "
    "border-top-width: 1px; "
    "border-top-color: var(--divider-color, rgba(0,0,0,0.12)); "
    "} "
    ":host([data-orientation=\"vertical\"]) { "
    "border-top: none; "
    "border-left-style: solid; "
    "border-left-width: 1px; "
    "border-left-color: var(--divider-color, rgba(0,0,0,0.12)); "
    "height: auto; "
    "align-self: stretch; "
    "} "
    ":host([data-inset=\"true\"]) { "
    "margin-left: 72px; "
    "}";

/** \brief ui_divider_base */
struct ui_divider_base {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  enum ui_divider_orientation orientation;
  int inset;
  struct ui_signal *data_signal;
};

static enum ui_error update_dom_state(struct ui_divider_base *divider) {
  if (divider->orientation == UI_DIVIDER_ORIENTATION_VERTICAL) {
    ui_dom_node_set_attribute(divider->root_node, "data-orientation",
                              "vertical");
  } else {
    ui_dom_node_set_attribute(divider->root_node, "data-orientation",
                              "horizontal");
  }

  if (divider->inset) {
    ui_dom_node_set_attribute(divider->root_node, "data-inset", "true");
  } else {
    ui_dom_node_remove_attribute(divider->root_node, "data-inset");
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_divider_base_create(struct ui_divider_base **out_divider) {
  struct ui_divider_base *divider;
  struct ui_css_stylesheet *default_style = NULL;
  enum ui_error rc;

  if (!out_divider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  divider = (struct ui_divider_base *)UI_MALLOC(sizeof(struct ui_divider_base));
  if (!divider) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(divider, 0, sizeof(struct ui_divider_base));

  rc = ui_component_create(&divider->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &divider->root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Use <hr> tag for proper ARIA semantics parsing (UI_ARIA_ROLE_SEPARATOR) */
  rc = ui_dom_node_set_tag_name(divider->root_node, "hr");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_divider_base_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  ui_component_set_default_style(divider->component, default_style);

  divider->component->shadow_root = divider->root_node;
  divider->orientation = UI_DIVIDER_ORIENTATION_HORIZONTAL;
  divider->inset = 0;

  (void)update_dom_state(divider);

  *out_divider = divider;
  return UI_ERROR_NONE;

cleanup:
  if (divider->root_node) {
    ui_dom_node_destroy(divider->root_node);
  }
  ui_component_destroy(divider->component);
  UI_FREE(divider);
  return rc;
}

void ui_divider_base_destroy(struct ui_divider_base *divider) {
  if (!divider) {
    return;
  }
  ui_component_destroy(divider->component);
  UI_FREE(divider);
}

/** \brief ui_error */
enum ui_error
ui_divider_base_set_orientation(struct ui_divider_base *divider,
                                enum ui_divider_orientation orientation) {
  if (!divider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  divider->orientation = orientation;
  (void)update_dom_state(divider);
  return UI_ERROR_NONE;
}

enum ui_error ui_divider_base_set_inset(struct ui_divider_base *divider,
                                        int inset) {
  if (!divider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  divider->inset = inset;
  (void)update_dom_state(divider);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_divider_base_get_component(struct ui_divider_base *divider,
                              struct ui_component **out_component) {
  if (!divider || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = divider->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_divider_base_bind_data(struct ui_divider_base *widget,
                                        struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
