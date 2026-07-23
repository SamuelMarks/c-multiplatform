/* clang-format off */
#include "ui_page_control_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

enum ui_error
ui_page_control_base_create(struct ui_page_control_base **out_control) {
  struct ui_page_control_base *control;
  struct ui_component *base_comp;
  enum ui_error err;

  if (!out_control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  control = (struct ui_page_control_base *)UI_MALLOC(
      sizeof(struct ui_page_control_base));
  if (!control) {
    ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  control->base = *base_comp;
  UI_FREE(base_comp);

  control->current_page = 0;
  control->number_of_pages = 0;

  err =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &control->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    UI_FREE(control);
    return err;
  }

  err = ui_dom_node_set_tag_name(control->base.shadow_root, "ui-page-control");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(control->base.shadow_root);
    UI_FREE(control);
    return err;
  }

  *out_control = control;
  return UI_ERROR_NONE;
}

static enum ui_error
update_page_control_dom(struct ui_page_control_base *control) {
  char buf[32];
  /* This is a simple unstyled stub. A real implementation would append child
   * dots to the shadow_root for each page. */

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%d", control->number_of_pages);
#else
  sprintf(buf, "%d", control->number_of_pages);
#endif
  ui_dom_node_set_attribute(control->base.shadow_root, "data-total-pages", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%d", control->current_page);
#else
  sprintf(buf, "%d", control->current_page);
#endif
  ui_dom_node_set_attribute(control->base.shadow_root, "data-current-page",
                            buf);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_page_control_base_set_number_of_pages(struct ui_page_control_base *control,
                                         int count) {
  if (!control || count < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->number_of_pages = count;
  (void)update_page_control_dom(control);

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_page_control_base_set_current_page(struct ui_page_control_base *control,
                                      int page) {
  if (!control || page < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->current_page = page;
  (void)update_page_control_dom(control);

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_page_control_base_bind_current_page(struct ui_page_control_base *widget,
                                       struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->current_page_signal = signal;
  return UI_ERROR_NONE;
}
