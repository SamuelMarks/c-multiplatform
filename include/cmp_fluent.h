/* clang-format off */
#ifndef CMP_FLUENT_H
#define CMP_FLUENT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Opaque Fluent 2 Reveal Highlight Context
 */
typedef struct cmp_fluent_reveal cmp_fluent_reveal_t;

/**
 * @brief Create a reveal highlight context for proximity lighting
 * @param out_reveal Pointer to receive the context
 * @return 0 on success
 */
int cmp_fluent_reveal_create(cmp_fluent_reveal_t **out_reveal);

/**
 * @brief Destroy a reveal highlight context
 * @param reveal The context to destroy
 * @return 0 on success
 */
int cmp_fluent_reveal_destroy(cmp_fluent_reveal_t *reveal);

/**
 * @brief Update the pointer coordinates for the volumetric lighting shader
 * @param reveal The context
 * @param pointer_x X coordinate of the mouse
 * @param pointer_y Y coordinate of the mouse
 * @return 0 on success
 */
int cmp_fluent_reveal_update_pointer(cmp_fluent_reveal_t *reveal, float pointer_x, float pointer_y);

/**
 * @brief Opaque Fluent 2 Acrylic Noise Map
 */
typedef struct cmp_acrylic_noise cmp_acrylic_noise_t;

/**
 * @brief Generate a static high-frequency monochrome noise texture for Acrylic
 * @param width Width of the noise map
 * @param height Height of the noise map
 * @param out_noise Pointer to receive the noise context
 * @return 0 on success
 */
int cmp_acrylic_noise_create(int width, int height, cmp_acrylic_noise_t **out_noise);

/**
 * @brief Destroy an acrylic noise context
 * @param noise The context to destroy
 * @return 0 on success
 */
int cmp_acrylic_noise_destroy(cmp_acrylic_noise_t *noise);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CMP_FLUENT_H */
/* clang-format on */