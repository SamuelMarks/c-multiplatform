/**
 * @file ui_scroll_dispatcher.h
 */
#ifndef UI_SCROLL_DISPATCHER_H
#define UI_SCROLL_DISPATCHER_H

/**
 * \file ui_scroll_dispatcher.h
 * \brief UI Scroll Dispatcher component.
 *
 * This file contains definitions for a central dispatcher of scroll events.
 * It allows multiple UI components to react to scrolling regardless of the
 * scrolling container that triggered it.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include <stddef.h>
/* clang-format on */

/** \brief Forward declaration of ui_layout_observer */
struct ui_layout_observer;
/** \brief Opaque handle representing a scroll dispatcher */
struct ui_scroll_dispatcher;

/**
 * @brief Represents information about a scroll event.
 */
struct ui_scroll_info {
  float scroll_x; /**< The current horizontal scroll position */
  float scroll_y; /**< The current vertical scroll position */
  float delta_x;  /**< The horizontal scroll delta from the previous position */
  float delta_y;  /**< The vertical scroll delta from the previous position */
  int source_id;  /**< Identifier for the scroll container that triggered the
                     event */
};

/**
 * @brief Callback invoked when a scroll event occurs.
 *
 * @param dispatcher The scroll dispatcher emitting the event.
 * @param info Information about the scroll event.
 * @param user_data Opaque user data provided during registration.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_scroll_dispatcher_cb_t)(
    struct ui_scroll_dispatcher *dispatcher, const struct ui_scroll_info *info,
    void *user_data);

/**
 * @brief Creates a new scroll dispatcher instance.
 *
 * @param out_dispatcher Pointer to receive the created dispatcher.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_scroll_dispatcher_create(struct ui_scroll_dispatcher **out_dispatcher);

/**
 * @brief Destroys a scroll dispatcher and frees its resources.
 *
 * @param dispatcher The dispatcher to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_scroll_dispatcher_destroy(struct ui_scroll_dispatcher *dispatcher);

/**
 * @brief Registers a callback to be notified of scroll events.
 *
 * @param dispatcher The scroll dispatcher.
 * @param callback The function to call when a scroll event occurs.
 * @param user_data Opaque user data to pass to the callback.
 * @param out_registration_id Pointer to receive a unique registration ID.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_scroll_dispatcher_register(struct ui_scroll_dispatcher *dispatcher,
                              ui_scroll_dispatcher_cb_t callback,
                              void *user_data, int *out_registration_id);

/**
 * @brief Unregisters a previously registered scroll callback.
 *
 * @param dispatcher The scroll dispatcher.
 * @param registration_id The ID of the callback to unregister.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if ID doesn't exist, or
 * an appropriate error code.
 */
ui_error_t
ui_scroll_dispatcher_unregister(struct ui_scroll_dispatcher *dispatcher,
                                int registration_id);

/**
 * @brief Dispatches a scroll event to all registered callbacks.
 *
 * @param dispatcher The scroll dispatcher.
 * @param info Information about the scroll event.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_dispatcher_notify(struct ui_scroll_dispatcher *dispatcher,
                                       const struct ui_scroll_info *info);

/**
 * @brief Integrates the scroll dispatcher with a layout observer.
 * When the layout changes (e.g., window resize), a generic scroll notification
 * with zero deltas is emitted to allow components to re-evaluate their
 * scroll-dependent logic.
 *
 * @param dispatcher The scroll dispatcher.
 * @param layout_observer The layout observer to integrate with.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_dispatcher_bind_layout_observer(
    struct ui_scroll_dispatcher *dispatcher,
    struct ui_layout_observer *layout_observer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SCROLL_DISPATCHER_H */
