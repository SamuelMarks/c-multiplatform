/* clang-format off */
#include "ui_scaffold_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

static ui_error_t create_slot(struct ui_dom_node *parent, const char *slot_name,
                              struct ui_dom_node **out_slot) {
  ui_error_t err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, out_slot);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_set_tag_name(*out_slot, "div");
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(*out_slot);
    return err;
  }

  {
    ui_error_t set_rc =
        ui_dom_node_set_attribute(*out_slot, "data-slot", slot_name);
    if (set_rc != UI_ERROR_NONE) {
      (void)ui_dom_node_destroy(*out_slot);
      return set_rc;
    }
  }
  {
    ui_error_t ap_rc = ui_dom_node_append_child(parent, *out_slot);
    (void)ap_rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_scaffold_base_create(struct ui_scaffold_base **out_scaffold) {
  struct ui_scaffold_base *scaffold;
  struct ui_component *base_comp;
  ui_error_t err;

  if (!out_scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  scaffold = (struct ui_scaffold_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_scaffold_base));
  if (!scaffold) {
    (void)ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  scaffold->base = *base_comp;
  C_MULTIPLATFORM_FREE(base_comp);

  err =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &scaffold->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(scaffold);
    return err;
  }

  err = ui_dom_node_set_tag_name(scaffold->base.shadow_root, "ui-scaffold");
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(scaffold->base.shadow_root);
    C_MULTIPLATFORM_FREE(scaffold);
    return err;
  }

  /* Create slots */
  {
    ui_error_t cs_rc1 = create_slot(scaffold->base.shadow_root, "top-bar",
                                    &scaffold->slot_top_bar);
    if (cs_rc1 != UI_ERROR_NONE)
      return cs_rc1;
  }
  {
    ui_error_t cs_rc2 = create_slot(scaffold->base.shadow_root, "side-nav",
                                    &scaffold->slot_side_nav);
    if (cs_rc2 != UI_ERROR_NONE)
      return cs_rc2;
  }
  {
    ui_error_t cs_rc3 = create_slot(scaffold->base.shadow_root, "main-content",
                                    &scaffold->slot_main_content);
    if (cs_rc3 != UI_ERROR_NONE)
      return cs_rc3;
  }
  {
    ui_error_t cs_rc4 = create_slot(scaffold->base.shadow_root, "bottom-bar",
                                    &scaffold->slot_bottom_bar);
    if (cs_rc4 != UI_ERROR_NONE)
      return cs_rc4;
  }

  *out_scaffold = scaffold;
  return UI_ERROR_NONE;
}

ui_error_t ui_scaffold_base_set_top_bar(struct ui_scaffold_base *scaffold,
                                        struct ui_component *top_bar) {
  if (!scaffold || !top_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_append_child(scaffold->slot_top_bar, top_bar->shadow_root);
}

/** \brief ui_error */
ui_error_t ui_scaffold_base_set_main_content(struct ui_scaffold_base *scaffold,
                                             struct ui_component *content) {
  if (!scaffold || !content) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_append_child(scaffold->slot_main_content,
                                  content->shadow_root);
}

ui_error_t ui_scaffold_base_bind_data(struct ui_scaffold_base *widget,
                                      struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
