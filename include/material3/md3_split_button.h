/**
 * @file md3_split_button.h
 * @brief Material 3 Expressive Split Button component wrapping
 * ui_split_button_base.
 */

#ifndef MATERIAL3_MD3_SPLIT_BUTTON_H
#define MATERIAL3_MD3_SPLIT_BUTTON_H

/* clang-format off */
#include "material3/md3_button.h"
#include "ui_split_button_base.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @struct md3_split_button
 * @brief Material 3 Expressive Split Button skin wrapping ui_split_button_base.
 */
struct md3_split_button {
  struct ui_split_button_base *base;
  enum md3_button_variant variant;
};

/**
 * @brief Creates a Material 3 split button wrapping ui_split_button_base.
 *
 * @param engine Pointer to ui_engine instance.
 * @param variant Visual button variant (Elevated, Filled, Filled Tonal,
 * Outlined, Text).
 * @param out_split_button Pointer to receive newly created split button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_split_button_create(
    struct ui_engine *engine, enum md3_button_variant variant,
    struct md3_split_button **out_split_button);

/**
 * @brief Destroys a Material 3 split button and its underlying base.
 *
 * @param split_button Split button to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_split_button_destroy(struct md3_split_button *split_button);

/**
 * @brief Retrieves the underlying ui_split_button_base handle.
 *
 * @param split_button The split button.
 * @param out_base Pointer to receive the base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_split_button_get_base(struct md3_split_button *split_button,
                          struct ui_split_button_base **out_base);

/**
 * @brief Gets the primary action button component.
 *
 * @param split_button The split button.
 * @param out_main_btn Pointer to receive the primary action button base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_split_button_get_main_button(struct md3_split_button *split_button,
                                 struct ui_button_base **out_main_btn);

/**
 * @brief Gets the trailing dropdown trigger button component.
 *
 * @param split_button The split button.
 * @param out_trigger_btn Pointer to receive the trigger button base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_split_button_get_trigger_button(struct md3_split_button *split_button,
                                    struct ui_button_base **out_trigger_btn);

/**
 * @brief Sets the disabled state of the split button.
 *
 * @param split_button The split button.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_split_button_set_disabled(
    struct md3_split_button *split_button, int disabled);

/**
 * @brief Sets text on the primary action button.
 *
 * @param split_button The split button.
 * @param text The text to set.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_split_button_set_text(
    struct md3_split_button *split_button, const char *text);

/**
 * @brief Sets click handler on the primary action button.
 *
 * @param split_button The split button.
 * @param on_click Callback function.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_split_button_set_on_click(struct md3_split_button *split_button,
                              ui_button_on_click_t on_click, void *user_data);

/**
 * @brief Sets click handler on the dropdown trigger button.
 *
 * @param split_button The split button.
 * @param on_trigger Callback function for dropdown click.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_split_button_set_on_trigger_click(
    struct md3_split_button *split_button, ui_button_on_click_t on_trigger,
    void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_SPLIT_BUTTON_H */
