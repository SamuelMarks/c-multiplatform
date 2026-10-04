/**
 * @file md2_button.h
 * @brief Material Design 2 Button component wrapping ui_button_base.
 */

#ifndef MATERIAL2_MD2_BUTTON_H
#define MATERIAL2_MD2_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_button_base.h"
#include <stddef.h>
/* clang-format on */

struct ui_engine;
struct md2_button;

/**
 * @brief Material 2 Button visual variants.
 */
enum md2_button_variant {
  MD2_BUTTON_CONTAINED, /**< Raised button, 2dp resting elevation. */
  MD2_BUTTON_OUTLINED,  /**< Outlined button, 0dp resting elevation. */
  MD2_BUTTON_TEXT       /**< Flat/Text button, 0dp resting elevation. */
};

/**
 * @brief Creates a Material 2 Button component.
 *
 * @param engine Pointer to ui_engine.
 * @param variant Visual variant (Contained, Outlined, Text).
 * @param text The button text (will be uppercase transformed automatically).
 * @param out_button Pointer to receive newly created button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_button_create(struct ui_engine *engine, enum md2_button_variant variant,
                  const char *text, struct md2_button **out_button);

/**
 * @brief Destroys a Material 2 Button component.
 *
 * @param button Button to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_button_destroy(struct md2_button *button);

/**
 * @brief Sets the text of the button.
 *
 * @param button The button.
 * @param text The text to set.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_button_set_text(struct md2_button *button, const char *text);

/**
 * @brief Sets the click handler for the button.
 *
 * @param button The button.
 * @param on_click Callback function.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_button_set_on_click(
    struct md2_button *button, ui_button_on_click_t on_click, void *user_data);

/**
 * @brief Retrieves the underlying ui_button_base handle.
 *
 * @param button The button.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_button_get_base(
    struct md2_button *button, struct ui_button_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL2_MD2_BUTTON_H */
