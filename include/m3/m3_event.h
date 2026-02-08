#ifndef M3_EVENT_H
#define M3_EVENT_H

/**
 * @file m3_event.h
 * @brief Event dispatcher and focus management for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_render.h"

/**
 * @brief Event dispatcher state.
 */
typedef struct M3EventDispatcher {
    M3Bool initialized; /**< M3_TRUE when initialized. */
    M3Widget *focused; /**< Focused widget (not owned). */
    M3Bool focus_visible; /**< M3_TRUE when focus ring should be shown. */
} M3EventDispatcher;

/**
 * @brief Initialize an event dispatcher.
 * @param dispatcher Dispatcher instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_dispatcher_init(M3EventDispatcher *dispatcher);

/**
 * @brief Shut down an event dispatcher.
 * @param dispatcher Dispatcher instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_dispatcher_shutdown(M3EventDispatcher *dispatcher);

/**
 * @brief Retrieve the focused widget.
 * @param dispatcher Dispatcher instance.
 * @param out_widget Receives the focused widget (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_dispatcher_get_focus(const M3EventDispatcher *dispatcher, M3Widget **out_widget);

/**
 * @brief Set the focused widget.
 * @param dispatcher Dispatcher instance.
 * @param widget Widget to focus.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_dispatcher_set_focus(M3EventDispatcher *dispatcher, M3Widget *widget);

/**
 * @brief Clear the focused widget.
 * @param dispatcher Dispatcher instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_dispatcher_clear_focus(M3EventDispatcher *dispatcher);

/**
 * @brief Retrieve focus ring visibility.
 * @param dispatcher Dispatcher instance.
 * @param out_visible Receives M3_TRUE when focus ring is visible.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_dispatcher_get_focus_visible(const M3EventDispatcher *dispatcher, M3Bool *out_visible);

/**
 * @brief Set focus ring visibility.
 * @param dispatcher Dispatcher instance.
 * @param visible M3_TRUE to show the focus ring.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_dispatcher_set_focus_visible(M3EventDispatcher *dispatcher, M3Bool visible);

/**
 * @brief Dispatch an input event to a widget tree.
 * @param dispatcher Dispatcher instance.
 * @param root Root render node.
 * @param event Input event to dispatch.
 * @param out_target Receives the target widget (optional).
 * @param out_handled Receives M3_TRUE if the event was handled.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_dispatch(M3EventDispatcher *dispatcher, const M3RenderNode *root, const M3InputEvent *event,
    M3Widget **out_target, M3Bool *out_handled);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_EVENT_H */
