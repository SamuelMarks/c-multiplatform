/* clang-format off */
#include "cmp_ui_text_field.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */
/* Text Field Colors */
#ifndef CMP_UI_TEXT_FIELD_BG_COLOR
#define CMP_UI_TEXT_FIELD_BG_COLOR 0xFFF5F5F5
#endif


struct cmp_ui_text_field {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_label;
  cmp_ui_node_t *node_input;
  char *label;
  char *value;
};

/**
 * @brief cmp_ui_text_field_create
 *
 * @param out_field Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_text_field_create(cmp_ui_text_field_t **out_field,
                             const char *label) {
  cmp_ui_text_field_t *field;
  int err;
  size_t len;

  if (!out_field) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_text_field_t), (void **)&field);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(field, 0, sizeof(cmp_ui_text_field_t));

  if (label) {
    len = strlen(label);
    err = CMP_MALLOC(len + 1, (void **)&field->label);
    if (err == CMP_SUCCESS) {
      memcpy(field->label, label, len + 1);
    } else {
      CMP_FREE(field);
      return err;
    }
  }

  err = cmp_ui_box_create(&field->node_root);
  if (err != CMP_SUCCESS) {
    if (field->label) CMP_FREE(field->label);
    CMP_FREE(field);
    return err;
  }

  field->node_root->layout->direction = CMP_FLEX_COLUMN;
  field->node_root->bg_color = CMP_UI_TEXT_FIELD_BG_COLOR;

  err = cmp_ui_text_create(&field->node_label, field->label ? field->label : "", -1);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(field->node_root);
    if (field->label) CMP_FREE(field->label);
    CMP_FREE(field);
    return err;
  }
  err = cmp_ui_node_add_child(field->node_root, field->node_label);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(field->node_label);
    cmp_ui_node_destroy(field->node_root);
    if (field->label) CMP_FREE(field->label);
    CMP_FREE(field);
    return err;
  }

  err = cmp_ui_text_input_create(&field->node_input);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(field->node_root);
    if (field->label) CMP_FREE(field->label);
    CMP_FREE(field);
    return err;
  }
  err = cmp_ui_node_add_child(field->node_root, field->node_input);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(field->node_input);
    cmp_ui_node_destroy(field->node_root);
    if (field->label) CMP_FREE(field->label);
    CMP_FREE(field);
    return err;
  }

  *out_field = field;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_text_field_destroy
 *
 * @param field Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_text_field_destroy(cmp_ui_text_field_t *field) {
  int rc = CMP_SUCCESS;
  if (!field) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (field->label)
    CMP_FREE(field->label);
  if (field->value)
    CMP_FREE(field->value);
  CMP_FREE(field);

  return rc;
}

/**
 * @brief cmp_ui_text_field_get_node
 *
 * @param field Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_text_field_get_node(cmp_ui_text_field_t *field,
                               cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!field || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = field->node_root;

  return rc;
}

/**
 * @brief cmp_ui_text_field_set_value
 *
 * @param field Parameter description.
 * @param value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_text_field_set_value(cmp_ui_text_field_t *field, const char *value) {
  int rc = CMP_SUCCESS;
  size_t len;
  int err;

  if (!field) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (field->value) {
    CMP_FREE(field->value);
    field->value = NULL;
  }

  if (value) {
    len = strlen(value);
    err = CMP_MALLOC(len + 1, (void **)&field->value);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(field->value, value, len + 1);

    /* For floating label logic: visually adjust the label position and size
     * based on if it's filled */
    /* This handles the logic side, style state change would happen here */
  }

  return rc;
}
/**
 * @brief Binds the widget to the accessibility tree.
 *
 * @param widget The component.
 * @param tree The accessibility tree.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_text_field_bind_a11y(cmp_ui_text_field_t *widget,
                                cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "textbox",
                              "Text Field");
  if (rc != 0) {
    return rc;
  }

  return rc;
}
