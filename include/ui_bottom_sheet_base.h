/**
 * @file ui_bottom_sheet_base.h
 * @brief Bottom sheet base component for modal interactions.
 */

#ifndef UI_BOTTOM_SHEET_BASE_H
#define UI_BOTTOM_SHEET_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
#include "ui_event.h"
#include "ui_spring.h"
#include "ui_signal.h"
#include "ui_computed.h"
/* clang-format on */

/**
 * @brief Opaque structure representing the base bottom sheet.
 */
struct ui_bottom_sheet_base;

/**
 * @brief Callback invoked when the bottom sheet is closed via swipe down or
 * backdrop click.
 *
 * @param sheet The bottom sheet.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
typedef ui_error_t (*ui_bottom_sheet_on_close_t)(
    struct ui_bottom_sheet_base *sheet, void *user_data);

/**
 * @brief Creates a new base bottom sheet container.
 *
 * @param out_sheet Pointer to receive the allocated bottom sheet instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_sheet_base_create(struct ui_bottom_sheet_base **out_sheet);

/**
 * @brief Destroys a bottom sheet instance.
 *
 * @param sheet The bottom sheet.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_sheet_base_destroy(struct ui_bottom_sheet_base *sheet);

/**
 * @brief Sets the content component to render inside the bottom sheet.
 *
 * @param sheet The bottom sheet.
 * @param content The component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_sheet_base_set_content(struct ui_bottom_sheet_base *sheet,
                                            struct ui_component *content);

/**
 * @brief Toggles the open state of the bottom sheet.
 *
 * @param sheet The bottom sheet.
 * @param is_open 1 to open, 0 to close.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_sheet_base_set_open(struct ui_bottom_sheet_base *sheet,
                                         int is_open);

/**
 * @brief Checks if the bottom sheet is currently open.
 *
 * @param sheet The bottom sheet.
 * @param out_is_open Pointer to receive the open state (1 if open, 0 if
 * closed).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_sheet_base_is_open(const struct ui_bottom_sheet_base *sheet,
                             int *out_is_open);

/**
 * @brief Sets the overlay director used to mount the bottom sheet.
 *
 * @param sheet The bottom sheet.
 * @param director The overlay director.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_sheet_base_set_overlay_director(struct ui_bottom_sheet_base *sheet,
                                          struct ui_overlay_director *director);

/**
 * @brief Sets a callback invoked when the sheet is dismissed.
 *
 * @param sheet The bottom sheet.
 * @param on_close The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_sheet_base_set_on_close(struct ui_bottom_sheet_base *sheet,
                                  ui_bottom_sheet_on_close_t on_close,
                                  void *user_data);

/**
 * @brief Processes input events to handle gestures (swipe down) and backdrop
 * clicks.
 *
 * @param sheet The bottom sheet.
 * @param event The input event.
 * @param timestamp_ms The current timestamp.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_sheet_base_process_event(struct ui_bottom_sheet_base *sheet,
                                   const struct ui_event *event,
                                   double timestamp_ms);

/**
 * @brief Updates the internal gesture recognizer logic for time-based
 * thresholds.
 *
 * @param sheet The bottom sheet.
 * @param timestamp_ms The current timestamp.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_sheet_base_update(struct ui_bottom_sheet_base *sheet,
                                       double timestamp_ms);

/**
 * @brief Configures the interruptible, physics-based spring for gesture
 * dismissal.
 *
 * @param sheet The bottom sheet.
 * @param config The spring configuration.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_sheet_base_set_spring_config(struct ui_bottom_sheet_base *sheet,
                                       const struct ui_spring_config *config);

/**
 * @brief Retrieves the underlying component wrapper.
 *
 * @param sheet The bottom sheet.
 * @param out_component Pointer to receive the underlying component wrapper.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_sheet_base_get_component(struct ui_bottom_sheet_base *sheet,
                                   struct ui_component **out_component);

/**
 * @brief Binds the bottom sheet's open state to a signal.
 *
 * @param sheet The bottom sheet.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_sheet_base_bind_open(struct ui_bottom_sheet_base *sheet,
                                          struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the bottom sheet is
 * animating.
 *
 * @param sheet The bottom sheet.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_sheet_base_get_animating_signal(struct ui_bottom_sheet_base *sheet,
                                          struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BOTTOM_SHEET_BASE_H */
