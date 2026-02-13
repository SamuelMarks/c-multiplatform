#ifndef CMP_VISUALS_H
#define CMP_VISUALS_H

/**
 * @file cmp_visuals.h
 * @brief Visual primitives (ripple, shadow) for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_api_gfx.h"

/** @brief Ripple is idle (not visible). */
#define CMP_RIPPLE_STATE_IDLE 0
/** @brief Ripple is expanding. */
#define CMP_RIPPLE_STATE_EXPANDING 1
/** @brief Ripple is fading out. */
#define CMP_RIPPLE_STATE_FADING 2

/**
 * @brief Ripple effect state.
 */
typedef struct CMPRipple {
  CMPAnimController radius_anim;  /**< Radius animation controller. */
  CMPAnimController opacity_anim; /**< Opacity animation controller. */
  CMPScalar center_x;             /**< Ripple center X in pixels. */
  CMPScalar center_y;             /**< Ripple center Y in pixels. */
  CMPScalar max_radius;           /**< Maximum ripple radius in pixels. */
  CMPScalar radius;               /**< Current ripple radius in pixels. */
  CMPScalar opacity;              /**< Opacity multiplier (0..1). */
  CMPColor color; /**< Base ripple color (alpha is multiplied by opacity). */
  cmp_u32 state;  /**< Ripple state (CMP_RIPPLE_STATE_*). */
} CMPRipple;

/**
 * @brief Drop shadow descriptor.
 */
typedef struct CMPShadow {
  CMPScalar offset_x;      /**< Shadow X offset in pixels. */
  CMPScalar offset_y;      /**< Shadow Y offset in pixels. */
  CMPScalar blur_radius;   /**< Blur radius in pixels. */
  CMPScalar spread;        /**< Spread radius in pixels. */
  CMPScalar corner_radius; /**< Corner radius in pixels. */
  cmp_u32 layers;          /**< Number of shadow layers to draw (>= 1). */
  CMPColor color;          /**< Base shadow color. */
} CMPShadow;

/**
 * @brief Initialize a ripple state.
 * @param ripple Ripple instance to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ripple_init(CMPRipple *ripple);

/**
 * @brief Start a ripple expansion.
 * @param ripple Ripple instance.
 * @param center_x Center X coordinate in pixels.
 * @param center_y Center Y coordinate in pixels.
 * @param max_radius Maximum radius in pixels (> 0).
 * @param expand_duration Expansion duration in seconds (>= 0).
 * @param color Base ripple color (RGBA in 0..1 range).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ripple_start(CMPRipple *ripple, CMPScalar center_x,
                                      CMPScalar center_y, CMPScalar max_radius,
                                      CMPScalar expand_duration,
                                      CMPColor color);

/**
 * @brief Release a ripple (begin fade-out).
 * @param ripple Ripple instance.
 * @param fade_duration Fade duration in seconds (>= 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ripple_release(CMPRipple *ripple,
                                        CMPScalar fade_duration);

/**
 * @brief Advance the ripple simulation.
 * @param ripple Ripple instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_finished Receives CMP_TRUE when the ripple is finished.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ripple_step(CMPRipple *ripple, CMPScalar dt,
                                     CMPBool *out_finished);

/**
 * @brief Query whether a ripple is active.
 * @param ripple Ripple instance.
 * @param out_active Receives CMP_TRUE when active.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ripple_is_active(const CMPRipple *ripple,
                                          CMPBool *out_active);

/**
 * @brief Compute the maximum ripple radius required to cover a rectangle.
 * @param bounds Bounds rectangle.
 * @param center_x Center X coordinate in pixels.
 * @param center_y Center Y coordinate in pixels.
 * @param out_radius Receives the maximum radius.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ripple_compute_max_radius(const CMPRect *bounds,
                                                   CMPScalar center_x,
                                                   CMPScalar center_y,
                                                   CMPScalar *out_radius);

/**
 * @brief Paint a ripple using a graphics backend.
 * @param ripple Ripple instance.
 * @param gfx Graphics backend interface.
 * @param clip Optional clip rectangle (NULL to skip clipping).
 * @param corner_radius Corner radius for the ripple shape (>= 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ripple_paint(const CMPRipple *ripple, CMPGfx *gfx,
                                      const CMPRect *clip,
                                      CMPScalar corner_radius);

/**
 * @brief Initialize a shadow descriptor.
 * @param shadow Shadow instance to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_shadow_init(CMPShadow *shadow);

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
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_shadow_set(CMPShadow *shadow, CMPScalar offset_x,
                                    CMPScalar offset_y, CMPScalar blur_radius,
                                    CMPScalar spread, CMPScalar corner_radius,
                                    cmp_u32 layers, CMPColor color);

/**
 * @brief Paint a shadow for a rectangle.
 * @param shadow Shadow descriptor.
 * @param gfx Graphics backend interface.
 * @param rect Rectangle to shadow.
 * @param clip Optional clip rectangle (NULL to skip clipping).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_shadow_paint(const CMPShadow *shadow, CMPGfx *gfx,
                                      const CMPRect *rect, const CMPRect *clip);

#ifdef CMP_TESTING
/** @brief Fail point: force shadow normalization error. */
#ifndef CMP_VISUALS_TEST_FAIL_SHADOW_NORM
#define CMP_VISUALS_TEST_FAIL_SHADOW_NORM 18u
#endif
/** @brief Fail point: force shadow init error. */
#ifndef CMP_VISUALS_TEST_FAIL_SHADOW_INIT
#define CMP_VISUALS_TEST_FAIL_SHADOW_INIT 19u
#endif
/**
 * @brief Set a visuals test fail point.
 * @param fail_point Fail point identifier.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_visuals_test_set_fail_point(cmp_u32 fail_point);

/**
 * @brief Clear visuals test fail points.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_visuals_test_clear_fail_points(void);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_visuals_test_validate_rect(const CMPRect *rect);

/**
 * @brief Test wrapper for color validation.
 * @param color Color to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_visuals_test_validate_color(const CMPColor *color);

/**
 * @brief Test wrapper for graphics backend validation.
 * @param gfx Graphics backend.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_visuals_test_validate_gfx(const CMPGfx *gfx);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_VISUALS_H */
