#ifndef CMP_F2_TEXT_INPUTS_H
#define CMP_F2_TEXT_INPUTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

typedef enum cmp_f2_text_input_variant {
  CMP_F2_TEXT_INPUT_VARIANT_OUTLINE = 0,
  CMP_F2_TEXT_INPUT_VARIANT_UNDERLINE,
  CMP_F2_TEXT_INPUT_VARIANT_FILLED
} cmp_f2_text_input_variant_t;

typedef enum cmp_f2_text_input_size {
  CMP_F2_TEXT_INPUT_SIZE_MEDIUM = 0,
  CMP_F2_TEXT_INPUT_SIZE_SMALL,
  CMP_F2_TEXT_INPUT_SIZE_LARGE
} cmp_f2_text_input_size_t;

typedef enum cmp_f2_text_input_state {
  CMP_F2_TEXT_INPUT_STATE_REST = 0,
  CMP_F2_TEXT_INPUT_STATE_HOVER,
  CMP_F2_TEXT_INPUT_STATE_FOCUS,
  CMP_F2_TEXT_INPUT_STATE_DISABLED,
  CMP_F2_TEXT_INPUT_STATE_INVALID
} cmp_f2_text_input_state_t;

/* 3.1 Input / TextField */
typedef struct cmp_f2_text_input_s {
  cmp_f2_text_input_variant_t variant;
  cmp_f2_text_input_size_t size;
  cmp_f2_text_input_state_t state;
  int is_password;

  /* Component sub-nodes */
  cmp_ui_node_t *leading_icon_node;
  cmp_ui_node_t *trailing_icon_node;
  cmp_ui_node_t *clear_button_node;
} cmp_f2_text_input_t;

/**
 * @brief Initialize a Fluent 2 text_input_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_text_input_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_text_input_set_variant(cmp_ui_node_t *node,
                                  cmp_f2_text_input_variant_t variant);
CMP_API int cmp_f2_text_input_set_size(cmp_ui_node_t *node,
                               cmp_f2_text_input_size_t size);
CMP_API int cmp_f2_text_input_set_state(cmp_ui_node_t *node,
                                cmp_f2_text_input_state_t state);
CMP_API int cmp_f2_text_input_set_password_mode(cmp_ui_node_t *node, int is_password);

/* 3.2 Textarea */
typedef enum cmp_f2_textarea_resize {
  CMP_F2_TEXTAREA_RESIZE_NONE = 0,
  CMP_F2_TEXTAREA_RESIZE_VERTICAL,
  CMP_F2_TEXTAREA_RESIZE_HORIZONTAL,
  CMP_F2_TEXTAREA_RESIZE_BOTH
} cmp_f2_textarea_resize_t;

typedef struct cmp_f2_textarea_s {
  cmp_f2_text_input_size_t size;
  cmp_f2_text_input_state_t state;
  cmp_f2_textarea_resize_t resize_mode;
  int auto_grow;
  float max_height;
} cmp_f2_textarea_t;

/**
 * @brief Initialize a Fluent 2 textarea_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_textarea_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_textarea_set_size(cmp_ui_node_t *node,
                             cmp_f2_text_input_size_t size);
CMP_API int cmp_f2_textarea_set_state(cmp_ui_node_t *node,
                              cmp_f2_text_input_state_t state);
CMP_API int cmp_f2_textarea_set_resize_mode(cmp_ui_node_t *node,
                                    cmp_f2_textarea_resize_t resize_mode);
CMP_API int cmp_f2_textarea_set_auto_grow(cmp_ui_node_t *node, int auto_grow,
                                  float max_height);

/* 3.3 Field & Label */
typedef enum cmp_f2_field_label_position {
  CMP_F2_FIELD_LABEL_TOP = 0,
  CMP_F2_FIELD_LABEL_LEFT
} cmp_f2_field_label_position_t;

typedef struct cmp_f2_field_s {
  cmp_f2_field_label_position_t label_position;
  int is_required;
  cmp_f2_text_input_state_t validation_state;

  cmp_ui_node_t *label_node;
  cmp_ui_node_t *input_node;
  cmp_ui_node_t *help_text_node;
  cmp_ui_node_t *validation_message_node;
} cmp_f2_field_t;

/**
 * @brief Initialize a Fluent 2 field_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_field_create(cmp_ui_node_t **out_node, cmp_ui_node_t *input_node,
                        const char *label);
CMP_API int cmp_f2_field_set_label_position(cmp_ui_node_t *node,
                                    cmp_f2_field_label_position_t pos);
CMP_API int cmp_f2_field_set_required(cmp_ui_node_t *node, int is_required);
CMP_API int cmp_f2_field_set_help_text(cmp_ui_node_t *node, const char *help_text);
CMP_API int cmp_f2_field_set_validation_message(cmp_ui_node_t *node,
                                        const char *message,
                                        cmp_f2_text_input_state_t state);

/* 3.4 SearchBox */
typedef struct cmp_f2_searchbox_s {
  cmp_f2_text_input_size_t size;
  cmp_f2_text_input_state_t state;
  int is_collapsed; /* Icon only mode until clicked */
} cmp_f2_searchbox_t;

/**
 * @brief Initialize a Fluent 2 searchbox_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_searchbox_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_searchbox_set_size(cmp_ui_node_t *node,
                              cmp_f2_text_input_size_t size);
CMP_API int cmp_f2_searchbox_set_state(cmp_ui_node_t *node,
                               cmp_f2_text_input_state_t state);
CMP_API int cmp_f2_searchbox_set_collapsed_mode(cmp_ui_node_t *node, int is_collapsed);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_TEXT_INPUTS_H */
