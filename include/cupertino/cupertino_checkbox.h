/**
 * @file cupertino_checkbox.h
 * @brief Cupertino Checkbox component wrapping ui_checkbox_base with Apple HIG
 * styling.
 */

#ifndef CUPERTINO_CUPERTINO_CHECKBOX_H
#define CUPERTINO_CUPERTINO_CHECKBOX_H

/* clang-format off */
#include "ui_checkbox_base.h"
#include "ui_color_space.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Standard Apple HIG checkbox dimensions.
 */
#define CUPERTINO_CHECKBOX_SIZE 18.0f
/**
 * @brief Standard Apple HIG checkbox corner radius.
 */
#define CUPERTINO_CHECKBOX_CORNER_RADIUS 4.0f

/**
 * @struct cupertino_checkbox
 * @brief Cupertino Checkbox skin wrapping ui_checkbox_base.
 */
struct cupertino_checkbox {
  struct ui_checkbox_base *base;        /**< Wrapped CDK checkbox primitive. */
  struct ui_control_value_accessor cva; /**< Control Value Accessor. */
  enum ui_checkbox_state state;         /**< Current tri-state status. */
  float stroke_progress; /**< Animated checkmark reveal progress [0.0, 1.0]. */
  ui_color_t active_color; /**< Checked fill color (SystemBlue). */
  ui_color_t border_color; /**< Unchecked border color (SystemGray4). */
  char label[64];          /**< Associated label string. */
};

/**
 * @brief Creates a new Cupertino Checkbox component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_checkbox Pointer to receive newly created checkbox.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_checkbox_create(
    struct ui_engine *engine, struct cupertino_checkbox **out_checkbox,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Cupertino checkbox and its underlying base.
 *
 * @param cb The checkbox to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_checkbox_destroy(struct cupertino_checkbox *cb);

/**
 * @brief Sets tri-state status of the checkbox.
 *
 * @param cb The checkbox.
 * @param state New tri-state status (UNCHECKED, CHECKED, or INDETERMINATE).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_checkbox_set_state(
    struct cupertino_checkbox *cb, enum ui_checkbox_state state);

/**
 * @brief Gets current tri-state status of the checkbox.
 *
 * @param cb The checkbox.
 * @param out_state Pointer to receive current status.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_checkbox_get_state(
    struct cupertino_checkbox *cb, enum ui_checkbox_state *out_state);

/**
 * @brief Toggles checkbox state between checked and unchecked.
 *
 * @param cb The checkbox.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_checkbox_toggle(struct cupertino_checkbox *cb);

/**
 * @brief Sets optional label text for the checkbox.
 *
 * @param cb The checkbox.
 * @param label Text string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_checkbox_set_label(struct cupertino_checkbox *cb, const char *label);

/**
 * @brief Retrieves underlying ui_checkbox_base handle.
 *
 * @param cb The checkbox.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_checkbox_get_base(
    struct cupertino_checkbox *cb, struct ui_checkbox_base **out_base);

/**
 * @brief Sets animated checkmark reveal path stroke progress.
 *
 * @param cb The checkbox.
 * @param progress Fraction in [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_checkbox_set_stroke_progress(
    struct cupertino_checkbox *cb, float progress);

/**
 * @brief Gets current animated stroke progress.
 *
 * @param cb The checkbox.
 * @param out_progress Pointer to receive stroke fraction.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_checkbox_get_stroke_progress(
    const struct cupertino_checkbox *cb, float *out_progress);

/**
 * @brief Computes animated checkmark path vertices via linear stroke
 * interpolation.
 *
 * @param cb The checkbox.
 * @param out_x1 Pointer to receive start X.
 * @param out_y1 Pointer to receive start Y.
 * @param out_x2 Pointer to receive knee vertex X.
 * @param out_y2 Pointer to receive knee vertex Y.
 * @param out_x3 Pointer to receive current animated tip X.
 * @param out_y3 Pointer to receive current animated tip Y.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_checkbox_compute_checkmark_path(const struct cupertino_checkbox *cb,
                                          float *out_x1, float *out_y1,
                                          float *out_x2, float *out_y2,
                                          float *out_x3, float *out_y3);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_CHECKBOX_H */
