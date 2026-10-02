/**
 * @file ui_page_control_base.c
 * @brief ui_page_control_base.c implementation.
 */
/* clang-format off */
#include "ui_page_control_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_page_control_mock_fail = 0;

/**
 * @brief mock_page_control_component_destroy.
 * @param comp Parameter comp.
 * @return Return value.
 */
static ui_error_t
mock_page_control_component_destroy(struct ui_component *comp) {
  if (g_page_control_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_page_control_component_destroy
/** @endcond */

/**
 * @brief mock_page_control_dom_node_destroy.
 * @param node Parameter node.
 * @return Return value.
 */
static ui_error_t mock_page_control_dom_node_destroy(struct ui_dom_node *node) {
  if (g_page_control_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_destroy)(node);
}
#undef ui_dom_node_destroy
/** @cond */
#define ui_dom_node_destroy mock_page_control_dom_node_destroy
/** @endcond */
#endif

/**
 * @brief ui_page_control_base_create.
 * @param out_control Parameter out_control.
 * @return Return value.
 */
ui_error_t
ui_page_control_base_create(struct ui_page_control_base **out_control) {
  struct ui_page_control_base *control;
  struct ui_component *base_comp;
  ui_error_t err;
  ui_error_t rc_cleanup;

  if (!out_control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  control = (struct ui_page_control_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_page_control_base));
  if (!control) {
    rc_cleanup = ui_component_destroy(base_comp);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
    return UI_ERROR_OUT_OF_MEMORY;
  }

  control->base = *base_comp;
  C_MULTIPLATFORM_FREE(base_comp);

  control->current_page = 0;
  control->number_of_pages = 0;

  err =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &control->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(control);
    return err;
  }

  err = ui_dom_node_set_tag_name(control->base.shadow_root, "ui-page-control");
  if (err != UI_ERROR_NONE) {
    rc_cleanup = ui_dom_node_destroy(control->base.shadow_root);
    if (rc_cleanup != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(control);
      return rc_cleanup;
    }
    C_MULTIPLATFORM_FREE(control);
    return err;
  }

  *out_control = control;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_page_control_base_destroy.
 * @param control Parameter control.
 * @return Return value.
 */
ui_error_t ui_page_control_base_destroy(struct ui_page_control_base *control) {
  ui_error_t rc;

  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (control->base.shadow_root) {
    rc = ui_dom_node_destroy(control->base.shadow_root);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    control->base.shadow_root = NULL;
  }

  C_MULTIPLATFORM_FREE(control);
  return UI_ERROR_NONE;
}

/**
 * @brief update_page_control_dom.
 * @param control Parameter control.
 * @return Return value.
 */
static ui_error_t
update_page_control_dom(struct ui_page_control_base *control) {
  char buf[32];
  /* This is a simple unstyled stub. A real implementation would append child
   * dots to the shadow_root for each page. */

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%d", control->number_of_pages);
#else
  sprintf(buf, "%d", control->number_of_pages);
#endif
  {
    ui_error_t attr_rc = ui_dom_node_set_attribute(control->base.shadow_root,
                                                   "data-total-pages", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%d", control->current_page);
#else
  sprintf(buf, "%d", control->current_page);
#endif
  {
    ui_error_t attr_rc = ui_dom_node_set_attribute(control->base.shadow_root,
                                                   "data-current-page", buf);
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_page_control_base_set_number_of_pages(struct ui_page_control_base *control,
                                         int count) {
  ui_error_t rc;
  if (!control || count < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->number_of_pages = count;
  rc = update_page_control_dom(control);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_page_control_base_set_current_page(struct ui_page_control_base *control,
                                      int page) {
  ui_error_t rc;
  if (!control || page < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->current_page = page;
  rc = update_page_control_dom(control);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_page_control_base_bind_current_page(struct ui_page_control_base *widget,
                                       struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->current_page_signal = signal;
  return UI_ERROR_NONE;
}
