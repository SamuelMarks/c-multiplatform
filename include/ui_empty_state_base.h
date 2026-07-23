#ifndef UI_EMPTY_STATE_BASE_H
#define UI_EMPTY_STATE_BASE_H

struct ui_computed;

struct ui_signal;

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents an unstyled empty state container.
 */
struct ui_empty_state_base {
  struct ui_component base;
  struct ui_signal *data_signal;
};

/**
 * @brief Creates a new base empty state component.
 *
 * @param out_state Pointer to output the initialized empty state.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error
ui_empty_state_base_create(struct ui_empty_state_base **out_state);

/**
 * @brief Sets the title text of the empty state.
 *
 * @param state The empty state component.
 * @param text The title text.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_empty_state_base_set_title(struct ui_empty_state_base *state,
                                            const char *text);

/**
 * @brief Sets the description text of the empty state.
 *
 * @param state The empty state component.
 * @param text The description text.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_empty_state_base_set_description(struct ui_empty_state_base *state,
                                    const char *text);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_empty_state_base_bind_data(struct ui_empty_state_base *widget,
                                            struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_EMPTY_STATE_BASE_H */
