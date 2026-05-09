
#ifndef CMP_LOTTIE_H
#define CMP_LOTTIE_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Opaque Lottie/Rive Animation Context
 */
typedef struct cmp_lottie cmp_lottie_t;

/**
 * @brief Parse and create a Lottie animation from a JSON buffer
 * @param json_buffer The raw JSON data
 * @param size Size of the buffer in bytes
 * @param out_lottie Pointer to receive the context
 * @return 0 on success
 */
int cmp_lottie_create(const char *json_buffer, size_t size,
                      cmp_lottie_t **out_lottie);

/**
 * @brief Destroy a Lottie animation context
 * @param lottie The context to destroy
 * @return 0 on success
 */
int cmp_lottie_destroy(cmp_lottie_t *lottie);

/**
 * @brief Step the Lottie animation by delta time
 * @param lottie The context
 * @param dt_ms Delta time in milliseconds
 * @return 0 on success
 */
int cmp_lottie_step(cmp_lottie_t *lottie, float dt_ms);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CMP_LOTTIE_H */
