/* clang-format off */
#include "ui_button_group_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_button_group_mock_fail = 0;
#endif

/**
 * @brief ui_button_group_base_create.
 * @param out_group Parameter out_group.
 * @return Return value.
 */
ui_error_t
ui_button_group_base_create(struct ui_button_group_base **out_group) {
  struct ui_button_group_base *group;
  struct ui_component *base_comp;
  ui_error_t err;

  if (!out_group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  group = (struct ui_button_group_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_button_group_base));
  if (!group) {
    (void)ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  group->base = *base_comp;
  C_MULTIPLATFORM_FREE(base_comp);

  group->is_vertical = 0;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &group->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(group);
    return err;
  }

  err = ui_dom_node_set_tag_name(group->base.shadow_root, "ui-button-group");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(group->base.shadow_root);
    C_MULTIPLATFORM_FREE(group);
    return err;
  }

  err = ui_dom_node_set_attribute(group->base.shadow_root, "role", "group");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(group->base.shadow_root);
    C_MULTIPLATFORM_FREE(group);
    return err;
  }
  err = ui_dom_node_set_attribute(group->base.shadow_root, "data-orientation",
                                  "horizontal");
#ifdef UI_TEST_MOCK_ALLOC
  if (g_button_group_mock_fail == 1)
    err = UI_ERROR_UNKNOWN;
#endif
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(group->base.shadow_root);
    C_MULTIPLATFORM_FREE(group);
    return err;
  }

  *out_group = group;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_button_group_base_set_orientation(struct ui_button_group_base *group,
                                     int is_vertical) {
  if (!group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  group->is_vertical = is_vertical;

  if (is_vertical) {
    return ui_dom_node_set_attribute(group->base.shadow_root,
                                     "data-orientation", "vertical");
  } else {
    return ui_dom_node_set_attribute(group->base.shadow_root,
                                     "data-orientation", "horizontal");
  }
}

/** \brief ui_error */
ui_error_t
ui_button_group_base_append_button(struct ui_button_group_base *group,
                                   struct ui_component *child) {
  if (!group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!child) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Note: Normally components mount to host nodes, but since this is a
     structural stub, we just append the shadow root (or host node) directly. To
     keep it simple in this test layer, we append the shadow_root. In a real
     system, we'd mount the component properly. */
  return ui_dom_node_append_child(group->base.shadow_root, child->shadow_root);
}

/** \brief ui_error */
ui_error_t
ui_button_group_base_bind_disabled(struct ui_button_group_base *widget,
                                   struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_button_group_base_bind_text(struct ui_button_group_base *widget,
                                          struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}
