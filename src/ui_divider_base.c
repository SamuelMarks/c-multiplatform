/**
 * @file ui_divider_base.c
 * @brief ui_divider_base.c implementation.
 */
/* clang-format off */
#include "ui_divider_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/** @brief Default CSS stylesheet for divider base component */
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

/**
 * @struct ui_divider_base
 * @struct ui_divider_base
 * @brief Internal representation of a divider base component.
 */
struct ui_divider_base {
  /* @brief The base component. */
  struct ui_component *component; /**< component */
  /* @brief The DOM node. */
  struct ui_dom_node *root_node; /**< root_node */
  /* @brief The orientation of the divider. */
  enum ui_divider_orientation orientation; /**< orientation */
  /* @brief 1 if the divider has an inset, 0 otherwise. */
  int inset; /**< inset */
  /* @brief Signal bound for data. */
  struct ui_signal *data_signal; /**< data_signal */
};

/**
 * @brief update_dom_state.
 * @param divider Parameter divider.
 * @return Return value.
 */
static ui_error_t update_dom_state(struct ui_divider_base *divider) {
  if (divider->orientation == UI_DIVIDER_ORIENTATION_VERTICAL) {
    (void)ui_dom_node_set_attribute(divider->root_node, "data-orientation",
                                    "vertical");
  } else {
    (void)ui_dom_node_set_attribute(divider->root_node, "data-orientation",
                                    "horizontal");
  }

  if (divider->inset) {
    (void)ui_dom_node_set_attribute(divider->root_node, "data-inset", "true");
  } else {
    (void)ui_dom_node_remove_attribute(divider->root_node, "data-inset");
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_divider_base_create.
 * @param out_divider Parameter out_divider.
 * @return Return value.
 */
ui_error_t ui_divider_base_create(struct ui_divider_base **out_divider) {
  struct ui_divider_base *divider;
  struct ui_css_stylesheet *default_style = NULL;
  ui_error_t rc;

  if (!out_divider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  divider = (struct ui_divider_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_divider_base));
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

  (void)ui_component_set_default_style(divider->component, default_style);

  divider->component->shadow_root = divider->root_node;
  divider->orientation = UI_DIVIDER_ORIENTATION_HORIZONTAL;
  divider->inset = 0;

  (void)update_dom_state(divider);

  *out_divider = divider;
  return UI_ERROR_NONE;

cleanup:
  if (divider->root_node) {
    (void)ui_dom_node_destroy(divider->root_node);
  }
  if (divider->component) {
    (void)ui_component_destroy(divider->component);
  }
  C_MULTIPLATFORM_FREE(divider);
  return rc;
}

/**
 * @brief ui_divider_base_destroy.
 * @param divider Parameter divider.
 * @return Return value.
 */
ui_error_t ui_divider_base_destroy(struct ui_divider_base *divider) {
  if (!divider) {
    return UI_ERROR_NONE;
  }
  (void)ui_component_destroy(divider->component);
  C_MULTIPLATFORM_FREE(divider);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_divider_base_set_orientation.
 * @param divider Parameter divider.
 * @param orientation Parameter orientation.
 * @return Return value.
 */
ui_error_t
ui_divider_base_set_orientation(struct ui_divider_base *divider,
                                enum ui_divider_orientation orientation) {
  if (!divider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  divider->orientation = orientation;
  return update_dom_state(divider);
}

/**
 * @brief ui_divider_base_set_inset.
 * @param divider Parameter divider.
 * @param inset Parameter inset.
 * @return Return value.
 */
ui_error_t ui_divider_base_set_inset(struct ui_divider_base *divider,
                                     int inset) {
  if (!divider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  divider->inset = inset;
  return update_dom_state(divider);
}

/**
 * @brief ui_divider_base_get_component.
 * @param divider Parameter divider.
 * @param out_component Parameter out_component.
 * @return Return value.
 */
ui_error_t ui_divider_base_get_component(struct ui_divider_base *divider,
                                         struct ui_component **out_component) {
  if (!divider || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = divider->component;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_divider_base_bind_data.
 * @param widget Parameter widget.
 * @param signal Parameter signal.
 * @return Return value.
 */
ui_error_t ui_divider_base_bind_data(struct ui_divider_base *widget,
                                     struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
