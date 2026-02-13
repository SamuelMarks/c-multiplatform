#ifndef CMP_EVENT_H
#define CMP_EVENT_H

/**
 * @file cmp_event.h
 * @brief Event dispatcher and focus management for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_render.h"

/**
 * @brief Event dispatcher state.
 */
typedef struct CMPEventDispatcher {
  CMPBool initialized;   /**< CMP_TRUE when initialized. */
  CMPWidget *focused;    /**< Focused widget (not owned). */
  CMPBool focus_visible; /**< CMP_TRUE when focus ring should be shown. */
} CMPEventDispatcher;

/**
 * @brief Initialize an event dispatcher.
 * @param dispatcher Dispatcher instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_dispatcher_init(CMPEventDispatcher *dispatcher);

/**
 * @brief Shut down an event dispatcher.
 * @param dispatcher Dispatcher instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_event_dispatcher_shutdown(CMPEventDispatcher *dispatcher);

/**
 * @brief Retrieve the focused widget.
 * @param dispatcher Dispatcher instance.
 * @param out_widget Receives the focused widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_dispatcher_get_focus(
    const CMPEventDispatcher *dispatcher, CMPWidget **out_widget);

/**
 * @brief Set the focused widget.
 * @param dispatcher Dispatcher instance.
 * @param widget Widget to focus.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_dispatcher_set_focus(
    CMPEventDispatcher *dispatcher, CMPWidget *widget);

/**
 * @brief Clear the focused widget.
 * @param dispatcher Dispatcher instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_event_dispatcher_clear_focus(CMPEventDispatcher *dispatcher);

/**
 * @brief Retrieve focus ring visibility.
 * @param dispatcher Dispatcher instance.
 * @param out_visible Receives CMP_TRUE when focus ring is visible.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_dispatcher_get_focus_visible(
    const CMPEventDispatcher *dispatcher, CMPBool *out_visible);

/**
 * @brief Set focus ring visibility.
 * @param dispatcher Dispatcher instance.
 * @param visible CMP_TRUE to show the focus ring.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_dispatcher_set_focus_visible(
    CMPEventDispatcher *dispatcher, CMPBool visible);

/**
 * @brief Dispatch an input event to a widget tree.
 * @param dispatcher Dispatcher instance.
 * @param root Root render node.
 * @param event Input event to dispatch.
 * @param out_target Receives the target widget (optional).
 * @param out_handled Receives CMP_TRUE if the event was handled.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_dispatch(CMPEventDispatcher *dispatcher,
                                        const CMPRenderNode *root,
                                        const CMPInputEvent *event,
                                        CMPWidget **out_target,
                                        CMPBool *out_handled);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for validating a render rectangle.
 * @param rect Rectangle to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_test_validate_rect(const CMPRect *rect);

/**
 * @brief Test wrapper to query whether a widget is focusable.
 * @param widget Widget to query (may be NULL).
 * @param out_focusable Receives CMP_TRUE when focusable.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_test_widget_focusable(const CMPWidget *widget,
                                                     CMPBool *out_focusable);

/**
 * @brief Test wrapper for hit testing.
 * @param node Render node to test.
 * @param x X coordinate in pixels.
 * @param y Y coordinate in pixels.
 * @param out_widget Receives the hit widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_test_hit_test(const CMPRenderNode *node,
                                             CMPScalar x, CMPScalar y,
                                             CMPWidget **out_widget);

/**
 * @brief Test wrapper to dispatch directly to a widget.
 * @param widget Target widget (may be NULL).
 * @param event Event to dispatch.
 * @param out_handled Receives CMP_TRUE when handled.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_test_dispatch_to_widget(
    CMPWidget *widget, const CMPInputEvent *event, CMPBool *out_handled);

/**
 * @brief Force hit-test to surface a containment error (tests only).
 * @param enable CMP_TRUE to force errors, CMP_FALSE to disable.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_event_test_set_force_contains_error(CMPBool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_EVENT_H */
