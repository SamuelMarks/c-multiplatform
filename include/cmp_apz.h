/* clang-format off */
#ifndef CMP_APZ_H
#define CMP_APZ_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque Asynchronous Pan/Zoom (APZ) Context
 */
typedef struct cmp_apz cmp_apz_t;

/**
 * @brief Create an APZ context for compositor-thread pinch/zoom
 * @param out_apz Pointer to receive the context
 * @return 0 on success
 */
int cmp_apz_create(cmp_apz_t **out_apz);

/**
 * @brief Destroy an APZ context
 * @param apz The context to destroy
 * @return 0 on success
 */
int cmp_apz_destroy(cmp_apz_t *apz);

/**
 * @brief Inject a raw touch/pan event into the APZ compositor state
 * @param apz The APZ context
 * @param delta_x Horizontal pan delta
 * @param delta_y Vertical pan delta
 * @param scale Pinch scale factor
 * @return 0 on success
 */
int cmp_apz_inject_gesture(cmp_apz_t *apz, float delta_x, float delta_y, float scale);

/**
 * @brief Get the computed transformation matrix for the compositor
 * @param apz The APZ context
 * @param out_matrix 16-element float array receiving the 4x4 transform
 * @return 0 on success
 */
int cmp_apz_get_transform(const cmp_apz_t *apz, float *out_matrix);
#ifdef __cplusplus
}
#endif
#endif /* CMP_APZ_H */
/* clang-format on */