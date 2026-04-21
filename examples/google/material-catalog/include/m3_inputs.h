/* clang-format off */
#ifndef M3_INPUTS_H
#define M3_INPUTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cmp.h"
#include "material_catalog.h"
#include "m3_buttons.h"

/**
 * @brief Material 3 Checkbox states.
 */
typedef enum m3_checkbox_state {
    M3_CHECKBOX_STATE_UNCHECKED,
    M3_CHECKBOX_STATE_CHECKED,
    M3_CHECKBOX_STATE_INDETERMINATE
} m3_checkbox_state_t;

/**
 * @brief Configuration for a Material 3 Checkbox.
 */
typedef struct m3_checkbox_config {
    m3_checkbox_state_t state;   /**< Current state */
    int is_disabled;             /**< 1 if disabled, 0 otherwise */
    int is_error;                /**< 1 if error state, 0 otherwise */
    m3_event_cb_t on_toggle;     /**< Callback on toggle */
    void* user_data;             /**< User data for callback */
} m3_checkbox_config_t;

/**
 * @brief Creates a Material 3 Checkbox UI node.
 */
int m3_checkbox_create(material_catalog_state_t* state, const m3_checkbox_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Configuration for a Material 3 Radio Button.
 */
typedef struct m3_radio_button_config {
    int is_selected;             /**< 1 if selected, 0 otherwise */
    int is_disabled;             /**< 1 if disabled, 0 otherwise */
    m3_event_cb_t on_select;     /**< Callback on select */
    void* user_data;             /**< User data for callback */
} m3_radio_button_config_t;

/**
 * @brief Creates a Material 3 Radio Button UI node.
 */
int m3_radio_button_create(material_catalog_state_t* state, const m3_radio_button_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Configuration for a Material 3 Switch.
 */
typedef struct m3_switch_config {
    int is_on;                   /**< 1 if on, 0 if off */
    int is_disabled;             /**< 1 if disabled, 0 otherwise */
    int show_icon;               /**< 1 to show inner thumb icon (check/X), 0 otherwise */
    m3_event_cb_t on_toggle;     /**< Callback on toggle */
    void* user_data;             /**< User data for callback */
} m3_switch_config_t;

/**
 * @brief Creates a Material 3 Switch UI node.
 */
int m3_switch_create(material_catalog_state_t* state, const m3_switch_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Types of Material 3 Chips.
 */
typedef enum m3_chip_type {
    M3_CHIP_TYPE_ASSIST,
    M3_CHIP_TYPE_FILTER,
    M3_CHIP_TYPE_INPUT,
    M3_CHIP_TYPE_SUGGESTION
} m3_chip_type_t;

/**
 * @brief Configuration for a Material 3 Chip.
 */
typedef struct m3_chip_config {
    m3_chip_type_t type;         /**< Type of the chip */
    const char* label;           /**< Chip label text */
    const char* leading_icon;    /**< Optional leading icon */
    const char* trailing_icon;   /**< Optional trailing icon (close icon for input chips) */
    int is_elevated;             /**< 1 for elevated variant, 0 for flat/outlined */
    int is_selected;             /**< 1 if selected (mainly for filter chips), 0 otherwise */
    int is_disabled;             /**< 1 if disabled, 0 otherwise */
    m3_event_cb_t on_click;      /**< Main click callback */
    m3_event_cb_t on_trailing_click; /**< Callback for trailing icon click (e.g. dismiss) */
    void* user_data;             /**< User data for callbacks */
} m3_chip_config_t;

/**
 * @brief Creates a Material 3 Chip UI node.
 */
int m3_chip_create(material_catalog_state_t* state, const m3_chip_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Types of Material 3 Sliders.
 */
typedef enum m3_slider_type {
    M3_SLIDER_TYPE_CONTINUOUS,
    M3_SLIDER_TYPE_DISCRETE
} m3_slider_type_t;

/**
 * @brief Configuration for a Material 3 Slider.
 */
typedef struct m3_slider_config {
    m3_slider_type_t type;       /**< Continuous or Discrete */
    float value;                 /**< Current value */
    float min_value;             /**< Minimum value */
    float max_value;             /**< Maximum value */
    int steps;                   /**< Number of discrete steps (> 0 for discrete) */
    int is_disabled;             /**< 1 if disabled, 0 otherwise */
} m3_slider_config_t;

/**
 * @brief Creates a Material 3 Slider UI node.
 */
int m3_slider_create(material_catalog_state_t* state, const m3_slider_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Configuration for a Material 3 Range Slider.
 */
typedef struct m3_range_slider_config {
    m3_slider_type_t type;       /**< Continuous or Discrete */
    float start_value;           /**< Current start value */
    float end_value;             /**< Current end value */
    float min_value;             /**< Minimum value */
    float max_value;             /**< Maximum value */
    int steps;                   /**< Number of discrete steps (> 0 for discrete) */
    int is_disabled;             /**< 1 if disabled, 0 otherwise */
} m3_range_slider_config_t;

/**
 * @brief Creates a Material 3 Range Slider UI node.
 */
int m3_range_slider_create(material_catalog_state_t* state, const m3_range_slider_config_t* config, cmp_ui_node_t** out_node);

/**
 * @brief Types of Material 3 Text Fields.
 */
typedef enum m3_text_field_type {
    M3_TEXT_FIELD_TYPE_FILLED,
    M3_TEXT_FIELD_TYPE_OUTLINED
} m3_text_field_type_t;

/**
 * @brief Configuration for a Material 3 Text Field.
 */
typedef struct m3_text_field_config {
    m3_text_field_type_t type;   /**< Filled or Outlined */
    const char* text;            /**< Current text content */
    const char* label;           /**< Label/Placeholder */
    const char* supporting_text; /**< Bottom supporting text */
    const char* leading_icon;    /**< Optional leading icon */
    const char* trailing_icon;   /**< Optional trailing icon */
    int is_focused;              /**< 1 if currently focused, 0 otherwise */
    int is_error;                /**< 1 if in error state, 0 otherwise */
    int is_disabled;             /**< 1 if disabled, 0 otherwise */
} m3_text_field_config_t;

/**
 * @brief Creates a Material 3 Text Field UI node.
 */
int m3_text_field_create(material_catalog_state_t* state, const m3_text_field_config_t* config, cmp_ui_node_t** out_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_INPUTS_H */
/* clang-format on */