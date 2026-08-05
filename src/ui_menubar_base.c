/* clang-format off */
#include "ui_menubar_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

ui_error_t ui_menubar_base_create(struct ui_menubar_base **out_menubar) {
  struct ui_menubar_base *menubar;
  struct ui_component *base_comp;
  ui_error_t err;

  if (!out_menubar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  menubar = (struct ui_menubar_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_menubar_base));
  if (!menubar) {
    (void)ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  menubar->base = *base_comp;
  C_MULTIPLATFORM_FREE(base_comp);

  err =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &menubar->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(menubar);
    return err;
  }

  err = ui_dom_node_set_tag_name(menubar->base.shadow_root, "ui-menubar");
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(menubar->base.shadow_root);
    C_MULTIPLATFORM_FREE(menubar);
    return err;
  }

  err = ui_dom_node_set_attribute(menubar->base.shadow_root, "role", "menubar");
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(menubar->base.shadow_root);
    C_MULTIPLATFORM_FREE(menubar);
    return err;
  }
  err = ui_dom_node_set_attribute(menubar->base.shadow_root, "tabindex",
                                  "0"); /* Focusable context */
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(menubar->base.shadow_root);
    C_MULTIPLATFORM_FREE(menubar);
    return err;
  }

  *out_menubar = menubar;
  return UI_ERROR_NONE;
}

ui_error_t ui_menubar_base_append_item(struct ui_menubar_base *menubar,
                                       struct ui_component *item) {
  if (!menubar || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_dom_node_append_child(menubar->base.shadow_root, item->shadow_root);
}

ui_error_t ui_menubar_base_bind_active_index(struct ui_menubar_base *widget,
                                             struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
