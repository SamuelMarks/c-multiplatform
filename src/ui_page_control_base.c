/**
 * @file ui_page_control_base.c
 * @brief ui_page_control_base.c implementation.
 */
/* clang-format off */
#include "ui_page_control_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

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
    (void)ui_component_destroy(base_comp);
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
    (void)ui_dom_node_destroy(control->base.shadow_root);
    C_MULTIPLATFORM_FREE(control);
    return err;
  }

  *out_control = control;
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
