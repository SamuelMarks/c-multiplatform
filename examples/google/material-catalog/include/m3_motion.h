/* clang-format off */
#ifndef M3_MOTION_H
#define M3_MOTION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cmp.h>

/**
 * @brief Material 3 Motion Durations in milliseconds.
 */
typedef enum m3_duration_t {
    /* Short */
    M3_DUR_SHORT_1 = 50,
    M3_DUR_SHORT_2 = 100,
    M3_DUR_SHORT_3 = 150,
    M3_DUR_SHORT_4 = 200,

    /* Medium */
    M3_DUR_MEDIUM_1 = 250,
    M3_DUR_MEDIUM_2 = 300,
    M3_DUR_MEDIUM_3 = 350,
    M3_DUR_MEDIUM_4 = 400,

    /* Long */
    M3_DUR_LONG_1 = 450,
    M3_DUR_LONG_2 = 500,
    M3_DUR_LONG_3 = 550,
    M3_DUR_LONG_4 = 600,

    /* Extra Long */
    M3_DUR_EXTRA_LONG_1 = 700,
    M3_DUR_EXTRA_LONG_2 = 800,
    M3_DUR_EXTRA_LONG_3 = 900,
    M3_DUR_EXTRA_LONG_4 = 1000
} m3_duration_t;

/**
 * @brief Material 3 Easing Curve Types.
 */
typedef enum m3_easing_type_t {
    M3_EASING_EMPHASIZED = 0,
    M3_EASING_EMPHASIZED_DECELERATE,
    M3_EASING_EMPHASIZED_ACCELERATE,
    M3_EASING_STANDARD,
    M3_EASING_STANDARD_DECELERATE,
    M3_EASING_STANDARD_ACCELERATE,
    M3_EASING_COUNT
} m3_easing_type_t;

/**
 * @brief Represents the global motion context containing pre-allocated bezier curves.
 */
typedef struct m3_motion_ctx_t {
    cmp_bezier_ease_t *curves[M3_EASING_COUNT];
} m3_motion_ctx_t;

/**
 * @brief Initializes the M3 motion context, allocating all standard easing curves.
 * @param ctx Pointer to the motion context to initialize.
 * @return 0 on success, non-zero on error.
 */
int m3_motion_init(m3_motion_ctx_t *ctx);

/**
 * @brief Cleans up the M3 motion context.
 * @param ctx Pointer to the motion context to clean up.
 * @return 0 on success, non-zero on error.
 */
int m3_motion_cleanup(m3_motion_ctx_t *ctx);

/**
 * @brief Evaluates an easing curve for a given progress [0.0, 1.0].
 * @param ctx The motion context.
 * @param type The type of easing curve.
 * @param t The linear progress.
 * @param out_val The eased progress.
 * @return 0 on success, non-zero on error.
 */
int m3_motion_evaluate(const m3_motion_ctx_t *ctx, m3_easing_type_t type, float t, float *out_val);

/**
 * @brief Computes a shared axis transition offset.
 * @param t Linear progress.
 * @param is_forward 1 if navigating forward, 0 if backward.
 * @param distance The total physical distance in pixels.
 * @param out_offset Evaluated positional offset for the outgoing/incoming elements.
 * @return 0 on success, non-zero on error.
 */
int m3_motion_shared_axis_offset(const m3_motion_ctx_t *ctx, float t, int is_forward, float distance, float *out_offset);

/**
 * @brief Computes the shared axis fade opacities (cross-fade logic).
 * @param t Linear progress.
 * @param out_outgoing_opacity Computed opacity for the exiting view.
 * @param out_incoming_opacity Computed opacity for the entering view.
 * @return 0 on success, non-zero on error.
 */
int m3_motion_shared_axis_fade(const m3_motion_ctx_t *ctx, float t, float *out_outgoing_opacity, float *out_incoming_opacity);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_MOTION_H */
/* clang-format on */