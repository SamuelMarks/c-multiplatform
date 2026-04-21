/* clang-format off */
#ifndef M3_BUTTONS_H
#define M3_BUTTONS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cmp.h"
#include "material_catalog.h"

/**
 * @brief Types of standard Material 3 buttons.
 */
typedef enum m3_button_type {
    M3_BUTTON_TYPE_ELEVATED,
    M3_BUTTON_TYPE_FILLED,
    M3_BUTTON_TYPE_FILLED_TONAL,
    M3_BUTTON_TYPE_OUTLINED,
    M3_BUTTON_TYPE_TEXT
} m3_button_type_t;

typedef void (*m3_event_cb_t)(cmp_event_t*, cmp_ui_node_t*, void*);

/**
 * @brief Material 3 Button configuration structure.
 */
typedef struct m3_button_config {
    m3_button_type_t type;       /**< The button visual style */
    const char* label;           /**< The text label */
    const char* leading_icon;    /**< Optional leading icon name/path */
    const char* trailing_icon;   /**< Optional trailing icon name/path */
    int is_disabled;             /**< 1 if disabled, 0 otherwise */
    m3_event_cb_t on_click;      /**< Callback fired on click */
    void* user_data;             /**< User data passed to callback */
} m3_button_config_t;

/**
 * @brief Creates a new Material 3 standard button.
 * 
 * @param state The material catalog state.
 * @param config The button configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return int 0 on success, non-zero on error.
 */
int m3_button_create(material_catalog_state_t* state, const m3_button_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Material 3 Floating Action Button (FAB) sizes.
 */
typedef enum m3_fab_size {
    M3_FAB_SIZE_SMALL,
    M3_FAB_SIZE_STANDARD,
    M3_FAB_SIZE_LARGE
} m3_fab_size_t;

/**
 * @brief Material 3 FAB configuration structure.
 */
typedef struct m3_fab_config {
    m3_fab_size_t size;          /**< The FAB size */
    const char* icon;            /**< The icon name/path */
    const char* label;           /**< The label (only for extended FAB, NULL otherwise) */
    int is_lowered;              /**< 1 if lowered elevation, 0 for default */
    m3_event_cb_t on_click;     /**< Callback fired on click */
    void* user_data;             /**< User data passed to callback */
} m3_fab_config_t;

/**
 * @brief Creates a new Material 3 Floating Action Button.
 * 
 * @param state The material catalog state.
 * @param config The FAB configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return int 0 on success, non-zero on error.
 */
int m3_fab_create(material_catalog_state_t* state, const m3_fab_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Creates a new Material 3 Icon Button.
 * 
 * @param state The material catalog state.
 * @param type The button type (ELEVATED, FILLED, FILLED_TONAL, OUTLINED, TEXT).
 * @param icon The icon name/path.
 * @param is_disabled 1 if disabled, 0 otherwise.
 * @param on_click Callback fired on click.
 * @param user_data User data passed to callback.
 * @param out_node Pointer to receive the created UI node.
 * @return int 0 on success, non-zero on error.
 */
int m3_icon_button_create(material_catalog_state_t* state, m3_button_type_t type, const char* icon, int is_disabled, m3_event_cb_t on_click, void* user_data, cmp_ui_node_t** out_node);

/**
 * @brief Creates a Material 3 Segmented Button Group.
 * 
 * @param state The material catalog state.
 * @param segments Array of strings for each segment label.
 * @param segment_count Number of segments.
 * @param is_multi_select 1 if multiple segments can be selected, 0 for single select.
 * @param on_segment_changed Callback fired when a segment is selected/deselected.
 * @param user_data User data passed to callback.
 * @param out_node Pointer to receive the created UI node.
 * @return int 0 on success, non-zero on error.
 */
int m3_segmented_button_create(material_catalog_state_t* state, const char** segments, int segment_count, int is_multi_select, m3_event_cb_t on_segment_changed, void* user_data, cmp_ui_node_t** out_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_BUTTONS_H */
/* clang-format on */