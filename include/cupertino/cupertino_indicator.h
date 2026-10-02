/**
 * @file cupertino_indicator.h
 * @brief Cupertino Activity Indicator & Progress Bar conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_INDICATOR_H
#define CUPERTINO_CUPERTINO_INDICATOR_H

/* clang-format off */
#include "ui_progress_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_INDICATOR_STEP_INTERVAL_MS 83.33f
#define CUPERTINO_INDICATOR_SMALL_DIM 20.0f
#define CUPERTINO_INDICATOR_LARGE_DIM 37.0f
#define CUPERTINO_INDICATOR_BAR_HEIGHT 4.0f

/**
 * @enum cupertino_indicator_type
 * @brief Variant of progress indicator.
 */
enum cupertino_indicator_type {
  CUPERTINO_INDICATOR_TYPE_ACTIVITY =
      0,                       /**< Stepped radial spoke flower spinner. */
  CUPERTINO_INDICATOR_TYPE_BAR /**< Linear horizontal progress bar. */
};

/**
 * @enum cupertino_indicator_size
 * @brief Physical dimension scale for activity spinner.
 */
enum cupertino_indicator_size {
  CUPERTINO_INDICATOR_SIZE_SMALL = 0, /**< 20x20pt (8 spokes). */
  CUPERTINO_INDICATOR_SIZE_LARGE      /**< 37x37pt (12 spokes). */
};

/**
 * @struct cupertino_indicator_descriptor
 * @brief Configuration descriptor for creating a Cupertino indicator.
 */
struct cupertino_indicator_descriptor {
  enum cupertino_indicator_type type; /**< Spinner vs linear bar. */
  enum cupertino_indicator_size size; /**< Small (20pt) vs Large (37pt). */
  int is_animating;                   /**< 1 if active on creation. */
  int is_indeterminate;               /**< 1 for indeterminate bar. */
  float initial_value;                /**< Initial progress in [0.0, 1.0]. */
};

/**
 * @struct cupertino_indicator
 * @brief Cupertino Indicator instance wrapping ui_progress_base.
 */
struct cupertino_indicator {
  struct ui_progress_base *base;      /**< CDK progress primitive. */
  enum cupertino_indicator_type type; /**< Activity vs bar. */
  enum cupertino_indicator_size size; /**< Small vs large size. */
  int is_animating;                   /**< 1 if active, 0 if paused. */
  int spoke_count;                    /**< 8 for small, 12 for large. */
  int current_spoke_step;             /**< Leading spoke index. */
  float step_timer_ms;       /**< Timer accumulator for stepped rotation. */
  float value;               /**< Determinate value in [0.0, 1.0]. */
  float spoke_opacities[12]; /**< Alpha values for all spokes. */
};

/**
 * @brief Creates a new Cupertino indicator instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_indicator Pointer to receive newly created indicator.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_indicator_create(
    struct ui_engine *engine, const struct cupertino_indicator_descriptor *desc,
    struct cupertino_indicator **out_indicator);

/**
 * @brief Destroys a Cupertino indicator instance.
 *
 * @param indicator Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_indicator_destroy(struct cupertino_indicator *indicator);

/**
 * @brief Starts spinner animation or bar pulsing.
 *
 * @param indicator Target indicator.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_indicator_start(struct cupertino_indicator *indicator);

/**
 * @brief Stops indicator animation.
 *
 * @param indicator Target indicator.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_indicator_stop(struct cupertino_indicator *indicator);

/**
 * @brief Queries whether the indicator is animating.
 *
 * @param indicator Target indicator.
 * @param out_animating Pointer to receive animating state (1 if running, 0 if
 * stopped).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_indicator_is_animating(
    const struct cupertino_indicator *indicator, int *out_animating);

/**
 * @brief Advances stepped rotation timer for activity indicator.
 *
 * @param indicator Target indicator.
 * @param delta_ms Elapsed time in milliseconds.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_indicator_tick(struct cupertino_indicator *indicator, float delta_ms);

/**
 * @brief Retrieves calculated opacity for a specific radial spoke.
 *
 * Implements Apple's progressive alpha decay clockwise sequence.
 *
 * @param indicator Target indicator.
 * @param spoke_index Spoke index [0, spoke_count - 1].
 * @param out_opacity Pointer to receive spoke opacity in [0.08, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_indicator_get_spoke_opacity(
    const struct cupertino_indicator *indicator, size_t spoke_index,
    float *out_opacity);

/**
 * @brief Sets determinate progress value for progress bar.
 *
 * @param indicator Target indicator.
 * @param value Progress normalized to [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_indicator_set_value(
    struct cupertino_indicator *indicator, float value);

/**
 * @brief Retrieves current progress value.
 *
 * @param indicator Target indicator.
 * @param out_value Pointer to receive value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_indicator_get_value(
    const struct cupertino_indicator *indicator, float *out_value);

/**
 * @brief Retrieves physical dimensions in points.
 *
 * @param indicator Target indicator.
 * @param out_width Pointer to receive width in points.
 * @param out_height Pointer to receive height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_indicator_get_dimensions(const struct cupertino_indicator *indicator,
                                   float *out_width, float *out_height);

/**
 * @brief Retrieves underlying CDK progress base primitive.
 *
 * @param indicator Target indicator.
 * @param out_base Pointer to receive ui_progress_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_indicator_get_base(
    struct cupertino_indicator *indicator, struct ui_progress_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_INDICATOR_H */
