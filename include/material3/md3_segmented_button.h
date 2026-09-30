/**
 * @file md3_segmented_button.h
 * @brief Material 3 Segmented Button component wrapping
 * ui_segmented_control_base.
 */

#ifndef MATERIAL3_MD3_SEGMENTED_BUTTON_H
#define MATERIAL3_MD3_SEGMENTED_BUTTON_H

/* clang-format off */
#include "ui_segmented_control_base.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define MD3_SEGMENTED_BUTTON_MAX_SEGMENTS 16

/**
 * @struct md3_segment_item
 * @brief Represents an individual segment button in the segmented button group.
 */
struct md3_segment_item {
  struct ui_segmented_button_base *base;
  int id;
  char label[64];
  char icon[32];
};

/**
 * @struct md3_segmented_button
 * @brief Material 3 Segmented Button skin wrapping ui_segmented_control_base.
 */
struct md3_segmented_button {
  struct ui_segmented_control_base *base;
  enum ui_segmented_control_mode mode;
  struct ui_control_value_accessor cva;
  struct md3_segment_item segments[MD3_SEGMENTED_BUTTON_MAX_SEGMENTS];
  size_t segment_count;
  int selected_segment_id;
  int disabled;
};

/**
 * @brief Creates a Material 3 Segmented Button component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param mode Selection mode (single-select vs multi-select).
 * @param out_button Pointer to receive newly created segmented button.
 * @param out_cva Optional pointer to receive Control Value Accessor interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_segmented_button_create(
    struct ui_engine *engine, enum ui_segmented_control_mode mode,
    struct md3_segmented_button **out_button,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Material 3 segmented button.
 *
 * @param button Segmented button to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_segmented_button_destroy(struct md3_segmented_button *button);

/**
 * @brief Adds a segment button to the segmented button group.
 *
 * @param button The segmented button.
 * @param label Text label for the segment (can be NULL).
 * @param icon Icon identifier (can be NULL).
 * @param segment_id Numeric identifier for this segment.
 * @param out_segment Optional pointer to receive underlying segmented button
 * base handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_segmented_button_add_segment(
    struct md3_segmented_button *button, const char *label, const char *icon,
    int segment_id, struct ui_segmented_button_base **out_segment);

/**
 * @brief Selects or activates a segment by ID.
 *
 * @param button The segmented button.
 * @param segment_id ID of the segment to select.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_segmented_button_select_segment(
    struct md3_segmented_button *button, int segment_id);

/**
 * @brief Queries whether a segment is currently selected.
 *
 * @param button The segmented button.
 * @param segment_id ID of the segment to query.
 * @param out_selected Pointer to receive 1 if selected, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_segmented_button_is_selected(const struct md3_segmented_button *button,
                                 int segment_id, int *out_selected);

/**
 * @brief Retrieves the underlying ui_segmented_control_base handle.
 *
 * @param button The segmented button.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_segmented_button_get_base(struct md3_segmented_button *button,
                              struct ui_segmented_control_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_SEGMENTED_BUTTON_H */
