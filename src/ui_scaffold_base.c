/* clang-format off */
#include "ui_scaffold_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

static enum ui_error create_slot(struct ui_dom_node *parent,
                                 const char *slot_name,
                                 struct ui_dom_node **out_slot) {
  enum ui_error err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, out_slot);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_set_tag_name(*out_slot, "div");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(*out_slot);
    return err;
  }

  ui_dom_node_set_attribute(*out_slot, "data-slot", slot_name);
  ui_dom_node_append_child(parent, *out_slot);

  return UI_ERROR_NONE;
}

enum ui_error ui_scaffold_base_create(struct ui_scaffold_base **out_scaffold) {
  struct ui_scaffold_base *scaffold;
  struct ui_component *base_comp;
  enum ui_error err;

  if (!out_scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  scaffold =
      (struct ui_scaffold_base *)UI_MALLOC(sizeof(struct ui_scaffold_base));
  if (!scaffold) {
    ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  scaffold->base = *base_comp;
  UI_FREE(base_comp);

  err =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &scaffold->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    UI_FREE(scaffold);
    return err;
  }

  err = ui_dom_node_set_tag_name(scaffold->base.shadow_root, "ui-scaffold");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->base.shadow_root);
    UI_FREE(scaffold);
    return err;
  }

  /* Create slots */
  create_slot(scaffold->base.shadow_root, "top-bar", &scaffold->slot_top_bar);
  create_slot(scaffold->base.shadow_root, "side-nav", &scaffold->slot_side_nav);
  create_slot(scaffold->base.shadow_root, "main-content",
              &scaffold->slot_main_content);
  create_slot(scaffold->base.shadow_root, "bottom-bar",
              &scaffold->slot_bottom_bar);

  *out_scaffold = scaffold;
  return UI_ERROR_NONE;
}

enum ui_error ui_scaffold_base_set_top_bar(struct ui_scaffold_base *scaffold,
                                           struct ui_component *top_bar) {
  if (!scaffold || !top_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_append_child(scaffold->slot_top_bar, top_bar->shadow_root);
}

/** \brief ui_error */
enum ui_error
ui_scaffold_base_set_main_content(struct ui_scaffold_base *scaffold,
                                  struct ui_component *content) {
  if (!scaffold || !content) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_append_child(scaffold->slot_main_content,
                                  content->shadow_root);
}

enum ui_error ui_scaffold_base_bind_data(struct ui_scaffold_base *widget,
                                         struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
