/**
 * @file ui_clipboard.h
 * @brief System clipboard abstraction.
 */

#ifndef UI_CLIPBOARD_H
#define UI_CLIPBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Sets the clipboard text using OS-specific APIs, falling back to a
 * process-local buffer if unavailable.
 *
 * @param text The null-terminated text to copy to the clipboard.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_clipboard_set_text(const char *text);

/**
 * @brief Retrieves the clipboard text.
 *
 * @param out_text Pointer to receive the allocated text string. The caller MUST
 * free it using ui_clipboard_free_text.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_clipboard_get_text(char **out_text);

/**
 * @brief Frees text allocated by ui_clipboard_get_text.
 *
 * @param text The text to free.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_clipboard_free_text(char *text);

/**
 * @brief Clears the fallback memory clipboard, if any was allocated.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_clipboard_cleanup(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CLIPBOARD_H */
