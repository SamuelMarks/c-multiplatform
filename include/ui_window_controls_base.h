/**
 * @file ui_window_controls_base.h
 * @brief Window controls base component for client-side window decorations.
 */
#ifndef UI_WINDOW_CONTROLS_BASE_H
#define UI_WINDOW_CONTROLS_BASE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_component.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Window state definitions for OS integration.
 */
enum ui_window_state {
  UI_WINDOW_STATE_NORMAL = 0,
  UI_WINDOW_STATE_MAXIMIZED = 1,
  UI_WINDOW_STATE_MINIMIZED = 2,
  UI_WINDOW_STATE_FULLSCREEN = 3
};

/**
 * @brief Hit-test regions to pass back to the OS window manager.
 */
enum ui_window_control_hit_region {
  UI_WINDOW_CONTROL_HIT_REGION_NONE = 0,
  UI_WINDOW_CONTROL_HIT_REGION_CLIENT = 1,   /**< Normal interactive app area */
  UI_WINDOW_CONTROL_HIT_REGION_DRAG = 2,     /**< Draggable titlebar area */
  UI_WINDOW_CONTROL_HIT_REGION_MINIMIZE = 3, /**< Minimize button */
  UI_WINDOW_CONTROL_HIT_REGION_MAXIMIZE = 4, /**< Maximize/Restore button */
  UI_WINDOW_CONTROL_HIT_REGION_CLOSE = 5,    /**< Close button */
  UI_WINDOW_CONTROL_HIT_REGION_SNAP_MENU =
      6 /**< Hover area for Windows 11 snap layout */
};

/**
 * @brief Opaque handle for the window controls component.
 */
struct ui_window_controls_base;

/**
 * @brief Creates a window controls base component.
 *
 * @param arena The memory arena to use for allocation.
 * @param out_controls Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_window_controls_base_create(struct ui_arena *arena,
                               struct ui_window_controls_base **out_controls);

/**
 * @brief Destroys a window controls base component.
 *
 * @param controls The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_window_controls_base_destroy(struct ui_window_controls_base *controls);

/**
 * @brief Sets the current window state (e.g., maximized vs. restored).
 *
 * @param controls The component.
 * @param state The new state.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_window_controls_base_set_state(struct ui_window_controls_base *controls,
                                  enum ui_window_state state);

/**
 * @brief Retrieves the signal for window state changes.
 * The payload contains the int value of the ui_window_state.
 *
 * @param controls The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_window_controls_base_get_state_signal(
    struct ui_window_controls_base *controls, ui_signal_t **out_signal);

/**
 * @brief Performs a hit-test to determine the OS-level region at a given point.
 * This should be used to respond to WM_NCHITTEST (Win32) or equivalent OS
 * events.
 *
 * @param controls The component.
 * @param x The X coordinate.
 * @param y The Y coordinate.
 * @param out_region Pointer to receive the hit region.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_window_controls_base_hit_test(struct ui_window_controls_base *controls,
                                 int x, int y,
                                 enum ui_window_control_hit_region *out_region);

/**
 * @brief Sets the bounding box for a specific control region.
 *
 * @param controls The component.
 * @param region The region type to define.
 * @param x The X coordinate of the top-left corner.
 * @param y The Y coordinate of the top-left corner.
 * @param width The width of the region.
 * @param height The height of the region.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_window_controls_base_set_region_rect(
    struct ui_window_controls_base *controls,
    enum ui_window_control_hit_region region, int x, int y, int width,
    int height);

/**
 * @brief Triggers a hover intent for a region (e.g., showing snap layouts on
 * Windows 11).
 *
 * @param controls The component.
 * @param region The region being hovered.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_window_controls_base_trigger_hover_intent(
    struct ui_window_controls_base *controls,
    enum ui_window_control_hit_region region);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WINDOW_CONTROLS_BASE_H */
