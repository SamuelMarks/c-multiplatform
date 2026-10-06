/**
 * @file sampler_home.h
 * @brief Material 3 Catalog Home Screen Grid.
 */

#ifndef SAMPLER_HOME_H
#define SAMPLER_HOME_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include "sampler/sampler_nav.h"
#include "ui_engine.h"
#include "material3/md3_top_app_bar.h"
#include "material3/md3_card.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct sampler_home;

/**
 * @brief Creates a new sampler home screen instance.
 *
 * @param engine Pointer to ui_engine instance.
 * @param nav Pointer to the navigation instance.
 * @param out_screen Output pointer to receive the created instance.
 * @return SAMPLER_SUCCESS on success, or an appropriate error code.
 */
sampler_error_t sampler_home_create(struct ui_engine *engine,
                                    struct sampler_nav *nav,
                                    struct sampler_home **out_screen);

/**
 * @brief Retrieves the root DOM node for the sampler home screen.
 *
 * @param screen Pointer to the sampler home screen instance.
 * @param out_root Output pointer to receive the root DOM node.
 * @return SAMPLER_SUCCESS on success, or an appropriate error code.
 */
sampler_error_t sampler_home_get_root(const struct sampler_home *screen,
                                      struct ui_dom_node **out_root);

/**
 * @brief Destroys the given sampler home screen instance.
 *
 * @param screen Double pointer to the sampler home screen to destroy.
 * @return SAMPLER_SUCCESS on success, or an appropriate error code.
 */
sampler_error_t sampler_home_destroy(struct sampler_home **screen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SAMPLER_HOME_H */
