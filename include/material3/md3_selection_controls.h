/**
 * @file md3_selection_controls.h
 * @brief Material 3 & Expressive Selection and Advanced Input Controls.
 *
 * Provides spec-compliant Material 3 implementations for:
 * - md3_autocomplete (wrapping ui_autocomplete_base, ui_input_base,
 * ui_listbox_base)
 * - md3_select (wrapping ui_select_base, ui_menu_base)
 * - md3_pin_input (wrapping ui_pin_input_base, ui_input_base)
 * - md3_rating (wrapping ui_rating_base)
 * - md3_spin_button (wrapping ui_spin_button_base, ui_input_base)
 */

#ifndef MATERIAL3_MD3_SELECTION_CONTROLS_H
#define MATERIAL3_MD3_SELECTION_CONTROLS_H

/* clang-format off */
#include "material3/md3_text_field.h"
#include "ui_autocomplete_base.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_input_base.h"
#include "ui_listbox_base.h"
#include "ui_pin_input_base.h"
#include "ui_rating_base.h"
#include "ui_select_base.h"
#include "ui_spin_button_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @struct md3_autocomplete
 * @brief Material 3 Autocomplete component.
 */
struct md3_autocomplete {
  struct ui_autocomplete_base *base;
  enum md3_text_field_variant variant;
  int is_open;
  char query[256];
  float dropdown_elevation;
  float corner_radius;
  int expressive_spring_enabled;
};

/**
 * @brief Creates a Material 3 Autocomplete component.
 *
 * @param engine The UI engine context.
 * @param variant Text field variant (Filled or Outlined).
 * @param out_autocomplete Pointer to receive the allocated autocomplete handle.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_autocomplete_create(
    struct ui_engine *engine, enum md3_text_field_variant variant,
    struct md3_autocomplete **out_autocomplete,
    struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a Material 3 Autocomplete component.
 *
 * @param autocomplete The autocomplete component to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_autocomplete_destroy(struct md3_autocomplete *autocomplete);

/**
 * @brief Sets the filter query text on the autocomplete component.
 *
 * @param autocomplete The autocomplete component.
 * @param query The search query string.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_autocomplete_set_query(
    struct md3_autocomplete *autocomplete, const char *query);

/**
 * @brief Gets the current query text of the autocomplete component.
 *
 * @param autocomplete The autocomplete component.
 * @param out_query Pointer to buffer receiving query string.
 * @param query_capacity Size of out_query buffer.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_autocomplete_get_query(const struct md3_autocomplete *autocomplete,
                           char *out_query, size_t query_capacity);

/**
 * @struct md3_select_option
 * @brief Option item in an md3_select dropdown.
 */
struct md3_select_option {
  char label[64];
  char value[64];
  int is_selected;
  int is_disabled;
};

/**
 * @struct md3_select
 * @brief Material 3 Select / Dropdown menu component.
 */
struct md3_select {
  struct ui_select_base *base;
  enum md3_text_field_variant variant;
  int is_multi;
  int is_open;
  float arrow_rotation_deg;
  struct md3_select_option options[64];
  size_t option_count;
  int selected_index;
  char placeholder[64];
};

/**
 * @brief Creates a Material 3 Select component.
 *
 * @param engine The UI engine context.
 * @param variant Text field variant (Filled or Outlined).
 * @param out_select Pointer to receive the allocated select handle.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_select_create(
    struct ui_engine *engine, enum md3_text_field_variant variant,
    struct md3_select **out_select, struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a Material 3 Select component.
 *
 * @param select The select component to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_select_destroy(struct md3_select *select);

/**
 * @brief Adds an option item to the Material 3 Select dropdown.
 *
 * @param select The select component.
 * @param label Option display text.
 * @param value Option underlying value.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_select_add_option(
    struct md3_select *select, const char *label, const char *value);

/**
 * @brief Sets multi-selection mode for the select component.
 *
 * @param select The select component.
 * @param is_multi 1 for multi-select, 0 for single-select.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_select_set_multiselect(struct md3_select *select, int is_multi);

/**
 * @brief Sets the open/expanded state of the select dropdown.
 *
 * @param select The select component.
 * @param is_open 1 to open, 0 to close.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_select_set_open(struct md3_select *select, int is_open);

/**
 * @brief Sets the selected option index for single-select mode.
 *
 * @param select The select component.
 * @param index Index to select, or -1 for none.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_select_set_selected_index(struct md3_select *select, int index);

/**
 * @brief Gets the selected option index for single-select mode.
 *
 * @param select The select component.
 * @param out_index Pointer to receive selected index.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_select_get_selected_index(const struct md3_select *select, int *out_index);

/**
 * @struct md3_pin_input
 * @brief Material 3 PIN / One-Time Passcode (OTP) input component.
 */
struct md3_pin_input {
  struct ui_pin_input_base *base;
  int length;
  int is_masked;
  int focused_index;
  char values[16];
  float cell_width;
  float cell_height;
  int is_valid;
};

/**
 * @brief Creates a Material 3 PIN Input component.
 *
 * @param engine The UI engine context.
 * @param length Number of PIN digits (e.g. 4, 6, 8).
 * @param out_pin_input Pointer to receive the allocated PIN input handle.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_pin_input_create(
    struct ui_engine *engine, int length, struct md3_pin_input **out_pin_input,
    struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a Material 3 PIN input component.
 *
 * @param pin_input The PIN input component to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_pin_input_destroy(struct md3_pin_input *pin_input);

/**
 * @brief Sets masked PIN mode (bullet obscuring vs plain text OTP).
 *
 * @param pin_input The PIN input component.
 * @param is_masked 1 to mask digits, 0 for visible digits.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_pin_input_set_masked(struct md3_pin_input *pin_input, int is_masked);

/**
 * @brief Inserts a character at the specified digit cell and advances focus.
 *
 * @param pin_input The PIN input component.
 * @param index Cell index (0 to length - 1).
 * @param c Character string to insert.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_pin_input_on_input(
    struct md3_pin_input *pin_input, int index, const char *c);

/**
 * @brief Handles backspace deletion at the specified digit cell and retreats
 * focus.
 *
 * @param pin_input The PIN input component.
 * @param index Cell index.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_pin_input_on_backspace(struct md3_pin_input *pin_input, int index);

/**
 * @brief Handles paste string distribution across PIN digit cells.
 *
 * @param pin_input The PIN input component.
 * @param pasted_text String to distribute across cells.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_pin_input_on_paste(
    struct md3_pin_input *pin_input, const char *pasted_text);

/**
 * @brief Gets the assembled PIN text across all cells.
 *
 * @param pin_input The PIN input component.
 * @param out_text Buffer to receive the PIN string.
 * @param text_capacity Size of out_text buffer.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_pin_input_get_value(const struct md3_pin_input *pin_input, char *out_text,
                        size_t text_capacity);

/**
 * @struct md3_rating
 * @brief Material 3 Rating bar component with whole and fractional stars.
 */
struct md3_rating {
  struct ui_rating_base *base;
  int max_rating;
  float value;
  float hover_preview_value;
  int is_read_only;
  int allow_half_stars;
};

/**
 * @brief Creates a Material 3 Rating component.
 *
 * @param engine The UI engine context.
 * @param max_rating Maximum number of stars (e.g. 5).
 * @param out_rating Pointer to receive the allocated rating handle.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_rating_create(
    struct ui_engine *engine, int max_rating, struct md3_rating **out_rating,
    struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a Material 3 Rating component.
 *
 * @param rating The rating component to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_rating_destroy(struct md3_rating *rating);

/**
 * @brief Sets the current rating value.
 *
 * @param rating The rating component.
 * @param value The rating value (0.0 to max_rating).
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_rating_set_value(struct md3_rating *rating, float value);

/**
 * @brief Gets the current rating value.
 *
 * @param rating The rating component.
 * @param out_value Pointer to receive current value.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_rating_get_value(const struct md3_rating *rating, float *out_value);

/**
 * @brief Sets the hover preview rating value before confirmation.
 *
 * @param rating The rating component.
 * @param preview_value Preview rating value.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_rating_set_hover_preview(struct md3_rating *rating, float preview_value);

/**
 * @brief Sets read-only mode for display ratings.
 *
 * @param rating The rating component.
 * @param is_read_only 1 for read-only, 0 for interactive.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_rating_set_read_only(struct md3_rating *rating, int is_read_only);

/**
 * @struct md3_spin_button
 * @brief Material 3 Spin Button / Numeric Stepper component.
 */
struct md3_spin_button {
  struct ui_spin_button_base *base;
  double min_val;
  double max_val;
  double step_val;
  double current_val;
  int precision;
  char prefix[16];
  char suffix[16];
  int is_horizontal;
};

/**
 * @brief Creates a Material 3 Spin Button component.
 *
 * @param engine The UI engine context.
 * @param min Minimum allowable value.
 * @param max Maximum allowable value.
 * @param step Step increment.
 * @param out_spin_button Pointer to receive the allocated spin button handle.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_spin_button_create(struct ui_engine *engine, double min, double max,
                       double step, struct md3_spin_button **out_spin_button,
                       struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a Material 3 Spin Button component.
 *
 * @param spin_button The spin button component to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_spin_button_destroy(struct md3_spin_button *spin_button);

/**
 * @brief Sets the value of the spin button, clamped to [min, max].
 *
 * @param spin_button The spin button.
 * @param value The new numeric value.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_spin_button_set_value(struct md3_spin_button *spin_button, double value);

/**
 * @brief Gets the value of the spin button.
 *
 * @param spin_button The spin button.
 * @param out_value Pointer to receive current value.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_spin_button_get_value(
    const struct md3_spin_button *spin_button, double *out_value);

/**
 * @brief Steps the spin button value by N increments.
 *
 * @param spin_button The spin button.
 * @param step_count Number of steps (positive or negative).
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_spin_button_step(struct md3_spin_button *spin_button, int step_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_SELECTION_CONTROLS_H */
