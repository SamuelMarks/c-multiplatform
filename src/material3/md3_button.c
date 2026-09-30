/**
 * @file md3_button.c
 * @brief Material 3 Button component wrapping ui_button_base implementation.
 */

/* clang-format off */
#include "material3/md3_button.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

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
ui_error_t md3_button_create(struct ui_engine *engine,
                             enum md3_button_variant variant,
                             struct md3_button **out_button) {
  struct md3_button *btn;
  ui_error_t rc;

  if (!engine || !out_button || (unsigned)variant >= MD3_BUTTON_VARIANT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct md3_button *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_button));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(btn, 0, sizeof(struct md3_button));
  btn->variant = variant;

  rc = ui_button_base_create(&btn->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  *out_button = btn;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Material 3 button and its underlying base.
 *
 * @param button Button to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_button_destroy(struct md3_button *button) {
  ui_error_t rc;

  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_button_base_destroy(button->base);
  C_MULTIPLATFORM_FREE(button);
  return rc;
}

/**
 * @brief Sets static text on the button.
 *
 * @param button The button.
 * @param text The text to set.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_button_set_text(struct md3_button *button, const char *text) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_button_base_set_text(button->base, text);
}

/**
 * @brief Sets disabled state on the button.
 *
 * @param button The button.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_button_set_disabled(struct md3_button *button, int disabled) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_button_base_set_disabled(button->base, disabled);
}

/**
 * @brief Sets click handler on the button.
 *
 * @param button The button.
 * @param on_click Callback function.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_button_set_on_click(struct md3_button *button,
                                   ui_button_on_click_t on_click,
                                   void *user_data) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_button_base_set_on_click(button->base, on_click, user_data);
}

/**
 * @brief Sets indeterminate loading state, displaying spinner and updating ARIA
 * busy.
 *
 * @param button The button.
 * @param is_loading 1 if loading, 0 if normal.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_button_set_loading(struct md3_button *button, int is_loading) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  button->is_loading = is_loading ? 1 : 0;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets leading icon name.
 *
 * @param button The button.
 * @param icon_name Name of icon token, or NULL to clear.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_button_set_icon(struct md3_button *button,
                               const char *icon_name) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!icon_name) {
    button->icon_name[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strncpy_s(button->icon_name, sizeof(button->icon_name), icon_name,
              sizeof(button->icon_name) - 1);
#else
    strncpy(button->icon_name, icon_name, sizeof(button->icon_name) - 1);
    button->icon_name[sizeof(button->icon_name) - 1] = '\0';
#endif
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves underlying ui_button_base.
 *
 * @param button The button.
 * @param out_base Pointer to receive ui_button_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_button_get_base(struct md3_button *button,
                               struct ui_button_base **out_base) {
  if (!button || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = button->base;
  return UI_ERROR_NONE;
}
