#ifndef CMP_F2_SURFACES_H
#define CMP_F2_SURFACES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

/* 6.1 Card */
typedef enum cmp_f2_card_variant {
  CMP_F2_CARD_VARIANT_FILLED = 0,
  CMP_F2_CARD_VARIANT_OUTLINE,
  CMP_F2_CARD_VARIANT_SUBTLE
} cmp_f2_card_variant_t;

typedef enum cmp_f2_card_orientation {
  CMP_F2_CARD_ORIENTATION_VERTICAL = 0,
  CMP_F2_CARD_ORIENTATION_HORIZONTAL
} cmp_f2_card_orientation_t;

/** \brief Documented */
typedef struct cmp_f2_card_s {
  /** \brief Documented */
  cmp_f2_card_variant_t variant;
  /** \brief Documented */
  cmp_f2_card_orientation_t orientation;
  /** \brief Documented */
  int is_clickable;

  /** \brief Documented */
  cmp_ui_node_t *header_node;
  /** \brief Documented */
  cmp_ui_node_t *preview_node;
  /** \brief Documented */
  cmp_ui_node_t *body_node;
  /** \brief Documented */
  cmp_ui_node_t *footer_node;
} cmp_f2_card_t;

/**
 * @brief Initialize a Fluent 2 card_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_card_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_card_set_variant(cmp_ui_node_t *node,
                                    cmp_f2_card_variant_t variant);
CMP_API int cmp_f2_card_set_orientation(cmp_ui_node_t *node,
                                        cmp_f2_card_orientation_t orientation);
CMP_API int cmp_f2_card_set_clickable(cmp_ui_node_t *node, int is_clickable);

/* 6.2 Accordion */
typedef enum cmp_f2_accordion_variant {
  CMP_F2_ACCORDION_VARIANT_FLUSH = 0,
  CMP_F2_ACCORDION_VARIANT_CONTAINED
} cmp_f2_accordion_variant_t;

/** \brief Documented */
typedef struct cmp_f2_accordion_s {
  /** \brief Documented */
  cmp_f2_accordion_variant_t variant;
  /** \brief Documented */
  int multi_expand;
} cmp_f2_accordion_t;

/** \brief Documented */
typedef struct cmp_f2_accordion_panel_s {
  /** \brief Documented */
  int is_expanded;

  /** \brief Documented */
  cmp_ui_node_t *header_node;
  /** \brief Documented */
  cmp_ui_node_t *chevron_node;
  /** \brief Documented */
  cmp_ui_node_t *content_body_node;
} cmp_f2_accordion_panel_t;

/**
 * @brief Initialize a Fluent 2 accordion_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_accordion_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_accordion_set_variant(cmp_ui_node_t *node,
                                         cmp_f2_accordion_variant_t variant);
CMP_API int cmp_f2_accordion_set_multi_expand(cmp_ui_node_t *node,
                                              int multi_expand);

/**
 * @brief Initialize a Fluent 2 accordion_panel_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_accordion_panel_create(cmp_ui_node_t **out_node,
                                          const char *header_label);
CMP_API int cmp_f2_accordion_panel_set_expanded(cmp_ui_node_t *node,
                                                int is_expanded);

/* 6.3 Divider */
typedef enum cmp_f2_divider_orientation {
  CMP_F2_DIVIDER_HORIZONTAL = 0,
  CMP_F2_DIVIDER_VERTICAL
} cmp_f2_divider_orientation_t;

typedef enum cmp_f2_divider_appearance {
  CMP_F2_DIVIDER_APPEARANCE_DEFAULT = 0,
  CMP_F2_DIVIDER_APPEARANCE_SUBTLE,
  CMP_F2_DIVIDER_APPEARANCE_BRAND
} cmp_f2_divider_appearance_t;

/** \brief Documented */
typedef struct cmp_f2_divider_s {
  /** \brief Documented */
  cmp_f2_divider_orientation_t orientation;
  /** \brief Documented */
  cmp_f2_divider_appearance_t appearance;
  /** \brief Documented */
  int is_inset;
} cmp_f2_divider_t;

/**
 * @brief Initialize a Fluent 2 divider_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_divider_create(cmp_ui_node_t **out_node);
CMP_API int
cmp_f2_divider_set_orientation(cmp_ui_node_t *node,
                               cmp_f2_divider_orientation_t orientation);
CMP_API int
cmp_f2_divider_set_appearance(cmp_ui_node_t *node,
                              cmp_f2_divider_appearance_t appearance);
CMP_API int cmp_f2_divider_set_inset(cmp_ui_node_t *node, int is_inset);

/* 6.4 SplitView / Resizer */
/** \brief Documented */
typedef struct cmp_f2_split_view_s {
  /** \brief Documented */
  cmp_ui_node_t *pane_1_node;
  /** \brief Documented */
  cmp_ui_node_t *resizer_node;
  /** \brief Documented */
  cmp_ui_node_t *pane_2_node;
} cmp_f2_split_view_t;

/**
 * @brief Initialize a Fluent 2 split_view_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_split_view_create(cmp_ui_node_t **out_node);

/* 6.5 ScrollView / Scrollbar */
typedef enum cmp_f2_scrollbar_behavior {
  CMP_F2_SCROLLBAR_BEHAVIOR_AUTO_HIDE = 0,
  CMP_F2_SCROLLBAR_BEHAVIOR_ALWAYS_ON
} cmp_f2_scrollbar_behavior_t;

/** \brief Documented */
typedef struct cmp_f2_scroll_view_s {
  /** \brief Documented */
  cmp_f2_scrollbar_behavior_t behavior;
  /** \brief Documented */
  int is_overlay;

  /** \brief Documented */
  cmp_ui_node_t *content_wrapper_node;
  /** \brief Documented */
  cmp_ui_node_t *vertical_scrollbar_node;
  /** \brief Documented */
  cmp_ui_node_t *horizontal_scrollbar_node;
} cmp_f2_scroll_view_t;

/**
 * @brief Initialize a Fluent 2 scroll_view_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_scroll_view_create(cmp_ui_node_t **out_node);
CMP_API int
cmp_f2_scroll_view_set_behavior(cmp_ui_node_t *node,
                                cmp_f2_scrollbar_behavior_t behavior);
CMP_API int cmp_f2_scroll_view_set_overlay_mode(cmp_ui_node_t *node,
                                                int is_overlay);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_SURFACES_H */
