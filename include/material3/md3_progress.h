/**
 * @file md3_progress.h
 * @brief Material 3 Linear, Circular, and Expressive Progress Indicators
 * wrapping ui_progress_base.
 */

#ifndef MATERIAL3_MD3_PROGRESS_H
#define MATERIAL3_MD3_PROGRESS_H

/* clang-format off */
#include "ui_progress_base.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_progress_type
 * @brief Progress indicator style types.
 */
enum md3_progress_type {
  MD3_PROGRESS_LINEAR = 0,
  MD3_PROGRESS_CIRCULAR,
  MD3_PROGRESS_EXPRESSIVE_SEGMENTED,
  MD3_PROGRESS_TYPE_COUNT
};

/**
 * @struct md3_progress
 * @brief Material 3 Progress Indicator skin wrapping ui_progress_base.
 */
struct md3_progress {
  struct ui_progress_base *base;
  enum md3_progress_type type;
  float value;
  float min;
  float max;
  int is_indeterminate;
};

/**
 * @brief Creates a Material 3 Progress Indicator component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param type Indicator type (Linear, Circular, or Expressive Segmented).
 * @param out_progress Pointer to receive newly created progress component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_progress_create(struct ui_engine *engine, enum md3_progress_type type,
                    struct md3_progress **out_progress);

/**
 * @brief Destroys a Material 3 Progress Indicator.
 *
 * @param progress The progress indicator to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_progress_destroy(struct md3_progress *progress);

/**
 * @brief Sets determinate progress value.
 *
 * @param progress The progress indicator.
 * @param value Current progress value.
 * @param min Minimum range value.
 * @param max Maximum range value.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_progress_set_determinate(
    struct md3_progress *progress, float value, float min, float max);

/**
 * @brief Sets indeterminate oscillating animation mode.
 *
 * @param progress The progress indicator.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_progress_set_indeterminate(struct md3_progress *progress);

/**
 * @brief Queries current normalized progress percentage [0.0, 1.0].
 *
 * @param progress The progress indicator.
 * @param out_percentage Pointer to receive percentage.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_progress_get_percentage(
    const struct md3_progress *progress, float *out_percentage);

/**
 * @brief Queries whether progress indicator is indeterminate.
 *
 * @param progress The progress indicator.
 * @param out_is_indeterminate Pointer to receive 1 if indeterminate, 0 if
 * determinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_progress_is_indeterminate(
    const struct md3_progress *progress, int *out_is_indeterminate);

/**
 * @brief Retrieves underlying ui_progress_base handle.
 *
 * @param progress The progress indicator.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_progress_get_base(
    struct md3_progress *progress, struct ui_progress_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_PROGRESS_H */
