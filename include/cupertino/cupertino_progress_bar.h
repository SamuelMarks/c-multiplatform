#ifndef CUPERTINO_PROGRESS_BAR_H
#define CUPERTINO_PROGRESS_BAR_H

/**
 * @file cupertino_progress_bar.h
 * @brief Apple Cupertino style Progress Bar component.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cupertino/cupertino_color.h"
/* clang-format on */

/** @brief Cupertino Progress Bar Widget */
typedef struct CupertinoProgressBar {
  CMPWidget widget;        /**< Base widget interface. */
  CMPScalar progress;      /**< Current progress value (0.0 to 1.0). */
  CMPAnimController anim;  /**< Animation for smooth progress transitions. */
  CMPColor track_color;    /**< Color for the unfilled track. */
  CMPColor progress_color; /**< Color for the filled progress. */
  CMPRect bounds;          /**< Layout bounds. */
  CMPBool is_dark_mode;    /**< Dark mode styling. */
} CupertinoProgressBar;

/**
 * @brief Initializes a Cupertino Progress Bar.
 * @param bar Pointer to the progress bar instance.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_progress_bar_init(CupertinoProgressBar *bar);

/**
 * @brief Sets the progress value.
 * @param bar Pointer to the progress bar instance.
 * @param progress The new progress value (0.0 to 1.0).
 * @param animated Whether to animate the transition.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_progress_bar_set_progress(
    CupertinoProgressBar *bar, CMPScalar progress, CMPBool animated);

/**
 * @brief Updates the animations of the progress bar.
 * @param bar Pointer to the progress bar instance.
 * @param delta_time Elapsed time in seconds.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_progress_bar_update(CupertinoProgressBar *bar,
                                                   double delta_time);

/**
 * @brief Renders the progress bar.
 * @param bar Pointer to the progress bar instance.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_progress_bar_paint(
    const CupertinoProgressBar *bar, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_PROGRESS_BAR_H */
