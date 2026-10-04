/**
 * @file sampler_example_screen.h
 * @brief Interactive Example Canvas Screen for Compose Material Catalog.
 */

#ifndef C_MULTIPLATFORM_SAMPLER_EXAMPLE_SCREEN_H
#define C_MULTIPLATFORM_SAMPLER_EXAMPLE_SCREEN_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include "sampler/sampler_nav.h"
#include "sampler/sampler_models.h"
#include "ui_engine.h"
#include "ui_dom_node.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct sampler_example_screen;

/**
 * @brief Create an interactive example canvas screen.
 *
 * @param engine Pointer to the UI engine.
 * @param nav Pointer to the navigation router.
 * @param example The example model to display.
 * @param out_screen Output pointer to the created screen.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t
sampler_example_screen_create(struct ui_engine *engine, struct sampler_nav *nav,
                              const struct sampler_example *example,
                              struct sampler_example_screen **out_screen);

/**
 * @brief Get the root DOM node for the example screen.
 *
 * @param screen Pointer to the screen instance.
 * @param out_root Output pointer to the root DOM node.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t
sampler_example_screen_get_root(const struct sampler_example_screen *screen,
                                struct ui_dom_node **out_root);

/**
 * @brief Destroy an example screen.
 *
 * @param screen Pointer to the screen instance pointer.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t
sampler_example_screen_destroy(struct sampler_example_screen **screen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_MULTIPLATFORM_SAMPLER_EXAMPLE_SCREEN_H */
