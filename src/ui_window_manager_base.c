/* clang-format off */
#include "ui_window_manager_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

static const char *ui_window_manager_base_default_css =
    ".window-manager-container { "
    "position: relative; "
    "overflow: hidden; "
    "width: 100%; "
    "height: 100%; "
    "}";

/** \brief ui_window_manager_base */
struct ui_window_manager_base {
  struct ui_component *component;
  struct ui_computed *data_signal;
};

/** \brief ui_window_manager_base_create */
enum ui_error ui_window_manager_base_create(
    struct ui_window_manager_base **out_window_manager) {
  struct ui_window_manager_base *wm;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_window_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  wm = (struct ui_window_manager_base *)UI_MALLOC(
      sizeof(struct ui_window_manager_base));
  if (!wm) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  wm->component = NULL;

  rc = ui_component_create(&wm->component);
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

  rc =
      ui_dom_node_set_attribute(root_node, "class", "window-manager-container");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_window_manager_base_default_css,
                               &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  (void)ui_component_set_default_style(wm->component, default_style);

  wm->component->shadow_root = root_node;
  root_node = NULL;

  *out_window_manager = wm;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  if (wm && wm->component) {
    ui_component_destroy(wm->component);
  }
  UI_FREE(wm);
  return rc;
}

/** \brief ui_window_manager_base_destroy */
void ui_window_manager_base_destroy(
    struct ui_window_manager_base *window_manager) {
  if (!window_manager) {
    return;
  }
  if (window_manager->component) {
    ui_component_destroy(window_manager->component);
  }
  UI_FREE(window_manager);
}

/** \brief ui_window_manager_base_get_component */
enum ui_error ui_window_manager_base_get_component(
    struct ui_window_manager_base *window_manager,
    struct ui_component **out_component) {
  if (!window_manager || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = window_manager->component;
  return UI_ERROR_NONE;
}

/** \brief ui_window_manager_base_bring_to_front */
enum ui_error ui_window_manager_base_bring_to_front(
    struct ui_window_manager_base *window_manager, int window_id) {
  (void)window_id;
  if (!window_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* Stacking context modification logic here */
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_window_manager_base_drag(struct ui_window_manager_base *window_manager,
                            int window_id, float delta_x, float delta_y) {
  (void)window_id;
  (void)delta_x;
  (void)delta_y;
  if (!window_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* Drag constraint solver logic here */
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_window_manager_base_bind_data(struct ui_window_manager_base *widget,
                                 struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
