/**
 * @file md3_icon_button.h
 * @brief Material 3 Icon Button component wrapping ui_button_base.
 */

#ifndef MATERIAL3_MD3_ICON_BUTTON_H
#define MATERIAL3_MD3_ICON_BUTTON_H

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
 * @enum md3_icon_button_variant
 * @brief Material 3 Icon Button visual variants.
 */
enum md3_icon_button_variant {
  MD3_ICON_BUTTON_STANDARD = 0,
  MD3_ICON_BUTTON_FILLED,
  MD3_ICON_BUTTON_FILLED_TONAL,
  MD3_ICON_BUTTON_OUTLINED,
  MD3_ICON_BUTTON_VARIANT_COUNT
};

/**
 * @struct md3_icon_button
 * @brief Material 3 Icon Button skin wrapping ui_button_base.
 */
struct md3_icon_button {
  struct ui_button_base *base;
  enum md3_icon_button_variant variant;
  int is_toggleable;
  int is_selected;
  char icon_name[32];
  char selected_icon_name[32];
};

/**
 * @brief Creates a Material 3 Icon Button component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param variant Visual icon button variant (Standard, Filled, Filled Tonal,
 * Outlined).
 * @param icon_name Initial icon name.
 * @param out_button Pointer to receive newly created icon button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_icon_button_create(
    struct ui_engine *engine, enum md3_icon_button_variant variant,
    const char *icon_name, struct md3_icon_button **out_button);

/**
 * @brief Destroys a Material 3 Icon Button.
 *
 * @param button Icon button to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_button_destroy(struct md3_icon_button *button);

/**
 * @brief Configures toggleable state and selected icon for toggle icon button.
 *
 * @param button The icon button.
 * @param toggleable 1 if button is toggleable, 0 for standard trigger button.
 * @param selected_icon_name Icon name when selected (optional).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_button_set_toggleable(struct md3_icon_button *button, int toggleable,
                               const char *selected_icon_name);

/**
 * @brief Sets selected/pressed state for a toggle icon button, syncing
 * aria-pressed.
 *
 * @param button The icon button.
 * @param selected 1 if selected, 0 otherwise.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_button_set_selected(struct md3_icon_button *button, int selected);

/**
 * @brief Queries whether the icon button is currently selected.
 *
 * @param button The icon button.
 * @param out_selected Pointer to receive 1 if selected, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_icon_button_is_selected(
    const struct md3_icon_button *button, int *out_selected);

/**
 * @brief Sets disabled state on the icon button.
 *
 * @param button The icon button.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_button_set_disabled(struct md3_icon_button *button, int disabled);

/**
 * @brief Sets click handler on the icon button.
 *
 * @param button The icon button.
 * @param on_click Callback function.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_button_set_on_click(struct md3_icon_button *button,
                             ui_button_on_click_t on_click, void *user_data);

/**
 * @brief Sets the primary icon name for the button.
 *
 * @param button The icon button.
 * @param icon_name Icon identifier.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_icon_button_set_icon(struct md3_icon_button *button, const char *icon_name);

/**
 * @brief Retrieves the underlying ui_button_base handle.
 *
 * @param button The icon button.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_icon_button_get_base(
    struct md3_icon_button *button, struct ui_button_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_ICON_BUTTON_H */
