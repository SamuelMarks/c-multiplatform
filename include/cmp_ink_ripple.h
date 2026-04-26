/* clang-format off */
#ifndef CMP_INK_RIPPLE_H
#define CMP_INK_RIPPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for an ink ripple effect.
 */
typedef struct cmp_ink_ripple cmp_ink_ripple_t;

/**
 * @brief Create an ink ripple.
 * @param out_ripple Pointer to receive the ripple handle.
 * @return 0 on success, or an error code.
 */
int cmp_ink_ripple_create(cmp_ink_ripple_t **out_ripple);

/**
 * @brief Destroy an ink ripple.
 * @param ripple The ripple handle.
 * @return 0 on success, or an error code.
 */
int cmp_ink_ripple_destroy(cmp_ink_ripple_t *ripple);

/**
 * @brief Update the ripple animation.
 * @param ripple The ripple handle.
 * @param dt_ms Delta time in milliseconds.
 * @return 0 on success, or an error code.
 */
int cmp_ink_ripple_update(cmp_ink_ripple_t *ripple, float dt_ms);

/**
 * @brief Trigger a new ripple expansion.
 * @param ripple The ripple handle.
 * @param start_x The X origin of the ripple.
 * @param start_y The Y origin of the ripple.
 * @param box_width The width of the container box.
 * @param box_height The height of the container box.
 * @return 0 on success, or an error code.
 */
int cmp_ink_ripple_trigger(cmp_ink_ripple_t *ripple, float start_x, float start_y, float box_width, float box_height);

/**
 * @brief Begin fading the ripple out (e.g. pointer released).
 * @param ripple The ripple handle.
 * @return 0 on success, or an error code.
 */
int cmp_ink_ripple_release(cmp_ink_ripple_t *ripple);

/**
 * @brief Get the current visual state of the ripple.
 * @param ripple The ripple handle.
 * @param out_radius Pointer to receive the current radius (may be NULL).
 * @param out_opacity Pointer to receive the current opacity (may be NULL).
 * @param out_x Pointer to receive the origin X (may be NULL).
 * @param out_y Pointer to receive the origin Y (may be NULL).
 * @return 0 on success, or an error code.
 */
int cmp_ink_ripple_get_state(cmp_ink_ripple_t *ripple, float *out_radius, float *out_opacity, float *out_x, float *out_y);

#ifdef __cplusplus
}
#endif
#endif /* CMP_INK_RIPPLE_H */
/* clang-format on */