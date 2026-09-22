/**
 * @file ui_window_manager_base.c
 * @brief Implementation of the window manager base component.
 */

/* clang-format off */
#include "ui_window_manager_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

/** @brief Default CSS stylesheet */
static const char *ui_window_manager_base_default_css =
    ".window-manager-container { "
    "position: relative; "
    "overflow: hidden; "
    "width: 100%; "
    "height: 100%; "
    "}";

/**
 * @struct ui_window_manager_base
 * @struct ui_window_manager_base
 * @brief Internal state for the window manager base component.
 */
struct ui_window_manager_base {
  struct ui_component *component;  /**< component */
  struct ui_computed *data_signal; /**< data_signal */
};

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Global flag to simulate failure in window manager base operations */
int g_wm_mock_fail = 0;

/**
 * @brief Mock for ui_component_set_default_style in window manager tests.
 * @param[in,out] comp The component to set default style for.
 * @param[in] style The default CSS stylesheet.
 * @return UI_ERROR_NONE on success, or UI_ERROR_UNKNOWN when mock fail
 * triggered.
 */
static ui_error_t
mock_wm_component_set_default_style(struct ui_component *comp,
                                    struct ui_css_stylesheet *style) {
  if (g_wm_mock_fail == 1 || g_wm_mock_fail == 2 || g_wm_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_set_default_style(comp, style);
}
/** @cond */
#define ui_component_set_default_style mock_wm_component_set_default_style
/** @endcond */

/**
 * @brief Mock for ui_dom_node_destroy in window manager tests.
 * @param[in,out] node The DOM node to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_UNKNOWN when mock fail
 * triggered.
 */
static ui_error_t mock_wm_dom_node_destroy(struct ui_dom_node *node) {
  if (g_wm_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_destroy(node);
}
/** @cond */
#define ui_dom_node_destroy mock_wm_dom_node_destroy
/** @endcond */

/**
 * @brief Mock for ui_component_destroy in window manager tests.
 * @param[in,out] comp The component to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_UNKNOWN when mock fail
 * triggered.
 */
static ui_error_t mock_wm_component_destroy(struct ui_component *comp) {
  if (g_wm_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(comp);
}
/** @cond */
#define ui_component_destroy mock_wm_component_destroy
/** @endcond */
#endif

ui_error_t ui_window_manager_base_create(
    struct ui_window_manager_base **out_window_manager) {
  struct ui_window_manager_base *wm;
  ui_error_t rc;
  ui_error_t rc_cleanup;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_window_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_window_manager = NULL;

  wm = (struct ui_window_manager_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_window_manager_base));
  if (!wm) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  wm->component = NULL;
  wm->data_signal = NULL;

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

  rc = ui_component_set_default_style(wm->component, default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  wm->component->shadow_root = root_node;
  root_node = NULL;

  *out_window_manager = wm;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  if (wm->component) {
    rc_cleanup = ui_component_destroy(wm->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  C_MULTIPLATFORM_FREE(wm);
  return rc;
}

ui_error_t
ui_window_manager_base_destroy(struct ui_window_manager_base *window_manager) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!window_manager) {
    return UI_ERROR_NONE;
  }
  if (window_manager->component) {
    rc = ui_component_destroy(window_manager->component);
  }
  C_MULTIPLATFORM_FREE(window_manager);
  return rc;
}

ui_error_t ui_window_manager_base_get_component(
    struct ui_window_manager_base *window_manager,
    struct ui_component **out_component) {
  if (!window_manager || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = window_manager->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_window_manager_base_bring_to_front(
    struct ui_window_manager_base *window_manager, int window_id) {
  int unused_id = window_id;
  window_id = unused_id;
  if (!window_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* Stacking context modification logic here */
  return UI_ERROR_NONE;
}

ui_error_t
ui_window_manager_base_drag(struct ui_window_manager_base *window_manager,
                            int window_id, float delta_x, float delta_y) {
  int unused_id = window_id;
  float unused_dx = delta_x;
  float unused_dy = delta_y;
  window_id = unused_id;
  delta_x = unused_dx;
  delta_y = unused_dy;
  if (!window_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* Drag constraint solver logic here */
  return UI_ERROR_NONE;
}

ui_error_t
ui_window_manager_base_bind_data(struct ui_window_manager_base *widget,
                                 struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
