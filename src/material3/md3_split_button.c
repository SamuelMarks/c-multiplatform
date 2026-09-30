/**
 * @file md3_split_button.c
 * @brief Material 3 Expressive Split Button component wrapping
 * ui_split_button_base implementation.
 */

/* clang-format off */
#include "material3/md3_split_button.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/**
 * @brief Creates a Material 3 split button wrapping ui_split_button_base.
 *
 * @param engine Pointer to ui_engine instance.
 * @param variant Visual button variant (Elevated, Filled, Filled Tonal,
 * Outlined, Text).
 * @param out_split_button Pointer to receive newly created split button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_split_button_create(struct ui_engine *engine,
                                   enum md3_button_variant variant,
                                   struct md3_split_button **out_split_button) {
  struct md3_split_button *btn;
  ui_error_t rc;

  if (!engine || !out_split_button ||
      (unsigned)variant >= MD3_BUTTON_VARIANT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct md3_split_button *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_split_button));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(btn, 0, sizeof(struct md3_split_button));
  btn->variant = variant;

  rc = ui_split_button_base_create(&btn->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  *out_split_button = btn;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Material 3 split button and its underlying base.
 *
 * @param split_button Split button to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_split_button_destroy(struct md3_split_button *split_button) {
  ui_error_t rc;

  if (!split_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_split_button_base_destroy(split_button->base);
  C_MULTIPLATFORM_FREE(split_button);
  return rc;
}

/**
 * @brief Retrieves the underlying ui_split_button_base handle.
 *
 * @param split_button The split button.
 * @param out_base Pointer to receive the base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_split_button_get_base(struct md3_split_button *split_button,
                                     struct ui_split_button_base **out_base) {
  if (!split_button || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = split_button->base;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the primary action button component.
 *
 * @param split_button The split button.
 * @param out_main_btn Pointer to receive the primary action button base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
md3_split_button_get_main_button(struct md3_split_button *split_button,
                                 struct ui_button_base **out_main_btn) {
  if (!split_button || !out_main_btn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_split_button_base_get_main_button(split_button->base, out_main_btn);
}

/**
 * @brief Gets the trailing dropdown trigger button component.
 *
 * @param split_button The split button.
 * @param out_trigger_btn Pointer to receive the trigger button base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
md3_split_button_get_trigger_button(struct md3_split_button *split_button,
                                    struct ui_button_base **out_trigger_btn) {
  if (!split_button || !out_trigger_btn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_split_button_base_get_trigger_button(split_button->base,
                                                 out_trigger_btn);
}

/**
 * @brief Sets the disabled state of the split button.
 *
 * @param split_button The split button.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_split_button_set_disabled(struct md3_split_button *split_button,
                                         int disabled) {
  if (!split_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_split_button_base_set_disabled(split_button->base, disabled);
}

/**
 * @brief Sets text on the primary action button.
 *
 * @param split_button The split button.
 * @param text The text to set.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_split_button_set_text(struct md3_split_button *split_button,
                                     const char *text) {
  struct ui_button_base *main_btn;
  ui_error_t rc;

  if (!split_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_split_button_base_get_main_button(split_button->base, &main_btn);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_button_base_set_text(main_btn, text);
}

/**
 * @brief Sets click handler on the primary action button.
 *
 * @param split_button The split button.
 * @param on_click Callback function.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_split_button_set_on_click(struct md3_split_button *split_button,
                                         ui_button_on_click_t on_click,
                                         void *user_data) {
  struct ui_button_base *main_btn;
  ui_error_t rc;

  if (!split_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_split_button_base_get_main_button(split_button->base, &main_btn);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_button_base_set_on_click(main_btn, on_click, user_data);
}

/**
 * @brief Sets click handler on the dropdown trigger button.
 *
 * @param split_button The split button.
 * @param on_trigger Callback function for dropdown click.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t
md3_split_button_set_on_trigger_click(struct md3_split_button *split_button,
                                      ui_button_on_click_t on_trigger,
                                      void *user_data) {
  struct ui_button_base *trig_btn;
  ui_error_t rc;

  if (!split_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_split_button_base_get_trigger_button(split_button->base, &trig_btn);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_button_base_set_on_click(trig_btn, on_trigger, user_data);
}
