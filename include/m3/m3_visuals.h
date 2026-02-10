#ifndef M3_VISUALS_H
#define M3_VISUALS_H

/**
 * @file m3_visuals.h
 * @brief Visual primitives (ripple, shadow) for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_anim.h"
#include "m3_api_gfx.h"

/** @brief Ripple is idle (not visible). */
#define M3_RIPPLE_STATE_IDLE 0
/** @brief Ripple is expanding. */
#define M3_RIPPLE_STATE_EXPANDING 1
/** @brief Ripple is fading out. */
#define M3_RIPPLE_STATE_FADING 2

/**
 * @brief Ripple effect state.
 */
typedef struct M3Ripple {
    M3AnimController radius_anim; /**< Radius animation controller. */
    M3AnimController opacity_anim; /**< Opacity animation controller. */
    M3Scalar center_x; /**< Ripple center X in pixels. */
    M3Scalar center_y; /**< Ripple center Y in pixels. */
    M3Scalar max_radius; /**< Maximum ripple radius in pixels. */
    M3Scalar radius; /**< Current ripple radius in pixels. */
    M3Scalar opacity; /**< Opacity multiplier (0..1). */
    M3Color color; /**< Base ripple color (alpha is multiplied by opacity). */
    m3_u32 state; /**< Ripple state (M3_RIPPLE_STATE_*). */
} M3Ripple;

/**
 * @brief Drop shadow descriptor.
 */
typedef struct M3Shadow {
    M3Scalar offset_x; /**< Shadow X offset in pixels. */
    M3Scalar offset_y; /**< Shadow Y offset in pixels. */
    M3Scalar blur_radius; /**< Blur radius in pixels. */
    M3Scalar spread; /**< Spread radius in pixels. */
    M3Scalar corner_radius; /**< Corner radius in pixels. */
    m3_u32 layers; /**< Number of shadow layers to draw (>= 1). */
    M3Color color; /**< Base shadow color. */
} M3Shadow;

/**
 * @brief Initialize a ripple state.
 * @param ripple Ripple instance to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ripple_init(M3Ripple *ripple);

/**
 * @brief Start a ripple expansion.
 * @param ripple Ripple instance.
 * @param center_x Center X coordinate in pixels.
 * @param center_y Center Y coordinate in pixels.
 * @param max_radius Maximum radius in pixels (> 0).
 * @param expand_duration Expansion duration in seconds (>= 0).
 * @param color Base ripple color (RGBA in 0..1 range).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ripple_start(M3Ripple *ripple, M3Scalar center_x, M3Scalar center_y, M3Scalar max_radius,
    M3Scalar expand_duration, M3Color color);

/**
 * @brief Release a ripple (begin fade-out).
 * @param ripple Ripple instance.
 * @param fade_duration Fade duration in seconds (>= 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ripple_release(M3Ripple *ripple, M3Scalar fade_duration);

/**
 * @brief Advance the ripple simulation.
 * @param ripple Ripple instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_finished Receives M3_TRUE when the ripple is finished.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ripple_step(M3Ripple *ripple, M3Scalar dt, M3Bool *out_finished);

/**
 * @brief Query whether a ripple is active.
 * @param ripple Ripple instance.
 * @param out_active Receives M3_TRUE when active.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ripple_is_active(const M3Ripple *ripple, M3Bool *out_active);

/**
 * @brief Compute the maximum ripple radius required to cover a rectangle.
 * @param bounds Bounds rectangle.
 * @param center_x Center X coordinate in pixels.
 * @param center_y Center Y coordinate in pixels.
 * @param out_radius Receives the maximum radius.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ripple_compute_max_radius(const M3Rect *bounds, M3Scalar center_x, M3Scalar center_y,
    M3Scalar *out_radius);

/**
 * @brief Paint a ripple using a graphics backend.
 * @param ripple Ripple instance.
 * @param gfx Graphics backend interface.
 * @param clip Optional clip rectangle (NULL to skip clipping).
 * @param corner_radius Corner radius for the ripple shape (>= 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_ripple_paint(const M3Ripple *ripple, M3Gfx *gfx, const M3Rect *clip, M3Scalar corner_radius);

/**
 * @brief Initialize a shadow descriptor.
 * @param shadow Shadow instance to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_shadow_init(M3Shadow *shadow);

/**
 * @brief Configure a shadow descriptor.
 * @param shadow Shadow instance.
 * @param offset_x Shadow X offset in pixels.
 * @param offset_y Shadow Y offset in pixels.
 * @param blur_radius Blur radius in pixels (>= 0).
 * @param spread Spread radius in pixels (>= 0).
 * @param corner_radius Corner radius in pixels (>= 0).
 * @param layers Number of layers to draw (>= 1).
 * @param color Base shadow color (RGBA in 0..1 range).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_shadow_set(M3Shadow *shadow, M3Scalar offset_x, M3Scalar offset_y, M3Scalar blur_radius,
    M3Scalar spread, M3Scalar corner_radius, m3_u32 layers, M3Color color);

/**
 * @brief Paint a shadow for a rectangle.
 * @param shadow Shadow descriptor.
 * @param gfx Graphics backend interface.
 * @param rect Rectangle to shadow.
 * @param clip Optional clip rectangle (NULL to skip clipping).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_shadow_paint(const M3Shadow *shadow, M3Gfx *gfx, const M3Rect *rect, const M3Rect *clip);

#ifdef M3_TESTING
/**
 * @brief Set a visuals test fail point.
 * @param fail_point Fail point identifier.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_visuals_test_set_fail_point(m3_u32 fail_point);

/**
 * @brief Clear visuals test fail points.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_visuals_test_clear_fail_points(void);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_visuals_test_validate_rect(const M3Rect *rect);

/**
 * @brief Test wrapper for color validation.
 * @param color Color to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_visuals_test_validate_color(const M3Color *color);

/**
 * @brief Test wrapper for graphics backend validation.
 * @param gfx Graphics backend.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_visuals_test_validate_gfx(const M3Gfx *gfx);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_VISUALS_H */
