#ifndef CMP_F2_OVERLAYS_H
#define CMP_F2_OVERLAYS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

/* 7.1 Dialog / Modal */
typedef enum cmp_f2_dialog_variant {
  CMP_F2_DIALOG_VARIANT_STANDARD = 0,
  CMP_F2_DIALOG_VARIANT_ALERT,
  CMP_F2_DIALOG_VARIANT_FULLSCREEN
} cmp_f2_dialog_variant_t;

typedef struct cmp_f2_dialog_s {
  cmp_f2_dialog_variant_t variant;
  int is_open;

  cmp_ui_node_t *backdrop_node;
  cmp_ui_node_t *container_node;
  cmp_ui_node_t *title_node;
  cmp_ui_node_t *body_node;
  cmp_ui_node_t *footer_node;
} cmp_f2_dialog_t;

/**
 * @brief Initialize a Fluent 2 dialog_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_dialog_create(cmp_ui_node_t **out_node,
                         cmp_f2_dialog_variant_t variant);
CMP_API int cmp_f2_dialog_set_open(cmp_ui_node_t *node, int is_open);

/* 7.2 Flyout / Popover */
typedef enum cmp_f2_flyout_placement {
  CMP_F2_FLYOUT_PLACEMENT_TOP = 0,
  CMP_F2_FLYOUT_PLACEMENT_BOTTOM,
  CMP_F2_FLYOUT_PLACEMENT_LEFT,
  CMP_F2_FLYOUT_PLACEMENT_RIGHT
} cmp_f2_flyout_placement_t;

typedef struct cmp_f2_flyout_s {
  cmp_f2_flyout_placement_t placement;
  int is_open;

  cmp_ui_node_t *anchor_node;
  cmp_ui_node_t *container_node;
} cmp_f2_flyout_t;

/**
 * @brief Initialize a Fluent 2 flyout_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_flyout_create(cmp_ui_node_t **out_node, cmp_ui_node_t *anchor);
CMP_API int cmp_f2_flyout_set_placement(cmp_ui_node_t *node,
                                cmp_f2_flyout_placement_t placement);
CMP_API int cmp_f2_flyout_set_open(cmp_ui_node_t *node, int is_open);

/* 7.3 Tooltip */
typedef struct cmp_f2_tooltip_s {
  int is_visible;
  cmp_ui_node_t *anchor_node;
  cmp_ui_node_t *label_node;
} cmp_f2_tooltip_t;

/**
 * @brief Initialize a Fluent 2 tooltip_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_tooltip_create(cmp_ui_node_t **out_node, cmp_ui_node_t *anchor,
                          const char *label);
CMP_API int cmp_f2_tooltip_set_visible(cmp_ui_node_t *node, int is_visible);

/* 7.4 Toast / Notification */
typedef enum cmp_f2_toast_intent {
  CMP_F2_TOAST_INTENT_INFO = 0,
  CMP_F2_TOAST_INTENT_SUCCESS,
  CMP_F2_TOAST_INTENT_WARNING,
  CMP_F2_TOAST_INTENT_ERROR
} cmp_f2_toast_intent_t;

typedef struct cmp_f2_toast_s {
  cmp_f2_toast_intent_t intent;
  int is_visible;

  cmp_ui_node_t *icon_node;
  cmp_ui_node_t *title_node;
  cmp_ui_node_t *body_node;
  cmp_ui_node_t *actions_node;
  cmp_ui_node_t *close_button_node;
} cmp_f2_toast_t;

/**
 * @brief Initialize a Fluent 2 toast_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_toast_create(cmp_ui_node_t **out_node, cmp_f2_toast_intent_t intent);
CMP_API int cmp_f2_toast_set_visible(cmp_ui_node_t *node, int is_visible);

/* 7.5 MessageBar (Inline Alert) */
typedef enum cmp_f2_messagebar_variant {
  CMP_F2_MESSAGEBAR_VARIANT_SINGLELINE = 0,
  CMP_F2_MESSAGEBAR_VARIANT_MULTILINE
} cmp_f2_messagebar_variant_t;

typedef struct cmp_f2_messagebar_s {
  cmp_f2_messagebar_variant_t variant;
  cmp_f2_toast_intent_t intent;
} cmp_f2_messagebar_t;

/**
 * @brief Initialize a Fluent 2 messagebar_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_messagebar_create(cmp_ui_node_t **out_node,
                             cmp_f2_toast_intent_t intent);
CMP_API int cmp_f2_messagebar_set_variant(cmp_ui_node_t *node,
                                  cmp_f2_messagebar_variant_t variant);

/* 7.6 ProgressBar & ProgressRing */
typedef enum cmp_f2_progress_variant {
  CMP_F2_PROGRESS_VARIANT_DETERMINATE = 0,
  CMP_F2_PROGRESS_VARIANT_INDETERMINATE
} cmp_f2_progress_variant_t;

typedef enum cmp_f2_progress_state {
  CMP_F2_PROGRESS_STATE_ACTIVE = 0,
  CMP_F2_PROGRESS_STATE_PAUSED,
  CMP_F2_PROGRESS_STATE_ERROR
} cmp_f2_progress_state_t;

typedef struct cmp_f2_progress_s {
  cmp_f2_progress_variant_t variant;
  cmp_f2_progress_state_t state;
  float thickness;
  float value; /* 0.0 to 1.0 for determinate */
} cmp_f2_progress_t;

/**
 * @brief Initialize a Fluent 2 progress_bar_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_progress_bar_create(cmp_ui_node_t **out_node);
/**
 * @brief Initialize a Fluent 2 progress_ring_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_progress_ring_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_progress_set_variant(cmp_ui_node_t *node,
                                cmp_f2_progress_variant_t variant);
CMP_API int cmp_f2_progress_set_state(cmp_ui_node_t *node,
                              cmp_f2_progress_state_t state);
CMP_API int cmp_f2_progress_set_value(cmp_ui_node_t *node, float value);

/* 7.7 Skeleton / Shimmer */
typedef enum cmp_f2_skeleton_shape {
  CMP_F2_SKELETON_SHAPE_CIRCLE = 0,
  CMP_F2_SKELETON_SHAPE_RECTANGLE,
  CMP_F2_SKELETON_SHAPE_TEXT_LINE
} cmp_f2_skeleton_shape_t;

typedef struct cmp_f2_skeleton_s {
  cmp_f2_skeleton_shape_t shape;
  int is_animated;
} cmp_f2_skeleton_t;

/**
 * @brief Initialize a Fluent 2 skeleton_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_skeleton_create(cmp_ui_node_t **out_node,
                           cmp_f2_skeleton_shape_t shape);
CMP_API int cmp_f2_skeleton_set_animated(cmp_ui_node_t *node, int is_animated);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_OVERLAYS_H */
