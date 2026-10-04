/**
 * @file sampler_component_screen.h
 * @brief Component Details Screen for Compose Material Catalog.
 */

#ifndef C_MULTIPLATFORM_SAMPLER_COMPONENT_SCREEN_H
#define C_MULTIPLATFORM_SAMPLER_COMPONENT_SCREEN_H

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

struct sampler_component_screen;

/**
 * @brief Create a component details screen.
 *
 * @param engine Pointer to the UI engine.
 * @param nav Pointer to the navigation router.
 * @param component The component model to display.
 * @param out_screen Output pointer to the created screen.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t
sampler_component_screen_create(struct ui_engine *engine,
                                struct sampler_nav *nav,
                                const struct sampler_component *component,
                                struct sampler_component_screen **out_screen);

/**
 * @brief Get the root DOM node for the component screen.
 *
 * @param screen Pointer to the screen instance.
 * @param out_root Output pointer to the root DOM node.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t
sampler_component_screen_get_root(const struct sampler_component_screen *screen,
                                  struct ui_dom_node **out_root);

/**
 * @brief Destroy a component details screen.
 *
 * @param screen Pointer to the screen instance pointer.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t
sampler_component_screen_destroy(struct sampler_component_screen **screen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_MULTIPLATFORM_SAMPLER_COMPONENT_SCREEN_H */
