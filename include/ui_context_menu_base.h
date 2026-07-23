#ifndef UI_CONTEXT_MENU_BASE_H
#define UI_CONTEXT_MENU_BASE_H
struct ui_computed;
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include "ui_menu_base.h"
#include "ui_overlay_director.h"
/* clang-format on */

struct ui_context_menu_base;

/**
 * @brief Creates a new context menu base manager.
 *
 * @param out_menu Pointer to receive the allocated context menu base.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_context_menu_base_create(struct ui_context_menu_base **out_menu);

/**
 * @brief Destroys a context menu component.
 *
 * @param menu The context menu component to destroy.
 */
enum ui_error ui_context_menu_base_destroy(struct ui_context_menu_base *menu);

/**
 * @brief Gets the underlying ui_menu_base to allow adding items.
 *
 * @param menu The context menu component.
 * @param out_menu Pointer to receive the underlying menu component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_context_menu_base_get_menu(struct ui_context_menu_base *menu,
                                            struct ui_menu_base **out_menu);

/**
 * @brief Opens the context menu at specific screen coordinates, clamping it to
 * the viewport bounds.
 *
 * @param menu The context menu component.
 * @param director The overlay director to mount the menu into.
 * @param target_x The target X screen coordinate (e.g. from mouse click).
 * @param target_y The target Y screen coordinate.
 * @param menu_width The expected width of the menu.
 * @param menu_height The expected height of the menu.
 * @param viewport_width The maximum width of the viewport.
 * @param viewport_height The maximum height of the viewport.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_context_menu_base_open_at(struct ui_context_menu_base *menu,
                                           struct ui_overlay_director *director,
                                           int target_x, int target_y,
                                           int menu_width, int menu_height,
                                           int viewport_width,
                                           int viewport_height);

/**
 * @brief Binds the open state to a signal.
 *
 * @param widget The widget.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_context_menu_base_bind_open(struct ui_context_menu_base *widget,
                               struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the widget is animating.
 *
 * @param widget The widget.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_context_menu_base_get_animating_signal(struct ui_context_menu_base *widget,
                                          struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif

#endif /* UI_CONTEXT_MENU_BASE_H */
