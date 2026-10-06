/**
 * @file m2_home.h
 * @brief Material 2 Catalog Home Screen.
 */

#ifndef C_MULTIPLATFORM_M2_HOME_H
#define C_MULTIPLATFORM_M2_HOME_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include "sampler/sampler_nav.h"
#include "ui_engine.h"
#include "ui_dom_node.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct m2_home_screen;

/**
 * @brief Create the Material 2 catalog home screen.
 *
 * @param engine Pointer to the UI engine.
 * @param nav Pointer to the navigation router.
 * @param out_screen Output pointer to the created screen.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t m2_home_create(struct ui_engine *engine,
                                      struct sampler_nav *nav,
                                      struct m2_home_screen **out_screen);

/**
 * @brief Get the root DOM node for the M2 home screen.
 *
 * @param screen Pointer to the screen instance.
 * @param out_root Output pointer to the root DOM node.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t m2_home_get_root(const struct m2_home_screen *screen,
                                        struct ui_dom_node **out_root);

/**
 * @brief Destroy the M2 home screen.
 *
 * @param screen Pointer to the screen instance pointer.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t m2_home_destroy(struct m2_home_screen **screen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_MULTIPLATFORM_M2_HOME_H */
