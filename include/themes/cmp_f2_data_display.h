#ifndef CMP_F2_DATA_DISPLAY_H
#define CMP_F2_DATA_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

/* 8.1 Avatar */
typedef enum cmp_f2_avatar_size {
  CMP_F2_AVATAR_SIZE_16 = 16,
  CMP_F2_AVATAR_SIZE_20 = 20,
  CMP_F2_AVATAR_SIZE_24 = 24,
  CMP_F2_AVATAR_SIZE_28 = 28,
  CMP_F2_AVATAR_SIZE_32 = 32,
  CMP_F2_AVATAR_SIZE_36 = 36,
  CMP_F2_AVATAR_SIZE_40 = 40,
  CMP_F2_AVATAR_SIZE_48 = 48,
  CMP_F2_AVATAR_SIZE_56 = 56,
  CMP_F2_AVATAR_SIZE_64 = 64,
  CMP_F2_AVATAR_SIZE_72 = 72,
  CMP_F2_AVATAR_SIZE_96 = 96,
  CMP_F2_AVATAR_SIZE_120 = 120
} cmp_f2_avatar_size_t;

typedef enum cmp_f2_avatar_shape {
  CMP_F2_AVATAR_SHAPE_CIRCULAR = 0,
  CMP_F2_AVATAR_SHAPE_SQUARE
} cmp_f2_avatar_shape_t;

/** \brief Documented */
typedef struct cmp_f2_avatar_s {
  /** \brief Documented */
  cmp_f2_avatar_size_t size;
  /** \brief Documented */
  cmp_f2_avatar_shape_t shape;

  /** \brief Documented */
  cmp_ui_node_t *image_node;
  /** \brief Documented */
  cmp_ui_node_t *initials_node;
  /** \brief Documented */
  cmp_ui_node_t *icon_node;
} cmp_f2_avatar_t;

/**
 * @brief Initialize a Fluent 2 Avatar component.
 * @param out_node Pointer to receive the allocated node.
 * @param size Predefined avatar size.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 avatar_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 avatar_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_avatar_create(cmp_ui_node_t **out_node,
                                 cmp_f2_avatar_size_t size,
                                 cmp_f2_avatar_shape_t shape);
CMP_API int cmp_f2_avatar_set_initials(cmp_ui_node_t *node,
                                       const char *initials);

/* 8.2 AvatarGroup */
typedef enum cmp_f2_avatar_group_layout {
  CMP_F2_AVATAR_GROUP_LAYOUT_STACKED = 0,
  CMP_F2_AVATAR_GROUP_LAYOUT_PIE
} cmp_f2_avatar_group_layout_t;

/** \brief Documented */
typedef struct cmp_f2_avatar_group_s {
  /** \brief Documented */
  cmp_f2_avatar_group_layout_t layout;
  /** \brief Documented */
  int max_count;
  /** \brief Documented */
  int current_count;

  /** \brief Documented */
  cmp_ui_node_t *overflow_node;
} cmp_f2_avatar_group_t;

/**
 * @brief Initialize a Fluent 2 avatar_group_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 avatar_group_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_avatar_group_create(cmp_ui_node_t **out_node,
                                       cmp_f2_avatar_group_layout_t layout,
                                       int max_count);

/* 8.3 PresenceBadge */
typedef enum cmp_f2_presence_status {
  CMP_F2_PRESENCE_AVAILABLE = 0,
  CMP_F2_PRESENCE_BUSY,
  CMP_F2_PRESENCE_DND,
  CMP_F2_PRESENCE_AWAY,
  CMP_F2_PRESENCE_OFFLINE,
  CMP_F2_PRESENCE_OOF
} cmp_f2_presence_status_t;

/** \brief Documented */
typedef struct cmp_f2_presence_badge_s {
  /** \brief Documented */
  cmp_f2_presence_status_t status;
} cmp_f2_presence_badge_t;

/**
 * @brief Initialize a Fluent 2 presence_badge_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 presence_badge_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_presence_badge_create(cmp_ui_node_t **out_node,
                                         cmp_f2_presence_status_t status);

/* 8.4 Badge & InfoLabel */
typedef enum cmp_f2_badge_variant {
  CMP_F2_BADGE_VARIANT_FILLED = 0,
  CMP_F2_BADGE_VARIANT_OUTLINE,
  CMP_F2_BADGE_VARIANT_TINT,
  CMP_F2_BADGE_VARIANT_GHOST
} cmp_f2_badge_variant_t;

typedef enum cmp_f2_badge_shape {
  CMP_F2_BADGE_SHAPE_ROUNDED = 0,
  CMP_F2_BADGE_SHAPE_CIRCULAR
} cmp_f2_badge_shape_t;

/** \brief Documented */
typedef struct cmp_f2_badge_s {
  /** \brief Documented */
  cmp_f2_badge_variant_t variant;
  /** \brief Documented */
  cmp_f2_badge_shape_t shape;

  /** \brief Documented */
  cmp_ui_node_t *text_node;
} cmp_f2_badge_t;

/**
 * @brief Initialize a Fluent 2 badge_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 badge_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_badge_create(cmp_ui_node_t **out_node, const char *label);
CMP_API int cmp_f2_badge_set_variant(cmp_ui_node_t *node,
                                     cmp_f2_badge_variant_t variant);
CMP_API int cmp_f2_badge_set_shape(cmp_ui_node_t *node,
                                   cmp_f2_badge_shape_t shape);

/* 8.5 Tag / TagGroup */
/** \brief Documented */
typedef struct cmp_f2_tag_s {
  /** \brief Documented */
  int is_dismissible;
  /** \brief Documented */
  int is_selected;

  /** \brief Documented */
  cmp_ui_node_t *text_node;
  /** \brief Documented */
  cmp_ui_node_t *dismiss_button_node;
} cmp_f2_tag_t;

/**
 * @brief Initialize a Fluent 2 tag_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 tag_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_tag_create(cmp_ui_node_t **out_node, const char *label,
                              int is_dismissible);
CMP_API int cmp_f2_tag_set_selected(cmp_ui_node_t *node, int is_selected);

/* 8.6 DataGrid / Table */
/** \brief Documented */
typedef struct cmp_f2_datagrid_s {
  /** \brief Documented */
  cmp_ui_node_t *header_row_node;
  /** \brief Documented */
  cmp_ui_node_t *rows_container_node;
} cmp_f2_datagrid_t;

/**
 * @brief Initialize a Fluent 2 datagrid_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 datagrid_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_datagrid_create(cmp_ui_node_t **out_node);
/**
 * @brief Initialize a Fluent 2 datagrid_row component.
 * @param out_node Pointer to receive the allocated node.
 * @param is_header Whether this is a header row.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_datagrid_row_create(cmp_ui_node_t **out_node, int is_header);

/* 8.7 Tree / TreeView */
/** \brief Documented */
typedef struct cmp_f2_tree_item_s {
  /** \brief Documented */
  int is_expanded;
  /** \brief Documented */
  int is_selected;

  /** \brief Documented */
  cmp_ui_node_t *chevron_node;
  /** \brief Documented */
  cmp_ui_node_t *content_node;
  /** \brief Documented */
  cmp_ui_node_t *children_container_node;
} cmp_f2_tree_item_t;

/**
 * @brief Initialize a Fluent 2 tree_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 tree_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_tree_create(cmp_ui_node_t **out_node);
/**
 * @brief Initialize a Fluent 2 tree_item_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 tree_item_create component.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_tree_item_create(cmp_ui_node_t **out_node,
                                    const char *label);
CMP_API int cmp_f2_tree_item_set_expanded(cmp_ui_node_t *node, int is_expanded);
CMP_API int cmp_f2_tree_item_set_selected(cmp_ui_node_t *node, int is_selected);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_DATA_DISPLAY_H */
