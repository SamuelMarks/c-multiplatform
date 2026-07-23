#ifndef UI_EFFECTS_SHADERS_H
#define UI_EFFECTS_SHADERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

struct ui_shader_manager;

/**
 * @brief Compiles and retrieves a Gaussian Blur shader program (horizontal
 * pass).
 *
 * @param manager The shader manager.
 * @param out_program_id Pointer to receive the compiled program ID.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_effects_shaders_get_blur_h_program(struct ui_shader_manager *manager,
                                      unsigned int *out_program_id);

/**
 * @brief Compiles and retrieves a Gaussian Blur shader program (vertical pass).
 *
 * @param manager The shader manager.
 * @param out_program_id Pointer to receive the compiled program ID.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_effects_shaders_get_blur_v_program(struct ui_shader_manager *manager,
                                      unsigned int *out_program_id);

/**
 * @brief Compiles and retrieves a blend shader program for a specific CSS blend
 * mode.
 *
 * @param manager The shader manager.
 * @param mode The blend mode.
 * @param out_program_id Pointer to receive the compiled program ID.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_effects_shaders_get_blend_program(struct ui_shader_manager *manager,
                                     enum ui_css_blend_mode mode,
                                     unsigned int *out_program_id);

enum ui_error
ui_effects_shaders_get_color_filter_program(struct ui_shader_manager *manager,
                                            enum ui_css_filter_type type,
                                            unsigned int *out_program);
enum ui_error
ui_effects_shaders_get_drop_shadow_program(struct ui_shader_manager *manager,
                                           unsigned int *out_program);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_EFFECTS_SHADERS_H */
