/* clang-format off */
#ifndef M3_SHAPES_H
#define M3_SHAPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Material 3 Shape Scales.
 */
typedef enum m3_shape_scale_t {
    M3_SHAPE_NONE = 0,
    M3_SHAPE_EXTRA_SMALL, /* 4dp */
    M3_SHAPE_SMALL,       /* 8dp */
    M3_SHAPE_MEDIUM,      /* 12dp */
    M3_SHAPE_LARGE,       /* 16dp */
    M3_SHAPE_EXTRA_LARGE, /* 28dp */
    M3_SHAPE_FULL         /* Circle/Pill */
} m3_shape_scale_t;

/**
 * @brief Representation of an M3 Shape, supporting asymmetric corners.
 */
typedef struct m3_shape_t {
    float top_left_dp;
    float top_right_dp;
    float bottom_right_dp;
    float bottom_left_dp;
    int is_full; /* If 1, radiuses are ignored and it draws as a pill/circle based on rect bounds */
} m3_shape_t;

/**
 * @brief Initializes a uniform shape based on an M3 scale.
 * @param scale The shape scale.
 * @param out_shape Pointer to store the initialized shape.
 * @return 0 on success, non-zero on error.
 */
int m3_shape_init_uniform(m3_shape_scale_t scale, m3_shape_t *out_shape);

/**
 * @brief Initializes an asymmetric shape (e.g., for bottom sheets: top rounded, bottom flat).
 * @param top_left The top-left scale.
 * @param top_right The top-right scale.
 * @param bottom_right The bottom-right scale.
 * @param bottom_left The bottom-left scale.
 * @param out_shape Pointer to store the initialized shape.
 * @return 0 on success, non-zero on error.
 */
int m3_shape_init_asymmetric(m3_shape_scale_t top_left, m3_shape_scale_t top_right,
                             m3_shape_scale_t bottom_right, m3_shape_scale_t bottom_left,
                             m3_shape_t *out_shape);

/**
 * @brief Interpolates between two shapes (e.g., FAB expanding to Bottom Sheet).
 * @param start The initial shape.
 * @param end The target shape.
 * @param t Interpolation factor (0.0 to 1.0).
 * @param out_shape Pointer to store the interpolated shape.
 * @return 0 on success, non-zero on error.
 */
int m3_shape_tween(const m3_shape_t *start, const m3_shape_t *end, float t, m3_shape_t *out_shape);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_SHAPES_H */
/* clang-format on */