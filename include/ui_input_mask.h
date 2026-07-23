/* clang-format off */
#ifndef UI_INPUT_MASK_H
#define UI_INPUT_MASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_input_base.h"
#include "ui_types.h"

/* clang-format on */

struct ui_input_mask;

/**
 * @brief Creates a new input mask behavior.
 *
 * @param out_mask Pointer to store the created mask.
 * @return enum ui_error
 */
enum ui_error ui_input_mask_create(struct ui_input_mask **out_mask);

/**
 * @brief Destroys the input mask behavior.
 *
 * @param mask The mask behavior.
 * @return enum ui_error
 */
enum ui_error ui_input_mask_destroy(struct ui_input_mask *mask);

/**
 * @brief Binds the mask behavior to an input base component.
 *
 * @param mask The mask behavior.
 * @param input The input base component to bind to.
 * @return enum ui_error
 */
enum ui_error ui_input_mask_bind(struct ui_input_mask *mask,
                                 struct ui_input_base *input);

/**
 * @brief Sets the declarative mask syntax (e.g., "(999) 999-9999").
 * 9 = number, a = alpha, * = alphanumeric.
 *
 * @param mask The mask behavior.
 * @param pattern The mask pattern.
 * @return enum ui_error
 */
enum ui_error ui_input_mask_set_pattern(struct ui_input_mask *mask,
                                        const char *pattern);

/**
 * @brief Gets the raw, unformatted value.
 *
 * @param mask The mask behavior.
 * @param out_raw Pointer to store the raw value string pointer.
 * @return enum ui_error
 */
enum ui_error ui_input_mask_get_raw_value(struct ui_input_mask *mask,
                                          const char **out_raw);

/**
 * @brief Intercepts input changes to format the text and update raw value.
 * This should be hooked up to the input's on_change event.
 *
 * @param mask The mask behavior.
 * @param text The newly input text.
 * @return enum ui_error
 */
enum ui_error ui_input_mask_process_text(struct ui_input_mask *mask,
                                         const char *text);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_INPUT_MASK_H */
