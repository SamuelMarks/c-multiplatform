/**
 * @file ui_scaffold_base.c
 * @brief ui_scaffold_base.c implementation.
 */
/* clang-format off */
#include "ui_scaffold_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_scaffold_mock_fail = 0;
int g_scaffold_slot_fail_idx = 0;
static int g_slot_counter = 0;

static ui_error_t mock_scaffold_set_tag_name(struct ui_dom_node *node,
                                             const char *tag) {
  if (g_scaffold_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_scaffold_mock_fail == 2) {
    if (strcmp(tag, "div") == 0) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_dom_node_set_tag_name)(node, tag);
}
#undef ui_dom_node_set_tag_name
/** @cond */
#define ui_dom_node_set_tag_name mock_scaffold_set_tag_name
/** @endcond */

static ui_error_t mock_scaffold_set_attribute(struct ui_dom_node *node,
                                              const char *k, const char *v) {
  if (g_scaffold_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_set_attribute)(node, k, v);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_scaffold_set_attribute
/** @endcond */

static ui_error_t mock_scaffold_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_scaffold_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_scaffold_append_child
/** @endcond */
#endif

/**
 * @brief create_slot.
 * @param parent Parameter parent.
 * @param slot_name Parameter slot_name.
 * @param out_slot Parameter out_slot.
 * @return Return value.
 */
static ui_error_t create_slot(struct ui_dom_node *parent, const char *slot_name,
                              struct ui_dom_node **out_slot) {
  ui_error_t err;

#ifdef UI_TEST_MOCK_ALLOC
  g_slot_counter++;
  if (g_scaffold_slot_fail_idx > 0 &&
      g_slot_counter == g_scaffold_slot_fail_idx) {
    return UI_ERROR_UNKNOWN;
  }
#endif

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, out_slot);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_set_tag_name(*out_slot, "div");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(*out_slot);
    return err;
  }

  err = ui_dom_node_set_attribute(*out_slot, "data-slot", slot_name);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(*out_slot);
    return err;
  }

  err = ui_dom_node_append_child(parent, *out_slot);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(*out_slot);
    return err;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_scaffold_base_create.
 * @param out_scaffold Parameter out_scaffold.
 * @return Return value.
 */
ui_error_t ui_scaffold_base_create(struct ui_scaffold_base **out_scaffold) {
  struct ui_scaffold_base *scaffold;
  struct ui_component *base_comp;
  ui_error_t err;

  if (!out_scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_TEST_MOCK_ALLOC
  g_slot_counter = 0;
#endif

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  scaffold = (struct ui_scaffold_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_scaffold_base));
  if (!scaffold) {
    ui_component_destroy(base_comp);
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
    ui_dom_node_destroy(scaffold->base.shadow_root);
    C_MULTIPLATFORM_FREE(scaffold);
    return err;
  }

  /* Create slots */
  err = create_slot(scaffold->base.shadow_root, "top-bar",
                    &scaffold->slot_top_bar);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->base.shadow_root);
    C_MULTIPLATFORM_FREE(scaffold);
    return err;
  }

  err = create_slot(scaffold->base.shadow_root, "side-nav",
                    &scaffold->slot_side_nav);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->base.shadow_root);
    C_MULTIPLATFORM_FREE(scaffold);
    return err;
  }

  err = create_slot(scaffold->base.shadow_root, "main-content",
                    &scaffold->slot_main_content);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->base.shadow_root);
    C_MULTIPLATFORM_FREE(scaffold);
    return err;
  }

  err = create_slot(scaffold->base.shadow_root, "bottom-bar",
                    &scaffold->slot_bottom_bar);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->base.shadow_root);
    C_MULTIPLATFORM_FREE(scaffold);
    return err;
  }

  *out_scaffold = scaffold;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_scaffold_base_set_top_bar.
 * @param scaffold Parameter scaffold.
 * @param top_bar Parameter top_bar.
 * @return Return value.
 */
ui_error_t ui_scaffold_base_set_top_bar(struct ui_scaffold_base *scaffold,
                                        struct ui_component *top_bar) {
  if (!scaffold || !top_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_append_child(scaffold->slot_top_bar, top_bar->shadow_root);
}

/**
 * @brief Sets the main content component of the scaffold.
 * @param scaffold Parameter scaffold.
 * @param content Parameter content.
 * @return Return value.
 */
ui_error_t ui_scaffold_base_set_main_content(struct ui_scaffold_base *scaffold,
                                             struct ui_component *content) {
  if (!scaffold || !content) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_append_child(scaffold->slot_main_content,
                                  content->shadow_root);
}

/**
 * @brief ui_scaffold_base_bind_data.
 * @param widget Parameter widget.
 * @param signal Parameter signal.
 * @return Return value.
 */
ui_error_t ui_scaffold_base_bind_data(struct ui_scaffold_base *widget,
                                      struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
