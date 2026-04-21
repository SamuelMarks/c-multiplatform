/* clang-format off */
#ifndef M3_COMMUNICATION_H
#define M3_COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cmp.h"
#include "material_catalog.h"

/**
 * @brief Material 3 Badge types.
 */
typedef enum m3_badge_type {
    M3_BADGE_TYPE_SMALL_DOT,
    M3_BADGE_TYPE_LARGE
} m3_badge_type_t;

/**
 * @brief Configuration for a Material 3 Badge.
 */
typedef struct m3_badge_config {
    m3_badge_type_t type;  /**< Type of the badge (dot or large) */
    int number;            /**< Number to display (only for LARGE badge, e.g. 99 for "99+") */
} m3_badge_config_t;

/**
 * @brief Creates a Material 3 Badge UI node.
 * 
 * @param state The catalog state.
 * @param config The badge configuration.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, non-zero on error.
 */
int m3_badge_create(material_catalog_state_t* state, const m3_badge_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Material 3 Progress Indicator types.
 */
typedef enum m3_progress_type {
    M3_PROGRESS_TYPE_LINEAR,
    M3_PROGRESS_TYPE_CIRCULAR
} m3_progress_type_t;

/**
 * @brief Material 3 Progress Indicator configuration.
 */
typedef struct m3_progress_config {
    m3_progress_type_t type;  /**< Linear or Circular */
    int is_determinate;       /**< 1 if determinate, 0 if indeterminate */
    float progress;           /**< 0.0f to 1.0f progress value for determinate */
    int is_paused;            /**< 1 to pause indeterminate animation (Stop/Pause states) */
    int four_color_morph;     /**< 1 for 4-color morphing variant (only applicable for circular indeterminate) */
} m3_progress_config_t;

/**
 * @brief Creates a Material 3 Progress Indicator UI node.
 * 
 * @param state The catalog state.
 * @param config The progress indicator configuration.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, non-zero on error.
 */
int m3_progress_create(material_catalog_state_t* state, const m3_progress_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Snackbar duration configurations.
 */
typedef enum m3_snackbar_duration {
    M3_SNACKBAR_DURATION_SHORT,  /**< ~4000ms */
    M3_SNACKBAR_DURATION_LONG,   /**< ~10000ms */
    M3_SNACKBAR_DURATION_INDEFINITE
} m3_snackbar_duration_t;

/**
 * @brief Material 3 Snackbar configuration.
 */
typedef struct m3_snackbar_config {
    const char* text;                    /**< Main text (Single-line or Two-line depending on length) */
    const char* action_label;            /**< Optional action label */
    int show_close_icon;                 /**< 1 to show a close icon, 0 otherwise */
    m3_snackbar_duration_t duration;     /**< Auto-dismiss duration */
    void (*on_action)(void* user_data);  /**< Callback when action is clicked */
    void (*on_dismiss)(void* user_data); /**< Callback when snackbar is dismissed */
    void* user_data;                     /**< User data for callbacks */
} m3_snackbar_config_t;

/**
 * @brief Creates a Material 3 Snackbar UI node.
 * 
 * @param state The catalog state.
 * @param config The snackbar configuration.
 * @param out_node Pointer to receive the allocated node.
 * @return 0 on success, non-zero on error.
 */
int m3_snackbar_create(material_catalog_state_t* state, const m3_snackbar_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Queues a Snackbar to be shown globally.
 * 
 * @param state The catalog state.
 * @param config The snackbar configuration.
 * @return 0 on success, non-zero on error.
 */
int m3_snackbar_enqueue(material_catalog_state_t* state, const m3_snackbar_config_t* config);

/**
 * @brief Processes the snackbar queue (to be called in the main event loop).
 * 
 * @param state The catalog state.
 * @param dt Delta time in milliseconds.
 * @return 0 on success, non-zero on error.
 */
int m3_snackbar_process_queue(material_catalog_state_t* state, float dt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_COMMUNICATION_H */
/* clang-format on */