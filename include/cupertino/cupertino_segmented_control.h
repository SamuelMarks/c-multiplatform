/**
 * @file cupertino_segmented_control.h
 * @brief Cupertino Segmented Control wrapping ui_segmented_control_base with
 * Apple HIG styling.
 */

#ifndef CUPERTINO_CUPERTINO_SEGMENTED_CONTROL_H
#define CUPERTINO_CUPERTINO_SEGMENTED_CONTROL_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_segmented_control_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Maximum segments supported in a single Cupertino segmented control.
 */
#define CUPERTINO_SEGMENTED_CONTROL_MAX_SEGMENTS 16

/**
 * @struct cupertino_segmented_control
 * @brief Cupertino Segmented Control widget holding state and wrapping
 * ui_segmented_control_base.
 */
struct cupertino_segmented_control {
  struct ui_segmented_control_base *base; /**< Wrapped CDK segmented control. */
  struct ui_control_value_accessor cva;   /**< Control Value Accessor. */
  int selected_index;     /**< Currently selected segment index. */
  int segment_count;      /**< Number of appended segments. */
  float thumb_offset_x;   /**< Animated sliding thumb position. */
  float thumb_width;      /**< Active segment width. */
  ui_color_t bg_color;    /**< Container fill (SystemGray5). */
  ui_color_t thumb_color; /**< Sliding thumb fill (White). */
};

/**
 * @brief Creates a new Cupertino Segmented Control component.
 *
 * @param engine Pointer to ui_engine.
 * @param out_control Pointer to receive newly created control.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_segmented_control_create(
    struct ui_engine *engine, struct cupertino_segmented_control **out_control,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Cupertino segmented control and its underlying base.
 *
 * @param control The control to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_segmented_control_destroy(
    struct cupertino_segmented_control *control);

/**
 * @brief Appends a new text segment to the control.
 *
 * @param control The control.
 * @param label Segment label text.
 * @param out_index Pointer to receive newly created segment index.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_segmented_control_add_segment(
    struct cupertino_segmented_control *control, const char *label,
    int *out_index);

/**
 * @brief Selects segment by index, updating slider thumb position.
 *
 * @param control The control.
 * @param index Index to select (0 to segment_count - 1).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_segmented_control_select_index(
    struct cupertino_segmented_control *control, int index);

/**
 * @brief Queries currently selected segment index.
 *
 * @param control The control.
 * @param out_index Pointer to receive selected index (-1 if none).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_segmented_control_get_selected_index(
    const struct cupertino_segmented_control *control, int *out_index);

/**
 * @brief Queries number of segments.
 *
 * @param control The control.
 * @param out_count Pointer to receive count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_segmented_control_get_segment_count(
    const struct cupertino_segmented_control *control, int *out_count);

/**
 * @brief Retrieves underlying ui_segmented_control_base handle.
 *
 * @param control The control.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_segmented_control_get_base(
    struct cupertino_segmented_control *control,
    struct ui_segmented_control_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SEGMENTED_CONTROL_H */
