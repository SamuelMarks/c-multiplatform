/**
 * @file ui_datagrid_base.c
 * @brief ui_datagrid_base.c implementation.
 */
/* clang-format off */
#include "ui_datagrid_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

/** @brief Default CSS stylesheet for datagrid base component */
static const char *ui_datagrid_base_default_css = ".datagrid-container { "
                                                  "display: grid; "
                                                  "overflow: auto; "
                                                  "position: relative; "
                                                  "}";

/**
 * @struct ui_datagrid_base
 * @struct ui_datagrid_base
 * @brief Internal representation of a datagrid component.
 */
struct ui_datagrid_base {
  /* @brief The base component. */
  struct ui_component *component; /**< component */
  /* @brief Signal bound for data. */
  struct ui_computed *data_signal; /**< data_signal */
};

/**
 * @brief ui_datagrid_base_create.
 * @param out_datagrid Parameter out_datagrid.
 * @return Return value.
 */
ui_error_t ui_datagrid_base_create(struct ui_datagrid_base **out_datagrid) {
  struct ui_datagrid_base *datagrid;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_datagrid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  datagrid = (struct ui_datagrid_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_datagrid_base));
  if (!datagrid) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  datagrid->component = NULL;

  rc = ui_component_create(&datagrid->component);
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

  rc = ui_dom_node_set_attribute(root_node, "class", "datagrid-container");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "role", "grid");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_datagrid_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  {
    ui_error_t rc_cleanup =
        ui_component_set_default_style(datagrid->component, default_style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  datagrid->component->shadow_root = root_node;
  root_node = NULL;

  *out_datagrid = datagrid;
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
  if (datagrid->component) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(datagrid->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  C_MULTIPLATFORM_FREE(datagrid);
  return rc;
}

/**
 * @brief ui_datagrid_base_destroy.
 * @param datagrid Parameter datagrid.
 * @return Return value.
 */
ui_error_t ui_datagrid_base_destroy(struct ui_datagrid_base *datagrid) {
  if (!datagrid) {
    return UI_ERROR_NONE;
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(datagrid->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  C_MULTIPLATFORM_FREE(datagrid);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_datagrid_base_get_component.
 * @param datagrid Parameter datagrid.
 * @param out_component Parameter out_component.
 * @return Return value.
 */
ui_error_t ui_datagrid_base_get_component(struct ui_datagrid_base *datagrid,
                                          struct ui_component **out_component) {
  if (!datagrid || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = datagrid->component;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_datagrid_base_resize_column.
 * @param datagrid Parameter datagrid.
 * @param col_index Parameter col_index.
 * @param new_width Parameter new_width.
 * @return Return value.
 */
ui_error_t ui_datagrid_base_resize_column(struct ui_datagrid_base *datagrid,
                                          int col_index, float new_width) {
  (void)col_index;
  (void)new_width;
  if (!datagrid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_datagrid_base_move_focus.
 * @param datagrid Parameter datagrid.
 * @param row_delta Parameter row_delta.
 * @param col_delta Parameter col_delta.
 * @return Return value.
 */
ui_error_t ui_datagrid_base_move_focus(struct ui_datagrid_base *datagrid,
                                       int row_delta, int col_delta) {
  (void)row_delta;
  (void)col_delta;
  if (!datagrid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_datagrid_base_bind_data.
 * @param widget Parameter widget.
 * @param signal Parameter signal.
 * @return Return value.
 */
ui_error_t ui_datagrid_base_bind_data(struct ui_datagrid_base *widget,
                                      struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
