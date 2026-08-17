/**
 * @file ui_virtual_keyboard.c
 * @brief Implementation of the virtual keyboard manager component.
 */

/* clang-format off */
#include "ui_virtual_keyboard.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct ui_virtual_keyboard
 * @brief Internal state for the virtual keyboard manager.
 */
struct ui_virtual_keyboard {
  float height;
  struct ui_signal *height_signal;
  struct ui_safe_area_manager *safe_area_manager;
  float base_bottom_inset;
};

/* In a real implementation we would subscribe to the safe area manager's signal
   and compute: vk_height = current_bottom_inset - base_bottom_inset.
   For this CDK base, we provide the manual setter and the structural layout. */

ui_error_t ui_virtual_keyboard_create(struct ui_virtual_keyboard **out_vk) {
  struct ui_virtual_keyboard *vk;

  if (!out_vk) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  vk = (struct ui_virtual_keyboard *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_virtual_keyboard));
  if (!vk) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  vk->height = 0.0f;
  vk->height_signal = NULL;
  vk->safe_area_manager = NULL;
  vk->base_bottom_inset = 0.0f;

  *out_vk = vk;
  return UI_ERROR_NONE;
}

ui_error_t ui_virtual_keyboard_destroy(struct ui_virtual_keyboard *vk) {
  if (!vk)
    return UI_ERROR_NONE;
  /* Unsubscribe from safe_area_manager signal here */
  C_MULTIPLATFORM_FREE(vk);
  return UI_ERROR_NONE;
}

ui_error_t ui_virtual_keyboard_bind_safe_area(
    struct ui_virtual_keyboard *vk,
    struct ui_safe_area_manager *safe_area_manager) {
  if (!vk || !safe_area_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  vk->safe_area_manager = safe_area_manager;
  /* Setup signal listener here */
  return UI_ERROR_NONE;
}

ui_error_t ui_virtual_keyboard_set_height(struct ui_virtual_keyboard *vk,
                                          float height_px) {
  if (!vk) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (vk->height != height_px) {
    vk->height = height_px;
    if (vk->height_signal) {
      /* Normally emit the signal here: ui_signal_emit(vk->height_signal,
       * &vk->height); */
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_virtual_keyboard_get_height(const struct ui_virtual_keyboard *vk,
                                          float *out_height) {
  if (!vk || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_height = vk->height;
  return UI_ERROR_NONE;
}

ui_error_t
ui_virtual_keyboard_bind_height_signal(struct ui_virtual_keyboard *vk,
                                       struct ui_signal *height_signal) {
  if (!vk || !height_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  vk->height_signal = height_signal;
  return UI_ERROR_NONE;
}
