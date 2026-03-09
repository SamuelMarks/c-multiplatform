#ifndef F2_PROGRESS_H
#define F2_PROGRESS_H

/**
 * @file f2_progress.h
 * @brief Microsoft Fluent 2 progress indicators.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "f2/f2_color.h"

/** @brief Default linear progress height. */
#define F2_LINEAR_PROGRESS_DEFAULT_HEIGHT 2.0f
/** @brief Default circular progress size. */
#define F2_CIRCULAR_PROGRESS_DEFAULT_SIZE 32.0f
/** @brief Default circular progress stroke thickness. */
#define F2_CIRCULAR_PROGRESS_DEFAULT_THICKNESS 2.0f

/**
 * @brief Fluent 2 Linear Progress style.
 */
typedef struct F2LinearProgressStyle {
  CMPScalar height;     /**< Track height. */
  CMPColor track_color; /**< Track color. */
  CMPColor fill_color;  /**< Fill color. */
} F2LinearProgressStyle;

/**
 * @brief Fluent 2 Linear Progress widget.
 */
typedef struct F2LinearProgress {
  CMPWidget widget;            /**< Widget interface. */
  F2LinearProgressStyle style; /**< Current style. */
  CMPRect bounds;              /**< Layout bounds. */
  CMPScalar value; /**< Progress value (0.0 to 1.0). If < 0, indeterminate. */
} F2LinearProgress;

/**
 * @brief Initialize a Fluent 2 linear progress style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
f2_linear_progress_style_init(F2LinearProgressStyle *style);

/**
 * @brief Initialize a Fluent 2 linear progress widget.
 * @param progress Progress indicator instance.
 * @param style Style descriptor.
 * @param value Initial value (0.0 to 1.0, or -1.0 for indeterminate).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_linear_progress_init(F2LinearProgress *progress,
                                             const F2LinearProgressStyle *style,
                                             CMPScalar value);

/**
 * @brief Set linear progress value.
 * @param progress Progress indicator instance.
 * @param value New value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_linear_progress_set_value(F2LinearProgress *progress,
                                                  CMPScalar value);

/**
 * @brief Fluent 2 Circular Progress (Progress Ring) style.
 */
typedef struct F2CircularProgressStyle {
  CMPScalar size;       /**< Ring diameter. */
  CMPScalar thickness;  /**< Stroke thickness. */
  CMPColor track_color; /**< Unfilled track color. */
  CMPColor fill_color;  /**< Filled color. */
} F2CircularProgressStyle;

/**
 * @brief Fluent 2 Circular Progress (Progress Ring) widget.
 */
typedef struct F2CircularProgress {
  CMPWidget widget;              /**< Widget interface. */
  F2CircularProgressStyle style; /**< Current style. */
  CMPRect bounds;                /**< Layout bounds. */
  CMPScalar value; /**< Progress value (0.0 to 1.0). If < 0, indeterminate. */
} F2CircularProgress;

/**
 * @brief Initialize a Fluent 2 circular progress style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
f2_circular_progress_style_init(F2CircularProgressStyle *style);

/**
 * @brief Initialize a Fluent 2 circular progress widget.
 * @param progress Progress indicator instance.
 * @param style Style descriptor.
 * @param value Initial value (0.0 to 1.0, or -1.0 for indeterminate).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_circular_progress_init(
    F2CircularProgress *progress, const F2CircularProgressStyle *style,
    CMPScalar value);

/**
 * @brief Set circular progress value.
 * @param progress Progress indicator instance.
 * @param value New value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
f2_circular_progress_set_value(F2CircularProgress *progress, CMPScalar value);

#ifdef __cplusplus
}
#endif

#endif /* F2_PROGRESS_H */
