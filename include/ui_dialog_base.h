#ifndef UI_DIALOG_BASE_H
#define UI_DIALOG_BASE_H
struct ui_computed;
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
#include "ui_focus_manager.h"
#include "ui_backdrop.h"
#include "ui_event.h"
/* clang-format on */

struct ui_dialog_base;

/**
 * @brief Callback invoked when the dialog is closed (e.g. by backdrop click or
 * escape key).
 *
 * @param dialog The dialog component.
 * @param user_data Opaque user data.
 */
typedef enum ui_error (*ui_dialog_on_close_t)(struct ui_dialog_base *dialog,
                                              void *user_data);

/**
 * @brief Creates a new unstyled dialog base component.
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--dialog-bg`: Background color of the dialog surface.
 * - `--dialog-border`: Border of the dialog.
 * - `--dialog-border-radius`: Border radius.
 * - `--dialog-padding`: Padding inside the dialog.
 * - `--dialog-shadow`: Box shadow/elevation of the dialog.
 *
 * @param out_dialog Pointer to receive the allocated dialog base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_dialog_base_create(struct ui_dialog_base **out_dialog);

/**
 * @brief Destroys a dialog base component.
 *
 * @param dialog The dialog to destroy.
 */
void ui_dialog_base_destroy(struct ui_dialog_base *dialog);

/**
 * @brief Sets the content component that will be displayed inside the dialog.
 *
 * @param dialog The dialog component.
 * @param content The component to set as the inner content of the dialog.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_dialog_base_set_content(struct ui_dialog_base *dialog,
                                         struct ui_component *content);

/**
 * @brief Sets the overlay director to use when mounting the dialog.
 *
 * @param dialog The dialog component.
 * @param director The overlay director.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_dialog_base_set_overlay_director(struct ui_dialog_base *dialog,
                                    struct ui_overlay_director *director);

/**
 * @brief Sets the focus manager to use for trapping focus within the dialog.
 *
 * @param dialog The dialog component.
 * @param focus_manager The focus manager.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_dialog_base_set_focus_manager(struct ui_dialog_base *dialog,
                                 struct ui_focus_manager *focus_manager);

/**
 * @brief Opens or closes the dialog.
 * When opening, it mounts itself via the overlay director and pushes a focus
 * trap. When closing, it unmounts and pops the focus trap.
 *
 * @param dialog The dialog component.
 * @param is_open 1 to open, 0 to close.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_dialog_base_set_open(struct ui_dialog_base *dialog,
                                      int is_open);

/**
 * @brief Checks if the dialog is currently open.
 *
 * @param dialog The dialog component.
 * @return 1 if open, 0 if closed.
 */
enum ui_error ui_dialog_base_is_open(const struct ui_dialog_base *dialog,
                                     int *out_is_open);

/**
 * @brief Sets the callback invoked when the dialog is dismissed.
 *
 * @param dialog The dialog component.
 * @param on_close The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_dialog_base_set_on_close(struct ui_dialog_base *dialog,
                                          ui_dialog_on_close_t on_close,
                                          void *user_data);

/**
 * @brief Processes an incoming input event to handle backdrop clicks and escape
 * keys.
 *
 * @param dialog The dialog component.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_dialog_base_process_event(struct ui_dialog_base *dialog,
                                           const struct ui_event *event,
                                           double timestamp_ms);

/**
 * @brief Gets the underlying wrapper component of the dialog.
 *
 * @param dialog The dialog component.
 * @return The underlying component.
 */
enum ui_error ui_dialog_base_get_component(struct ui_dialog_base *dialog,
                                           struct ui_component **out_component);

/**
 * @brief Binds the open state to a signal.
 *
 * @param widget The widget.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_dialog_base_bind_open(struct ui_dialog_base *widget,
                                       struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the widget is animating.
 *
 * @param widget The widget.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_dialog_base_get_animating_signal(struct ui_dialog_base *widget,
                                    struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DIALOG_BASE_H */
