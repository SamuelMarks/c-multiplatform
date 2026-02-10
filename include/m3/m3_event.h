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

#ifdef M3_TESTING
/**
 * @brief Test wrapper for validating a render rectangle.
 * @param rect Rectangle to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_test_validate_rect(const M3Rect *rect);

/**
 * @brief Test wrapper to query whether a widget is focusable.
 * @param widget Widget to query (may be NULL).
 * @param out_focusable Receives M3_TRUE when focusable.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_test_widget_focusable(const M3Widget *widget, M3Bool *out_focusable);

/**
 * @brief Test wrapper for hit testing.
 * @param node Render node to test.
 * @param x X coordinate in pixels.
 * @param y Y coordinate in pixels.
 * @param out_widget Receives the hit widget (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_test_hit_test(const M3RenderNode *node, M3Scalar x, M3Scalar y, M3Widget **out_widget);

/**
 * @brief Test wrapper to dispatch directly to a widget.
 * @param widget Target widget (may be NULL).
 * @param event Event to dispatch.
 * @param out_handled Receives M3_TRUE when handled.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_test_dispatch_to_widget(M3Widget *widget, const M3InputEvent *event, M3Bool *out_handled);

/**
 * @brief Force hit-test to surface a containment error (tests only).
 * @param enable M3_TRUE to force errors, M3_FALSE to disable.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_event_test_set_force_contains_error(M3Bool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_EVENT_H */
