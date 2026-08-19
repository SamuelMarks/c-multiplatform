/**
 * @file ui_sidenav_base.h
 * @brief Sidenav component base declarations.
 *
 * @defgroup SidenavBase Sidenav Base
 * @brief Base implementation for side navigation drawers.
 * @{
 */

#ifndef UI_SIDENAV_BASE_H
#define UI_SIDENAV_BASE_H

/**
 * @brief Opaque handle to a signal.
 */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
#include "ui_event.h"
/* clang-format on */

/**
 * @brief Represents the behavior mode of the sidenav drawer.
 */
enum ui_sidenav_mode {
  UI_SIDENAV_MODE_OVER =
      0,                /**< Drawer floats over content. Requires backdrop. */
  UI_SIDENAV_MODE_PUSH, /**< Drawer pushes main content aside. */
  UI_SIDENAV_MODE_SIDE  /**< Drawer is placed side-by-side with main content. */
};

/**
 * @brief Represents the physical position of the drawer.
 */
enum ui_sidenav_position {
  UI_SIDENAV_POSITION_START = 0, /**< Left side in LTR. */
  UI_SIDENAV_POSITION_END        /**< Right side in LTR. */
};

/**
 * @brief Opaque structure representing the sidenav base.
 */
struct ui_sidenav_base;

/**
 * @brief Callback invoked when the drawer is closed via backdrop or gesture.
 *
 * @param sidenav The sidenav.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
typedef ui_error_t (*ui_sidenav_on_close_t)(struct ui_sidenav_base *sidenav,
                                            void *user_data);

/**
 * @brief Creates a new base sidenav container.
 *
 * @param out_sidenav Pointer to receive the allocated sidenav instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_create(struct ui_sidenav_base **out_sidenav);

/**
 * @brief Destroys a sidenav instance.
 *
 * @param sidenav The sidenav.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_destroy(struct ui_sidenav_base *sidenav);

/**
 * @brief Sets the behavior mode.
 *
 * @param sidenav The sidenav.
 * @param mode The mode (over, push, side).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_mode(struct ui_sidenav_base *sidenav,
                                    enum ui_sidenav_mode mode);

/**
 * @brief Sets the drawer position (start or end).
 *
 * @param sidenav The sidenav.
 * @param position The position.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_position(struct ui_sidenav_base *sidenav,
                                        enum ui_sidenav_position position);

/**
 * @brief Sets the component to render inside the drawer.
 *
 * @param sidenav The sidenav.
 * @param content The drawer content component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_drawer_content(struct ui_sidenav_base *sidenav,
                                              struct ui_component *content);

/**
 * @brief Sets the component to render in the main content area.
 *
 * @param sidenav The sidenav.
 * @param content The main content component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_main_content(struct ui_sidenav_base *sidenav,
                                            struct ui_component *content);

/**
 * @brief Toggles the open state of the drawer.
 *
 * @param sidenav The sidenav.
 * @param is_open 1 to open, 0 to close.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_open(struct ui_sidenav_base *sidenav,
                                    int is_open);

/**
 * @brief Checks if the drawer is currently open.
 *
 * @param sidenav The sidenav.
 * @param out_is_open Pointer to receive the open state (1 if open, 0 if
 * closed).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_is_open(const struct ui_sidenav_base *sidenav,
                                   int *out_is_open);

/**
 * @brief Sets the overlay director used to mount the backdrop in 'over' mode.
 *
 * @param sidenav The sidenav.
 * @param director The overlay director.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_sidenav_base_set_overlay_director(struct ui_sidenav_base *sidenav,
                                     struct ui_overlay_director *director);

/**
 * @brief Sets a callback invoked when the drawer is dismissed (e.g., clicking
 * backdrop).
 *
 * @param sidenav The sidenav.
 * @param on_close The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_on_close(struct ui_sidenav_base *sidenav,
                                        ui_sidenav_on_close_t on_close,
                                        void *user_data);

/**
 * @brief Processes input events to handle backdrop clicks and escape keys.
 *
 * @param sidenav The sidenav.
 * @param event The input event.
 * @param timestamp_ms The timestamp in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_process_event(struct ui_sidenav_base *sidenav,
                                         const struct ui_event *event,
                                         double timestamp_ms);

/**
 * @brief Retrieves the underlying component wrapper.
 *
 * @param sidenav The sidenav.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_get_component(struct ui_sidenav_base *sidenav,
                                         struct ui_component **out_component);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The sidenav widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_bind_active_index(struct ui_sidenav_base *widget,
                                             struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SIDENAV_BASE_H */

/** @} */
