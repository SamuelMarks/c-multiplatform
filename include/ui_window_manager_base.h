/**
 * @file ui_window_manager_base.h
 * @brief Window manager base component for MDI interface elements.
 */
#ifndef UI_WINDOW_MANAGER_BASE_H
#define UI_WINDOW_MANAGER_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_window_manager_base;

/**
 * @brief Creates a new unstyled window manager base component (MDI).
 *
 * @param out_window_manager Pointer to receive the allocated component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_window_manager_base_create(
    struct ui_window_manager_base **out_window_manager);

/**
 * @brief Destroys a window manager base component.
 *
 * @param window_manager The component to destroy.
 */
ui_error_t
ui_window_manager_base_destroy(struct ui_window_manager_base *window_manager);

/**
 * @brief Gets the underlying component instance.
 *
 * @param window_manager The window manager component.
 * @param out_component Pointer to receive the underlying component.
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t ui_window_manager_base_get_component(
    struct ui_window_manager_base *window_manager,
    struct ui_component **out_component);

/**
 * @brief Brings a managed window to the front (Z-order stacking).
 *
 * @param window_manager The window manager component.
 * @param window_id Identifier for the internal window.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_window_manager_base_bring_to_front(
    struct ui_window_manager_base *window_manager, int window_id);

/**
 * @brief Simulates dragging an internal window.
 *
 * @param window_manager The window manager component.
 * @param window_id Identifier for the internal window.
 * @param delta_x X-axis drag delta.
 * @param delta_y Y-axis drag delta.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_window_manager_base_drag(struct ui_window_manager_base *window_manager,
                            int window_id, float delta_x, float delta_y);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_window_manager_base_bind_data(struct ui_window_manager_base *widget,
                                 struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WINDOW_MANAGER_BASE_H */
