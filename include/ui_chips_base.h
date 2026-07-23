#ifndef UI_CHIPS_BASE_H
#define UI_CHIPS_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_control_value_accessor.h"
#include <stddef.h>
/* clang-format on */

struct ui_chips_base;

/**
 * @brief Creates a new chips collection manager.
 *
 * @param out_chips Pointer to receive the allocated chips base.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_chips_base_create(struct ui_chips_base **out_chips,
                                   struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a chips collection manager and frees its tokens.
 *
 * @param chips The chips base to destroy.
 */
void ui_chips_base_destroy(struct ui_chips_base *chips);

/**
 * @brief Adds a token to the collection.
 *
 * @param chips The chips base.
 * @param token The string token to copy and add.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_chips_base_add(struct ui_chips_base *chips, const char *token);

/**
 * @brief Removes a token from the collection by index.
 *
 * @param chips The chips base.
 * @param index The index of the token to remove.
 * @return UI_ERROR_NONE on success, or UI_ERROR_OUT_OF_BOUNDS.
 */
enum ui_error ui_chips_base_remove(struct ui_chips_base *chips, size_t index);

/**
 * @brief Removes the last token from the collection.
 *
 * @param chips The chips base.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if empty.
 */
enum ui_error ui_chips_base_remove_last(struct ui_chips_base *chips);

/**
 * @brief Gets the number of tokens in the collection.
 *
 * @param chips The chips base.
 * @param out_count Pointer to receive the count.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_chips_base_get_count(const struct ui_chips_base *chips,
                                      size_t *out_count);

/**
 * @brief Gets a token string from the collection by index.
 *
 * @param chips The chips base.
 * @param index The index of the token.
 * @param out_token Pointer to receive the string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_OUT_OF_BOUNDS.
 */
enum ui_error ui_chips_base_get_token(const struct ui_chips_base *chips,
                                      size_t index, const char **out_token);

/**
 * @brief Handles focus routing when Backspace is pressed.
 * Hitting `Backspace` on an empty input moves focus to the last chip.
 *
 * @param chips The chips base.
 * @param current_input The current string in the input field.
 * @param out_focus_moved_to_last Pointer to an integer that receives 1 if focus
 * should move, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_chips_base_handle_backspace(struct ui_chips_base *chips,
                                             const char *current_input,
                                             int *out_focus_moved_to_last);

#ifdef __cplusplus
}
#endif

#endif /* UI_CHIPS_BASE_H */
