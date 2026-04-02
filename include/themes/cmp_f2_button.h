#ifndef CMP_F2_BUTTON_H
#define CMP_F2_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

typedef enum cmp_f2_button_variant {
  CMP_F2_BUTTON_VARIANT_SECONDARY = 0,
  CMP_F2_BUTTON_VARIANT_PRIMARY,
  CMP_F2_BUTTON_VARIANT_SUBTLE,
  CMP_F2_BUTTON_VARIANT_OUTLINE,
  CMP_F2_BUTTON_VARIANT_TRANSPARENT
} cmp_f2_button_variant_t;

typedef enum cmp_f2_button_size {
  CMP_F2_BUTTON_SIZE_MEDIUM = 0,
  CMP_F2_BUTTON_SIZE_SMALL,
  CMP_F2_BUTTON_SIZE_LARGE
} cmp_f2_button_size_t;

typedef enum cmp_f2_button_shape {
  CMP_F2_BUTTON_SHAPE_ROUNDED = 0,
  CMP_F2_BUTTON_SHAPE_CIRCULAR,
  CMP_F2_BUTTON_SHAPE_SQUARE
} cmp_f2_button_shape_t;

typedef enum cmp_f2_button_state {
  CMP_F2_BUTTON_STATE_REST = 0,
  CMP_F2_BUTTON_STATE_HOVER,
  CMP_F2_BUTTON_STATE_PRESSED,
  CMP_F2_BUTTON_STATE_FOCUS,
  CMP_F2_BUTTON_STATE_DISABLED
} cmp_f2_button_state_t;

typedef enum cmp_f2_button_layout {
  CMP_F2_BUTTON_LAYOUT_TEXT_ONLY = 0,
  CMP_F2_BUTTON_LAYOUT_ICON_TEXT,
  CMP_F2_BUTTON_LAYOUT_TEXT_ICON,
  CMP_F2_BUTTON_LAYOUT_ICON_ONLY
} cmp_f2_button_layout_t;

typedef enum cmp_f2_button_type {
  CMP_F2_BUTTON_TYPE_STANDARD = 0,
  CMP_F2_BUTTON_TYPE_COMPOUND,
  CMP_F2_BUTTON_TYPE_MENU,
  CMP_F2_BUTTON_TYPE_SPLIT,
  CMP_F2_BUTTON_TYPE_TOGGLE
} cmp_f2_button_type_t;

typedef struct cmp_f2_button_s {
  cmp_f2_button_type_t type;
  cmp_f2_button_variant_t variant;
  cmp_f2_button_size_t size;
  cmp_f2_button_shape_t shape;
  cmp_f2_button_state_t state;
  cmp_f2_button_layout_t layout_mode;

  /* Type-specific state */
  int is_toggled;   /* For ToggleButton */
  int is_menu_open; /* For MenuButton and SplitButton */

  /* Component sub-nodes */
  cmp_ui_node_t *icon_node;
  cmp_ui_node_t *text_node;
  cmp_ui_node_t *secondary_text_node; /* For CompoundButton */
  cmp_ui_node_t *chevron_node;        /* For MenuButton / SplitButton */
  cmp_ui_node_t *divider_node;        /* For SplitButton */
} cmp_f2_button_t;

/**
 * @brief Initialize a Standard Fluent 2 Button.
 */
/**
 * @brief Initialize a Fluent 2 button_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_button_create(cmp_ui_node_t **out_node, const char *label,
                         cmp_ui_node_t *icon);

/**
 * @brief Initialize a Fluent 2 CompoundButton (primary + secondary text).
 */
/**
 * @brief Initialize a Fluent 2 compound_button_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_compound_button_create(cmp_ui_node_t **out_node,
                                  const char *primary_text,
                                  const char *secondary_text,
                                  cmp_ui_node_t *icon);

/**
 * @brief Initialize a Fluent 2 MenuButton (contains a trailing chevron and
 * toggles open state).
 */
/**
 * @brief Initialize a Fluent 2 menu_button_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_menu_button_create(cmp_ui_node_t **out_node, const char *label,
                              cmp_ui_node_t *icon);

/**
 * @brief Initialize a Fluent 2 SplitButton (two hit targets, divider, chevron).
 */
/**
 * @brief Initialize a Fluent 2 split_button_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_split_button_create(cmp_ui_node_t **out_node, const char *label,
                               cmp_ui_node_t *icon);

/**
 * @brief Initialize a Fluent 2 ToggleButton (on/off state).
 */
/**
 * @brief Initialize a Fluent 2 toggle_button_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_toggle_button_create(cmp_ui_node_t **out_node, const char *label,
                                cmp_ui_node_t *icon);

CMP_API int cmp_f2_button_set_variant(cmp_ui_node_t *node,
                              cmp_f2_button_variant_t variant);
CMP_API int cmp_f2_button_set_size(cmp_ui_node_t *node, cmp_f2_button_size_t size);
CMP_API int cmp_f2_button_set_shape(cmp_ui_node_t *node, cmp_f2_button_shape_t shape);
CMP_API int cmp_f2_button_set_state(cmp_ui_node_t *node, cmp_f2_button_state_t state);
CMP_API int cmp_f2_button_set_toggled(cmp_ui_node_t *node, int is_toggled);
CMP_API int cmp_f2_button_set_menu_open(cmp_ui_node_t *node, int is_open);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_BUTTON_H */
