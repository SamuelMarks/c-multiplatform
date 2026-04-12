/* clang-format off */
#ifndef M3_NAVIGATION_H
#define M3_NAVIGATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cmp.h"
#include "material_catalog.h"
#include "m3_buttons.h"

/**
 * @brief Material 3 Top App Bar variants.
 */
typedef enum m3_top_app_bar_type {
    M3_TOP_APP_BAR_TYPE_CENTER_ALIGNED,
    M3_TOP_APP_BAR_TYPE_SMALL,
    M3_TOP_APP_BAR_TYPE_MEDIUM,
    M3_TOP_APP_BAR_TYPE_LARGE
} m3_top_app_bar_type_t;

/**
 * @brief Configuration for a Material 3 Top App Bar.
 */
typedef struct m3_top_app_bar_config {
    m3_top_app_bar_type_t type;  /**< App bar variant */
    const char* title;           /**< Title string */
    const char* nav_icon;        /**< Optional leading navigation icon */
    const char** action_icons;   /**< Array of trailing action icons */
    int action_count;            /**< Number of trailing action icons */
    int is_scrolled;             /**< 1 if content is scrolled beneath, 0 otherwise */
    m3_event_cb_t on_nav_click; /**< Navigation icon click callback */
    void* user_data;             /**< User data for callbacks */
} m3_top_app_bar_config_t;

/**
 * @brief Creates a Material 3 Top App Bar UI node.
 * 
 * @param state The material catalog state.
 * @param config The app bar configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return 0 on success, non-zero on error.
 */
int m3_top_app_bar_create(material_catalog_state_t* state, const m3_top_app_bar_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Configuration for a Material 3 Bottom App Bar.
 */
typedef struct m3_bottom_app_bar_config {
    const char** action_icons;   /**< Array of action icons */
    int action_count;            /**< Number of action icons */
    int has_fab;                 /**< 1 to include a FAB cutout/cradle or integrated FAB, 0 otherwise */
} m3_bottom_app_bar_config_t;

/**
 * @brief Creates a Material 3 Bottom App Bar UI node.
 * 
 * @param state The material catalog state.
 * @param config The bottom app bar configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return 0 on success, non-zero on error.
 */
int m3_bottom_app_bar_create(material_catalog_state_t* state, const m3_bottom_app_bar_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Material 3 Navigation Item (used in Bottom Nav, Rail, Drawer).
 */
typedef struct m3_nav_item {
    const char* icon;            /**< Unselected icon */
    const char* active_icon;     /**< Selected icon */
    const char* label;           /**< Item label */
    int badge_count;             /**< >0 to show large badge, 0 to show dot badge if enabled, -1 for no badge */
    int is_badge_dot;            /**< 1 to force dot badge if badge_count is 0 */
} m3_nav_item_t;

/**
 * @brief Material 3 Navigation Bar (Bottom) configuration.
 */
typedef struct m3_navigation_bar_config {
    const m3_nav_item_t* items;  /**< Array of navigation items (typically 3-5) */
    int item_count;              /**< Number of items */
    int selected_index;          /**< Currently selected item index */
    int hide_labels;             /**< 1 to hide all labels, 0 to show */
    void (*on_select)(int index, void* user_data); /**< Callback on selection */
    void* user_data;             /**< User data for callback */
} m3_navigation_bar_config_t;

/**
 * @brief Creates a Material 3 Navigation Bar (Bottom).
 * 
 * @param state The material catalog state.
 * @param config The navigation bar configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return 0 on success, non-zero on error.
 */
int m3_navigation_bar_create(material_catalog_state_t* state, const m3_navigation_bar_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Material 3 Navigation Rail alignment.
 */
typedef enum m3_nav_rail_align {
    M3_NAV_RAIL_ALIGN_TOP,
    M3_NAV_RAIL_ALIGN_CENTER,
    M3_NAV_RAIL_ALIGN_BOTTOM
} m3_nav_rail_align_t;

/**
 * @brief Material 3 Navigation Rail configuration.
 */
typedef struct m3_navigation_rail_config {
    const m3_nav_item_t* items;  /**< Array of navigation items (typically 3-7) */
    int item_count;              /**< Number of items */
    int selected_index;          /**< Currently selected item index */
    m3_nav_rail_align_t alignment; /**< Vertical alignment of items */
    int has_fab;                 /**< 1 to include a top FAB, 0 otherwise */
    int has_menu;                /**< 1 to include a top menu icon, 0 otherwise */
    void (*on_select)(int index, void* user_data); /**< Callback on selection */
    void* user_data;             /**< User data for callback */
} m3_navigation_rail_config_t;

/**
 * @brief Creates a Material 3 Navigation Rail UI node.
 * 
 * @param state The material catalog state.
 * @param config The navigation rail configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return 0 on success, non-zero on error.
 */
int m3_navigation_rail_create(material_catalog_state_t* state, const m3_navigation_rail_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Material 3 Navigation Drawer types.
 */
typedef enum m3_nav_drawer_type {
    M3_NAV_DRAWER_TYPE_STANDARD,
    M3_NAV_DRAWER_TYPE_MODAL
} m3_nav_drawer_type_t;

/**
 * @brief Material 3 Navigation Drawer configuration.
 */
typedef struct m3_navigation_drawer_config {
    m3_nav_drawer_type_t type;   /**< Standard or Modal */
    const m3_nav_item_t* items;  /**< Array of navigation items */
    int item_count;              /**< Number of items */
    int selected_index;          /**< Currently selected item index */
    const char* headline;        /**< Optional headline at the top */
    void (*on_select)(int index, void* user_data); /**< Callback on selection */
    void* user_data;             /**< User data for callback */
} m3_navigation_drawer_config_t;

/**
 * @brief Creates a Material 3 Navigation Drawer UI node.
 * 
 * @param state The material catalog state.
 * @param config The navigation drawer configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return 0 on success, non-zero on error.
 */
int m3_navigation_drawer_create(material_catalog_state_t* state, const m3_navigation_drawer_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Material 3 Tabs variants.
 */
typedef enum m3_tabs_type {
    M3_TABS_TYPE_PRIMARY,
    M3_TABS_TYPE_SECONDARY
} m3_tabs_type_t;

/**
 * @brief Material 3 Tabs configuration.
 */
typedef struct m3_tabs_config {
    m3_tabs_type_t type;         /**< Primary or Secondary */
    const char** tab_labels;     /**< Array of tab labels */
    const char** tab_icons;      /**< Optional array of tab icons (can be NULL) */
    int tab_count;               /**< Number of tabs */
    int selected_index;          /**< Currently active tab index */
    int is_scrollable;           /**< 1 if tabs can scroll horizontally, 0 if fixed width */
    void (*on_tab_select)(int index, void* user_data); /**< Callback on tab selection */
    void* user_data;             /**< User data for callback */
} m3_tabs_config_t;

/**
 * @brief Creates a Material 3 Tabs UI node.
 * 
 * @param state The material catalog state.
 * @param config The tabs configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return 0 on success, non-zero on error.
 */
int m3_tabs_create(material_catalog_state_t* state, const m3_tabs_config_t* config, cmp_ui_node_t** out_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_NAVIGATION_H */
/* clang-format on */