/**
 * @file ui_side_sheet_base.h
 * @brief Base component for sliding side sheets and drawers.
 */

/* clang-format off */
#ifndef UI_SIDE_SHEET_BASE_H
#define UI_SIDE_SHEET_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
#include "ui_event.h"
#include "ui_spring.h"
#include "ui_signal.h"
#include "ui_computed.h"

/* clang-format on */

/**
 * @brief Represents the side sheet's anchored edge.
 */
enum ui_side_sheet_edge {
  UI_SIDE_SHEET_EDGE_LEFT = 0, /**< Anchored to the left edge */
  UI_SIDE_SHEET_EDGE_RIGHT = 1 /**< Anchored to the right edge */
};

/**
 * @brief Represents the behavior when opened.
 */
enum ui_side_sheet_mode {
  UI_SIDE_SHEET_MODE_SLIDE_OVER = 0, /**< Overlays content, with backdrop */
  UI_SIDE_SHEET_MODE_PUSH = 1        /**< Pushes content, no backdrop */
};

/**
 * @brief Opaque structure representing the base side sheet.
 */
struct ui_side_sheet_base;

/**
 * @brief Callback invoked when the side sheet is closed via backdrop click or
 * dismiss gesture.
 */
typedef ui_error_t (*ui_side_sheet_on_close_t)(struct ui_side_sheet_base *sheet,
                                               void *user_data);

/**
 * @brief Creates a new base side sheet container.
 *
 * @param out_sheet Pointer to receive the allocated side sheet instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_side_sheet_base_create(struct ui_side_sheet_base **out_sheet);

/**
 * @brief Destroys a side sheet instance.
 *
 * @param sheet The side sheet.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_side_sheet_base_destroy(struct ui_side_sheet_base *sheet);

/**
 * @brief Sets the content component to render inside the side sheet.
 *
 * @param sheet The side sheet.
 * @param content The component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_side_sheet_base_set_content(struct ui_side_sheet_base *sheet,
                                          struct ui_component *content);

/**
 * @brief Sets the edge (left or right) the sheet is anchored to.
 *
 * @param sheet The side sheet.
 * @param edge The edge.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_side_sheet_base_set_edge(struct ui_side_sheet_base *sheet,
                                       enum ui_side_sheet_edge edge);

/**
 * @brief Sets the interaction mode (slide over or push).
 *
 * @param sheet The side sheet.
 * @param mode The mode.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_side_sheet_base_set_mode(struct ui_side_sheet_base *sheet,
                                       enum ui_side_sheet_mode mode);

/**
 * @brief Toggles the open state of the side sheet.
 *
 * @param sheet The side sheet.
 * @param is_open 1 to open, 0 to close.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_side_sheet_base_set_open(struct ui_side_sheet_base *sheet,
                                       int is_open);

/**
 * @brief Checks if the side sheet is currently open.
 *
 * @param sheet The side sheet.
 * @param out_is_open Pointer to store result.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_side_sheet_base_is_open(const struct ui_side_sheet_base *sheet,
                                      int *out_is_open);

/**
 * @brief Sets the overlay director used to mount the side sheet.
 * Only applies in slide-over mode.
 *
 * @param sheet The side sheet.
 * @param director The overlay director.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_side_sheet_base_set_overlay_director(struct ui_side_sheet_base *sheet,
                                        struct ui_overlay_director *director);

/**
 * @brief Sets a callback invoked when the sheet is dismissed.
 *
 * @param sheet The side sheet.
 * @param on_close The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_side_sheet_base_set_on_close(struct ui_side_sheet_base *sheet,
                                           ui_side_sheet_on_close_t on_close,
                                           void *user_data);

/**
 * @brief Processes input events to handle backdrop clicks and escape key
 * dismiss.
 *
 * @param sheet The side sheet.
 * @param event The input event.
 * @param timestamp_ms The current timestamp.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_side_sheet_base_process_event(struct ui_side_sheet_base *sheet,
                                            const struct ui_event *event,
                                            double timestamp_ms);

/**
 * @brief Retrieves the underlying component wrapper.
 *
 * @param sheet The side sheet.
 * @param out_component Pointer to receive the underlying component.
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t
ui_side_sheet_base_get_component(struct ui_side_sheet_base *sheet,
                                 struct ui_component **out_component);

/**
 * @brief Binds the side sheet's open state to a signal.
 *
 * @param sheet The side sheet.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_side_sheet_base_bind_open(struct ui_side_sheet_base *sheet,
                                        struct ui_signal *open_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SIDE_SHEET_BASE_H */
