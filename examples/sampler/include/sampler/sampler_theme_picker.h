/**
 * @file sampler_theme_picker.h
 * @brief Theme Picker Modal Bottom Sheet for Compose Material Catalog.
 */

#ifndef C_MULTIPLATFORM_SAMPLER_THEME_PICKER_H
#define C_MULTIPLATFORM_SAMPLER_THEME_PICKER_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include "ui_engine.h"
#include "ui_dom_node.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct sampler_theme_picker;

/**
 * @brief Create the theme picker modal bottom sheet.
 *
 * @param engine Pointer to the UI engine.
 * @param out_picker Output pointer to the created picker instance.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t
sampler_theme_picker_create(struct ui_engine *engine,
                            struct sampler_theme_picker **out_picker);

/**
 * @brief Get the root DOM node for the theme picker.
 *
 * @param picker Pointer to the picker instance.
 * @param out_root Output pointer to the root DOM node.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t
sampler_theme_picker_get_root(const struct sampler_theme_picker *picker,
                              struct ui_dom_node **out_root);

/**
 * @brief Destroy the theme picker instance.
 *
 * @param picker Pointer to the picker instance pointer.
 * @return sampler_error_t SAMPLER_SUCCESS on success.
 */
extern sampler_error_t
sampler_theme_picker_destroy(struct sampler_theme_picker **picker);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_MULTIPLATFORM_SAMPLER_THEME_PICKER_H */
