/**
 * @file ui_virtual_keyboard.h
 * @brief Virtual keyboard manager component.
 */
#ifndef UI_VIRTUAL_KEYBOARD_H
#define UI_VIRTUAL_KEYBOARD_H

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_safe_area_manager.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing the virtual keyboard manager.
 */
struct ui_virtual_keyboard;

/**
 * @brief Creates a new virtual keyboard manager.
 *
 * @param out_vk Pointer to receive the allocated manager.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_virtual_keyboard_create(struct ui_virtual_keyboard **out_vk);

/**
 * @brief Destroys a virtual keyboard manager.
 *
 * @param vk The manager to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_virtual_keyboard_destroy(struct ui_virtual_keyboard *vk);

/**
 * @brief Integrates the virtual keyboard manager with the safe area manager.
 * It will listen to safe area changes to detect keyboard
 * appearance/disappearance on platforms where the keyboard modifies the bottom
 * inset.
 *
 * @param vk The manager.
 * @param safe_area_manager The system safe area manager.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_virtual_keyboard_bind_safe_area(
    struct ui_virtual_keyboard *vk,
    struct ui_safe_area_manager *safe_area_manager);

/**
 * @brief Explicitly sets the virtual keyboard height (inset).
 * This is useful for platforms where keyboard events are distinct from safe
 * area insets.
 *
 * @param vk The manager.
 * @param height_px The height of the virtual keyboard in pixels.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_virtual_keyboard_set_height(struct ui_virtual_keyboard *vk,
                                          float height_px);

/**
 * @brief Gets the current virtual keyboard height.
 *
 * @param vk The manager.
 * @param out_height Pointer to store the height.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_virtual_keyboard_get_height(const struct ui_virtual_keyboard *vk,
                                          float *out_height);

/**
 * @brief Binds a signal that will receive the current keyboard height (float).
 *
 * @param vk The manager.
 * @param height_signal The signal to update.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_virtual_keyboard_bind_height_signal(struct ui_virtual_keyboard *vk,
                                       struct ui_signal *height_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIRTUAL_KEYBOARD_H */
