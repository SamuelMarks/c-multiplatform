/* clang-format off */
#ifndef M3_CONTAINMENT_H
#define M3_CONTAINMENT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cmp.h"
#include "material_catalog.h"

#include "m3_buttons.h"

/**
 * @brief Types of Material 3 Cards.
 */
typedef enum m3_card_type {
    M3_CARD_TYPE_ELEVATED,
    M3_CARD_TYPE_FILLED,
    M3_CARD_TYPE_OUTLINED
} m3_card_type_t;

/**
 * @brief Configuration for a Material 3 Card.
 */
typedef struct m3_card_config {
    m3_card_type_t type;         /**< The card visual style */
    m3_event_cb_t on_click; /**< Optional callback for interactive cards */
    void* user_data;             /**< User data passed to callback */
    int is_draggable;            /**< 1 if card can be dragged (increases elevation), 0 otherwise */
} m3_card_config_t;

/**
 * @brief Creates a new Material 3 Card UI node (container).
 * 
 * @param state The material catalog state.
 * @param config The card configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return int 0 on success, non-zero on error.
 */
int m3_card_create(material_catalog_state_t* state, const m3_card_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Types of Material 3 Dividers.
 */
typedef enum m3_divider_type {
    M3_DIVIDER_TYPE_FULL_WIDTH,
    M3_DIVIDER_TYPE_INSET,
    M3_DIVIDER_TYPE_MIDDLE_INSET
} m3_divider_type_t;

/**
 * @brief Material 3 Divider configuration.
 */
typedef struct m3_divider_config {
    m3_divider_type_t type;      /**< The divider inset style */
    int is_vertical;             /**< 1 for vertical divider, 0 for horizontal */
    float inset_start_dp;        /**< Custom inset from start (0 for full-width) */
    float inset_end_dp;          /**< Custom inset from end (0 for full-width) */
} m3_divider_config_t;

/**
 * @brief Creates a Material 3 Divider UI node.
 * 
 * @param state The material catalog state.
 * @param config The divider configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return int 0 on success, non-zero on error.
 */
int m3_divider_create(material_catalog_state_t* state, const m3_divider_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Types of Material 3 Dialogs.
 */
typedef enum m3_dialog_type {
    M3_DIALOG_TYPE_BASIC,
    M3_DIALOG_TYPE_FULL_SCREEN
} m3_dialog_type_t;

/**
 * @brief Material 3 Dialog configuration.
 */
typedef struct m3_dialog_config {
    m3_dialog_type_t type;       /**< Basic or Full-screen */
    const char* icon;            /**< Optional icon */
    const char* headline;        /**< Dialog title/headline */
    const char* supporting_text; /**< Optional supporting text */
    const char* confirm_label;   /**< Confirm button label */
    const char* dismiss_label;   /**< Dismiss button label */
    m3_event_cb_t on_confirm; /**< Confirm callback */
    m3_event_cb_t on_dismiss; /**< Dismiss callback */
    void* user_data;             /**< User data for callbacks */
} m3_dialog_config_t;

/**
 * @brief Creates a Material 3 Dialog UI node.
 * 
 * @param state The material catalog state.
 * @param config The dialog configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return int 0 on success, non-zero on error.
 */
int m3_dialog_create(material_catalog_state_t* state, const m3_dialog_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Types of Material 3 Bottom Sheets.
 */
typedef enum m3_bottom_sheet_type {
    M3_BOTTOM_SHEET_TYPE_STANDARD,
    M3_BOTTOM_SHEET_TYPE_MODAL
} m3_bottom_sheet_type_t;

/**
 * @brief Material 3 Bottom Sheet configuration.
 */
typedef struct m3_bottom_sheet_config {
    m3_bottom_sheet_type_t type; /**< Standard (inline) or Modal (scrim overlay) */
    int show_drag_handle;        /**< 1 to show the drag handle indicator, 0 otherwise */
    m3_event_cb_t on_dismiss; /**< Dismiss callback (for modal) */
    void* user_data;             /**< User data for callback */
} m3_bottom_sheet_config_t;

/**
 * @brief Creates a Material 3 Bottom Sheet UI node container.
 * 
 * @param state The material catalog state.
 * @param config The bottom sheet configuration.
 * @param out_node Pointer to receive the created UI node.
 * @return int 0 on success, non-zero on error.
 */
int m3_bottom_sheet_create(material_catalog_state_t* state, const m3_bottom_sheet_config_t* config, cmp_ui_node_t** out_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_CONTAINMENT_H */
/* clang-format on */