#ifndef CUPERTINO_ACTIVITY_INDICATOR_H
#define CUPERTINO_ACTIVITY_INDICATOR_H

/**
 * @file cupertino_activity_indicator.h
 * @brief Apple Cupertino style Activity Indicator (spinner).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_api_gfx.h"
#include "cupertino/cupertino_color.h"

/** @brief Activity Indicator Sizes */
typedef enum CupertinoActivityIndicatorSize {
    CUPERTINO_ACTIVITY_INDICATOR_SIZE_MEDIUM = 0, /* Standard size (e.g. 20x20) */
    CUPERTINO_ACTIVITY_INDICATOR_SIZE_LARGE       /* Large size (e.g. 37x37) */
} CupertinoActivityIndicatorSize;

/** @brief Cupertino Activity Indicator Widget */
typedef struct CupertinoActivityIndicator {
    CMPWidget widget;                         /**< Base widget interface. */
    CupertinoActivityIndicatorSize size;      /**< Size variant. */
    CMPColor color;                           /**< Tint color (defaults to gray). */
    CMPRect bounds;                           /**< Layout bounds. */
    CMPBool is_animating;                     /**< Animation state. */
    CMPBool hides_when_stopped;               /**< Whether to hide when not animating. */
    CMPBool is_dark_mode;                     /**< Dark mode styling. */
    double animation_time;                    /**< Internal animation timer. */
} CupertinoActivityIndicator;

/**
 * @brief Initializes a Cupertino Activity Indicator.
 * @param indicator Pointer to the indicator instance to initialize.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_activity_indicator_init(CupertinoActivityIndicator *indicator);

/**
 * @brief Starts animating the indicator.
 * @param indicator Pointer to the indicator instance.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_activity_indicator_start_animating(CupertinoActivityIndicator *indicator);

/**
 * @brief Stops animating the indicator.
 * @param indicator Pointer to the indicator instance.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_activity_indicator_stop_animating(CupertinoActivityIndicator *indicator);

/**
 * @brief Updates the animation state.
 * @param indicator Pointer to the indicator instance.
 * @param delta_time The elapsed time since the last update, in seconds.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_activity_indicator_update(CupertinoActivityIndicator *indicator, double delta_time);

/**
 * @brief Renders the activity indicator.
 * @param indicator Pointer to the indicator instance.
 * @param ctx Pointer to the paint context used for drawing.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_activity_indicator_paint(const CupertinoActivityIndicator *indicator, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_ACTIVITY_INDICATOR_H */
