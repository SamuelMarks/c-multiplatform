/**
 * @file ui_disclosure_base.h
 * @brief Disclosure (accordion) base component for collapsible content.
 */

#ifndef UI_DISCLOSURE_BASE_H
#define UI_DISCLOSURE_BASE_H

/** @brief Forward declaration of ui_computed. */
struct ui_computed;

/** @brief Forward declaration of ui_signal. */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_gesture.h"
#include "ui_event.h"
/* clang-format on */

/** @brief Opaque handle representing a disclosure base component. */
struct ui_disclosure_base;

/**
 * @brief Callback invoked when the disclosure state changes.
 *
 * @param disclosure The disclosure component.
 * @param is_expanded The new expanded state (1 if expanded, 0 if collapsed).
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
typedef ui_error_t (*ui_disclosure_on_toggle_t)(
    struct ui_disclosure_base *disclosure, int is_expanded, void *user_data);

/**
 * @brief Creates a new unstyled disclosure (accordion) base component.
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--disclosure-bg`: Background color of the disclosure container.
 * - `--disclosure-border`: Border.
 * - `--disclosure-padding`: Padding inside the header/summary.
 * - `--disclosure-color`: Text color of the header.
 * - `--disclosure-icon-color`: Color of the expand/collapse icon.
 * - `--disclosure-content-padding`: Padding for the collapsible content area.
 * - `--disclosure-content-bg`: Background of the content area.
 *
 * @param out_disclosure Pointer to receive the allocated disclosure base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_disclosure_base_create(struct ui_disclosure_base **out_disclosure);

/**
 * @brief Destroys a disclosure base component.
 *
 * @param disclosure The disclosure to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_disclosure_base_destroy(struct ui_disclosure_base *disclosure);

/**
 * @brief Sets the disabled state of the disclosure component.
 * Updates ARIA attributes and prevents state changes.
 *
 * @param disclosure The disclosure component.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_disclosure_base_set_disabled(struct ui_disclosure_base *disclosure,
                                int disabled);

/**
 * @brief Sets the expanded state of the disclosure.
 *
 * @param disclosure The disclosure component.
 * @param is_expanded 1 to expand, 0 to collapse.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_disclosure_base_set_expanded(struct ui_disclosure_base *disclosure,
                                int is_expanded);

/**
 * @brief Checks if the disclosure is currently expanded.
 *
 * @param disclosure The disclosure component.
 * @param out_is_expanded Pointer to receive 1 if expanded, 0 if collapsed.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_disclosure_base_is_expanded(const struct ui_disclosure_base *disclosure,
                               int *out_is_expanded);

/**
 * @brief Sets the callback invoked when the disclosure state changes.
 *
 * @param disclosure The disclosure component.
 * @param on_toggle The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_disclosure_base_set_on_toggle(struct ui_disclosure_base *disclosure,
                                 ui_disclosure_on_toggle_t on_toggle,
                                 void *user_data);

/**
 * @brief Processes an incoming input event to handle tap/click on the trigger.
 *
 * @param disclosure The disclosure component.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_disclosure_base_process_event(struct ui_disclosure_base *disclosure,
                                 const struct ui_event *event,
                                 double timestamp_ms);

/**
 * @brief Gets the underlying wrapper component of the disclosure.
 *
 * @param disclosure The disclosure component.
 * @param out_component Output pointer for the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_disclosure_base_get_component(struct ui_disclosure_base *disclosure,
                                 struct ui_component **out_component);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_disclosure_base_bind_data(struct ui_disclosure_base *widget,
                                        struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DISCLOSURE_BASE_H */
