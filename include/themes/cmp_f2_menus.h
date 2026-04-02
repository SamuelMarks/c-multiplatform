#ifndef CMP_F2_MENUS_H
#define CMP_F2_MENUS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

/* 5.1 Menu (Context & Dropdown) */
typedef struct cmp_f2_menu_item_s {
  cmp_ui_node_t *leading_icon_node;
  cmp_ui_node_t *text_node;
  cmp_ui_node_t *shortcut_text_node;
  cmp_ui_node_t *trailing_chevron_node; /* For sub-menus */
  int is_disabled;
} cmp_f2_menu_item_t;

/**
 * @brief Initialize a Fluent 2 menu_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_menu_create(cmp_ui_node_t **out_node);
/**
 * @brief Initialize a Fluent 2 menu_item_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_menu_item_create(cmp_ui_node_t **out_node, const char *label);
CMP_API int cmp_f2_menu_item_set_shortcut(cmp_ui_node_t *node, const char *shortcut);
CMP_API int cmp_f2_menu_item_set_disabled(cmp_ui_node_t *node, int is_disabled);

/* 5.2 NavigationView (App Navigation) */
typedef enum cmp_f2_nav_state {
  CMP_F2_NAV_STATE_EXPANDED = 0,
  CMP_F2_NAV_STATE_COLLAPSED,
  CMP_F2_NAV_STATE_OVERLAY
} cmp_f2_nav_state_t;

typedef struct cmp_f2_nav_view_s {
  cmp_f2_nav_state_t state;
  cmp_ui_node_t *header_node;
  cmp_ui_node_t *content_node;
  cmp_ui_node_t *footer_node;
} cmp_f2_nav_view_t;

typedef struct cmp_f2_nav_item_s {
  int is_active;
  cmp_ui_node_t *icon_node;
  cmp_ui_node_t *label_node;
  cmp_ui_node_t *active_indicator_node;
} cmp_f2_nav_item_t;

/**
 * @brief Initialize a Fluent 2 nav_view_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_nav_view_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_nav_view_set_state(cmp_ui_node_t *node, cmp_f2_nav_state_t state);

/**
 * @brief Initialize a Fluent 2 nav_item_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_nav_item_create(cmp_ui_node_t **out_node, const char *label);
CMP_API int cmp_f2_nav_item_set_active(cmp_ui_node_t *node, int is_active);

/* 5.3 Tabs / Pivot */
typedef enum cmp_f2_tab_variant {
  CMP_F2_TAB_VARIANT_TRANSPARENT = 0,
  CMP_F2_TAB_VARIANT_SUBTLE,
  CMP_F2_TAB_VARIANT_SOLID
} cmp_f2_tab_variant_t;

typedef enum cmp_f2_tab_size {
  CMP_F2_TAB_SIZE_SMALL = 0,
  CMP_F2_TAB_SIZE_MEDIUM,
  CMP_F2_TAB_SIZE_LARGE
} cmp_f2_tab_size_t;

typedef struct cmp_f2_tab_s {
  cmp_f2_tab_variant_t variant;
  cmp_f2_tab_size_t size;
  int is_vertical;

  cmp_ui_node_t *header_area_node;
  cmp_ui_node_t
      *active_indicator_node; /* Renders underline or background pill */
} cmp_f2_tab_t;

/**
 * @brief Initialize a Fluent 2 tab_container_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_tab_container_create(cmp_ui_node_t **out_node, int is_vertical);
CMP_API int cmp_f2_tab_container_set_variant(cmp_ui_node_t *node,
                                     cmp_f2_tab_variant_t variant);
CMP_API int cmp_f2_tab_container_set_size(cmp_ui_node_t *node, cmp_f2_tab_size_t size);

/**
 * @brief Initialize a Fluent 2 tab_item_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_tab_item_create(cmp_ui_node_t **out_node, const char *label);
CMP_API int cmp_f2_tab_item_set_selected(cmp_ui_node_t *node, int is_selected);

/* 5.4 Breadcrumb */
typedef struct cmp_f2_breadcrumb_s {
  cmp_ui_node_t *container_node;
} cmp_f2_breadcrumb_t;

/**
 * @brief Initialize a Fluent 2 breadcrumb_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_breadcrumb_create(cmp_ui_node_t **out_node);
/**
 * @brief Initialize a Fluent 2 breadcrumb_item_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_breadcrumb_item_create(cmp_ui_node_t **out_node, const char *label,
                                  int is_last);

/* 5.5 Toolbar / CommandBar */
typedef struct cmp_f2_toolbar_s {
  cmp_ui_node_t *primary_commands_node;
  cmp_ui_node_t *secondary_commands_node;
  cmp_ui_node_t *overflow_menu_node;
} cmp_f2_toolbar_t;

/**
 * @brief Initialize a Fluent 2 toolbar_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_toolbar_create(cmp_ui_node_t **out_node);

/* 5.6 Link */
typedef enum cmp_f2_link_variant {
  CMP_F2_LINK_VARIANT_INLINE = 0,
  CMP_F2_LINK_VARIANT_STANDALONE
} cmp_f2_link_variant_t;

typedef struct cmp_f2_link_s {
  cmp_f2_link_variant_t variant;
  int is_disabled;
  int is_visited;

  cmp_ui_node_t *text_node;
  cmp_ui_node_t *external_icon_node;
} cmp_f2_link_t;

/**
 * @brief Initialize a Fluent 2 link_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_link_create(cmp_ui_node_t **out_node, const char *label,
                       cmp_f2_link_variant_t variant);
CMP_API int cmp_f2_link_set_disabled(cmp_ui_node_t *node, int is_disabled);

/* 5.7 Stepper / Wizard */
typedef enum cmp_f2_stepper_state {
  CMP_F2_STEPPER_STATE_COMPLETED = 0,
  CMP_F2_STEPPER_STATE_CURRENT,
  CMP_F2_STEPPER_STATE_UPCOMING
} cmp_f2_stepper_state_t;

typedef struct cmp_f2_stepper_item_s {
  cmp_f2_stepper_state_t state;
  cmp_ui_node_t *indicator_node;
  cmp_ui_node_t *label_node;
} cmp_f2_stepper_item_t;

/**
 * @brief Initialize a Fluent 2 stepper_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_stepper_create(cmp_ui_node_t **out_node);
/**
 * @brief Initialize a Fluent 2 stepper_item_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_stepper_item_create(cmp_ui_node_t **out_node, const char *label);
CMP_API int cmp_f2_stepper_item_set_state(cmp_ui_node_t *node,
                                  cmp_f2_stepper_state_t state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_MENUS_H */
