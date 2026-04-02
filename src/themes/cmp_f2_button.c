/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_button.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

static int cmp_f2_button_init_internal(cmp_ui_node_t **out_node,
                                       cmp_f2_button_type_t type,
                                       const char *label, cmp_ui_node_t *icon) {
  cmp_f2_button_t *btn_data;
  int res;

  if (!out_node) {
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_button_create(out_node, label, -1);
  if (res != CMP_SUCCESS) {
    return res;
  }

  btn_data = (cmp_f2_button_t *)malloc(sizeof(cmp_f2_button_t));
  if (!btn_data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  btn_data->type = type;
  btn_data->variant = CMP_F2_BUTTON_VARIANT_SECONDARY;
  btn_data->size = CMP_F2_BUTTON_SIZE_MEDIUM;
  btn_data->shape = CMP_F2_BUTTON_SHAPE_ROUNDED;
  btn_data->state = CMP_F2_BUTTON_STATE_REST;
  btn_data->is_toggled = 0;
  btn_data->is_menu_open = 0;

  btn_data->icon_node = icon;
  btn_data->text_node =
      NULL; /* Usually initialized inside cmp_ui_button_create */
  btn_data->secondary_text_node = NULL;
  btn_data->chevron_node = NULL;
  btn_data->divider_node = NULL;

  if (icon) {
    if (label) {
      btn_data->layout_mode = CMP_F2_BUTTON_LAYOUT_ICON_TEXT;
    } else {
      btn_data->layout_mode = CMP_F2_BUTTON_LAYOUT_ICON_ONLY;
    }
  } else {
    btn_data->layout_mode = CMP_F2_BUTTON_LAYOUT_TEXT_ONLY;
  }

  (*out_node)->properties = (void *)btn_data;

  /* Apply defaults */
  cmp_f2_button_set_size(*out_node, CMP_F2_BUTTON_SIZE_MEDIUM);
  cmp_f2_button_set_shape(*out_node, CMP_F2_BUTTON_SHAPE_ROUNDED);

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_button_create(cmp_ui_node_t **out_node, const char *label,
                         cmp_ui_node_t *icon) {
  return cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_STANDARD,
                                     label, icon);
}

CMP_API int cmp_f2_compound_button_create(cmp_ui_node_t **out_node,
                                  const char *primary_text,
                                  const char *secondary_text,
                                  cmp_ui_node_t *icon) {
  int res;
  cmp_f2_button_t *btn_data;

  res = cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_COMPOUND,
                                    primary_text, icon);
  if (res != CMP_SUCCESS)
    return res;

  btn_data = (cmp_f2_button_t *)(*out_node)->properties;

  if (secondary_text) {
    /* MOCK: create a secondary text node representing descriptive text */
    res =
        cmp_ui_text_create(&btn_data->secondary_text_node, secondary_text, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, btn_data->secondary_text_node);
    }
  }

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_menu_button_create(cmp_ui_node_t **out_node, const char *label,
                              cmp_ui_node_t *icon) {
  int res;
  cmp_f2_button_t *btn_data;

  res = cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_MENU, label,
                                    icon);
  if (res != CMP_SUCCESS)
    return res;

  btn_data = (cmp_f2_button_t *)(*out_node)->properties;

  /* MOCK: create trailing chevron icon */
  res = cmp_ui_text_create(&btn_data->chevron_node, "V",
                           -1); /* Fake Chevron down */
  if (res == CMP_SUCCESS) {
    cmp_ui_node_add_child(*out_node, btn_data->chevron_node);
  }

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_split_button_create(cmp_ui_node_t **out_node, const char *label,
                               cmp_ui_node_t *icon) {
  int res;
  cmp_f2_button_t *btn_data;

  res = cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_SPLIT, label,
                                    icon);
  if (res != CMP_SUCCESS)
    return res;

  btn_data = (cmp_f2_button_t *)(*out_node)->properties;

  /* MOCK: create divider and chevron */
  res = cmp_ui_box_create(&btn_data->divider_node);
  if (res == CMP_SUCCESS) {
    btn_data->divider_node->layout->width = 1.0f; /* 1px split line */
    cmp_ui_node_add_child(*out_node, btn_data->divider_node);
  }

  res = cmp_ui_text_create(&btn_data->chevron_node, "V", -1); /* Fake Chevron */
  if (res == CMP_SUCCESS) {
    cmp_ui_node_add_child(*out_node, btn_data->chevron_node);
  }

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_toggle_button_create(cmp_ui_node_t **out_node, const char *label,
                                cmp_ui_node_t *icon) {
  return cmp_f2_button_init_internal(out_node, CMP_F2_BUTTON_TYPE_TOGGLE, label,
                                     icon);
}

CMP_API int cmp_f2_button_set_variant(cmp_ui_node_t *node,
                              cmp_f2_button_variant_t variant) {
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  btn_data = (cmp_f2_button_t *)node->properties;
  btn_data->variant = variant;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_button_set_size(cmp_ui_node_t *node, cmp_f2_button_size_t size) {
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  btn_data = (cmp_f2_button_t *)node->properties;
  btn_data->size = size;

  /* Fluent 2 Button Sizes */
  if (size == CMP_F2_BUTTON_SIZE_SMALL) {
    node->layout->height = 24.0f;
  } else if (size == CMP_F2_BUTTON_SIZE_LARGE) {
    node->layout->height = 40.0f;
  } else {
    node->layout->height = 32.0f; /* Medium default */
  }
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_button_set_shape(cmp_ui_node_t *node, cmp_f2_button_shape_t shape) {
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  btn_data = (cmp_f2_button_t *)node->properties;
  btn_data->shape = shape;
  /* We will interpret this dynamically during draw */
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_button_set_state(cmp_ui_node_t *node, cmp_f2_button_state_t state) {
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  btn_data = (cmp_f2_button_t *)node->properties;
  btn_data->state = state;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_button_set_toggled(cmp_ui_node_t *node, int is_toggled) {
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  btn_data = (cmp_f2_button_t *)node->properties;
  if (btn_data->type != CMP_F2_BUTTON_TYPE_TOGGLE) {
    return CMP_ERROR_INVALID_STATE;
  }
  btn_data->is_toggled = is_toggled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_button_set_menu_open(cmp_ui_node_t *node, int is_open) {
  cmp_f2_button_t *btn_data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  btn_data = (cmp_f2_button_t *)node->properties;

  if (btn_data->type != CMP_F2_BUTTON_TYPE_MENU &&
      btn_data->type != CMP_F2_BUTTON_TYPE_SPLIT) {
    return CMP_ERROR_INVALID_STATE;
  }

  btn_data->is_menu_open = is_open ? 1 : 0;

  /* In an actual implementation, this rotates the chevron icon node 180 degrees
   * via `style.transform` */

  return CMP_SUCCESS;
}
