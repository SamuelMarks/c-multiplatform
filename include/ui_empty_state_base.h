/**
 * @file ui_empty_state_base.h
 * @brief Base logic and state management for empty state components.
 *
 * This header defines the structure and functions for creating and manipulating
 * an unstyled empty state component, typically used to display placeholders
 * when no data is available.
 */

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
  struct ui_component base; /**< The base UI component structure. */
  struct ui_signal
      *data_signal; /**< Signal to observe for data availability. */
};

/**
 * @brief Creates a new base empty state component.
 *
 * @param out_state Pointer to output the initialized empty state structure.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_empty_state_base_create(struct ui_empty_state_base **out_state);

/**
 * @brief Sets the title text of the empty state.
 *
 * @param state Pointer to the empty state component.
 * @param text The title text to set.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_empty_state_base_set_title(struct ui_empty_state_base *state,
                                         const char *text);

/**
 * @brief Sets the description text of the empty state.
 *
 * @param state Pointer to the empty state component.
 * @param text The description text to set.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_empty_state_base_set_description(struct ui_empty_state_base *state,
                                    const char *text);

/**
 * @brief Binds a data signal to the empty state.
 *
 * @param widget Pointer to the empty state component.
 * @param signal Pointer to the signal to bind to.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_empty_state_base_bind_data(struct ui_empty_state_base *widget,
                                         struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_EMPTY_STATE_BASE_H */
