/**
 * @file md3_button.h
 * @brief Material 3 Button component wrapping ui_button_base.
 */

#ifndef MATERIAL3_MD3_BUTTON_H
#define MATERIAL3_MD3_BUTTON_H

/* clang-format off */
#include "ui_button_base.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_button_variant
 * @brief Material 3 button variants.
 */
enum md3_button_variant {
  MD3_BUTTON_ELEVATED = 0,
  MD3_BUTTON_FILLED,
  MD3_BUTTON_FILLED_TONAL,
  MD3_BUTTON_OUTLINED,
  MD3_BUTTON_TEXT,
  MD3_BUTTON_VARIANT_COUNT
};

/**
 * @struct md3_button
 * @brief Material 3 Button skin wrapping ui_button_base.
 */
struct md3_button {
  struct ui_button_base *base;
  enum md3_button_variant variant;
  int is_loading;
  char icon_name[32];
};

/**
 * @brief Creates a Material 3 button wrapping ui_button_base with designated
 * styling.
 *
 * @param engine Pointer to ui_engine instance.
 * @param variant Visual variant (Elevated, Filled, Filled Tonal, Outlined,
 * Text).
 * @param out_button Pointer to receive newly created button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_button_create(struct ui_engine *engine, enum md3_button_variant variant,
                  struct md3_button **out_button);

/**
 * @brief Destroys a Material 3 button and its underlying base.
 *
 * @param button Button to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_button_destroy(struct md3_button *button);

/**
 * @brief Sets static text on the button.
 *
 * @param button The button.
 * @param text The text to set.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_button_set_text(struct md3_button *button, const char *text);

/**
 * @brief Sets disabled state on the button.
 *
 * @param button The button.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_button_set_disabled(struct md3_button *button, int disabled);

/**
 * @brief Sets click handler on the button.
 *
 * @param button The button.
 * @param on_click Callback function.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_button_set_on_click(
    struct md3_button *button, ui_button_on_click_t on_click, void *user_data);

/**
 * @brief Sets indeterminate loading state, displaying spinner and updating ARIA
 * busy.
 *
 * @param button The button.
 * @param is_loading 1 if loading, 0 if normal.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_button_set_loading(struct md3_button *button, int is_loading);

/**
 * @brief Sets leading icon name.
 *
 * @param button The button.
 * @param icon_name Name of icon token, or NULL to clear.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_button_set_icon(struct md3_button *button, const char *icon_name);

/**
 * @brief Retrieves underlying ui_button_base.
 *
 * @param button The button.
 * @param out_base Pointer to receive ui_button_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_button_get_base(
    struct md3_button *button, struct ui_button_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_BUTTON_H */
