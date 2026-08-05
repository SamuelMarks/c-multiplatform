#ifndef UI_ALERT_BASE_H
#define UI_ALERT_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_signal.h"
#include "ui_computed.h"
/* clang-format on */

struct ui_alert_base;

/**
 * @brief Callback invoked when the alert is dismissed.
 *
 * @param alert The alert component being dismissed.
 * @param user_data Opaque user data provided during setup.
 */
typedef ui_error_t (*ui_alert_on_dismiss_t)(struct ui_alert_base *alert,
                                            void *user_data);

/**
 * @brief Alert roles defining the semantic meaning of the alert.
 */
enum ui_alert_role {
  UI_ALERT_ROLE_ALERT, /**< Immediate attention required (role="alert"). */
  UI_ALERT_ROLE_STATUS /**< Advisory information (role="status"). */
};

/**
 * @brief Creates a new unstyled alert base component.
 *
 * @param out_alert Pointer to receive the allocated alert base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_alert_base_create(struct ui_alert_base **out_alert);

/**
 * @brief Destroys an alert base component.
 *
 * @param alert The alert to destroy.
 */
ui_error_t ui_alert_base_destroy(struct ui_alert_base *alert);

/**
 * @brief Gets the underlying component for the alert.
 *
 * @param alert The alert.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_get_component(struct ui_alert_base *alert,
                                       struct ui_component **out_component);

/**
 * @brief Sets the semantic role of the alert.
 *
 * @param alert The alert.
 * @param role The role to assign (alert or status).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_role(struct ui_alert_base *alert,
                                  enum ui_alert_role role);

/**
 * @brief Gets the current semantic role of the alert.
 *
 * @param alert The alert.
 * @param out_role Pointer to receive the current role.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_get_role(struct ui_alert_base *alert,
                                  enum ui_alert_role *out_role);

/**
 * @brief Sets whether the alert is dismissible.
 *
 * If set to true, typically a dismiss button is presented, and dismiss logic
 * applies.
 *
 * @param alert The alert.
 * @param dismissible True if dismissible.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_dismissible(struct ui_alert_base *alert,
                                         int dismissible);

/**
 * @brief Checks if the alert is dismissible.
 *
 * @param alert The alert.
 * @param out_dismissible Pointer to receive the dismissible state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_is_dismissible(struct ui_alert_base *alert,
                                        int *out_dismissible);

/**
 * @brief Sets the callback invoked when the alert is dismissed.
 *
 * @param alert The alert.
 * @param on_dismiss The callback function.
 * @param user_data Opaque data passed to the callback.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_on_dismiss(struct ui_alert_base *alert,
                                        ui_alert_on_dismiss_t on_dismiss,
                                        void *user_data);

/**
 * @brief Dismisses the alert, updating state, firing callbacks, and managing
 * focus.
 *
 * @param alert The alert.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_dismiss(struct ui_alert_base *alert);

/**
 * @brief Sets the component to return focus to when this alert is dismissed.
 *
 * @param alert The alert.
 * @param focus_return The component to focus upon dismissal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_focus_return(struct ui_alert_base *alert,
                                          struct ui_component *focus_return);

/**
 * @brief Sets the open state of the alert.
 *
 * @param alert The alert.
 * @param is_open 1 to open, 0 to close (dismiss).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_open(struct ui_alert_base *alert, int is_open);

/**
 * @brief Checks if the alert is currently open.
 *
 * @param alert The alert.
 * @param out_is_open Pointer to receive the open state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_is_open(const struct ui_alert_base *alert,
                                 int *out_is_open);

/**
 * @brief Binds the alert's open state to a signal.
 *
 * @param alert The alert.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_bind_open(struct ui_alert_base *alert,
                                   struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the alert is animating.
 *
 * @param alert The alert.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_alert_base_get_animating_signal(struct ui_alert_base *alert,
                                   struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ALERT_BASE_H */
