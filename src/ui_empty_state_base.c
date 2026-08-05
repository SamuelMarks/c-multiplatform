/* clang-format off */
#include "ui_empty_state_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

ui_error_t ui_empty_state_base_create(struct ui_empty_state_base **out_state) {
  struct ui_empty_state_base *state;
  struct ui_component *base_comp;
  ui_error_t err;

  if (!out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  state = (struct ui_empty_state_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_empty_state_base));
  if (!state) {
    (void)ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  state->base = *base_comp;
  C_MULTIPLATFORM_FREE(base_comp);

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &state->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(state);
    return err;
  }

  err = ui_dom_node_set_tag_name(state->base.shadow_root, "ui-empty-state");
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(state->base.shadow_root);
    C_MULTIPLATFORM_FREE(state);
    return err;
  }

  *out_state = state;
  return UI_ERROR_NONE;
}

ui_error_t ui_empty_state_base_set_title(struct ui_empty_state_base *state,
                                         const char *text) {
  if (!state || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_set_attribute(state->base.shadow_root, "data-title", text);
}

/** \brief ui_error */
ui_error_t
ui_empty_state_base_set_description(struct ui_empty_state_base *state,
                                    const char *text) {
  if (!state || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_set_attribute(state->base.shadow_root, "data-description",
                                   text);
}

ui_error_t ui_empty_state_base_bind_data(struct ui_empty_state_base *widget,
                                         struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
