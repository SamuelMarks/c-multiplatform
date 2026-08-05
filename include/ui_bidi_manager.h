#ifndef UI_BIDI_MANAGER_H
#define UI_BIDI_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
/* clang-format on */

/**
 * @brief Represents the bidirectional text direction.
 */
enum ui_bidi_direction {
  UI_BIDI_DIR_LTR = 0, /**< Left-To-Right (Default) */
  UI_BIDI_DIR_RTL = 1  /**< Right-To-Left */
};

/**
 * @brief Sets the global bidirectional text direction.
 *
 * @param direction The new global direction (LTR or RTL).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bidi_set_direction(enum ui_bidi_direction direction);

/**
 * @brief Gets the current global bidirectional text direction.
 *
 * @return The current global direction.
 */
ui_error_t ui_bidi_get_direction(enum ui_bidi_direction *out_dir);

/**
 * @brief Normalizes a horizontal key code based on the current global
 * direction. In RTL mode, UI_KEY_LEFT becomes UI_KEY_RIGHT, and vice versa.
 *
 * @param key The input key code.
 * @return The normalized key code.
 */
ui_error_t ui_bidi_normalize_horizontal_key(enum ui_key_code key,
                                            enum ui_key_code *out_key);

#ifdef __cplusplus
}
#endif

#endif /* UI_BIDI_MANAGER_H */
