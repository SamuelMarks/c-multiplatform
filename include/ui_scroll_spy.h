#ifndef UI_SCROLL_SPY_H
#define UI_SCROLL_SPY_H

/**
 * \file ui_scroll_spy.h
 * \brief UI Scroll Spy component.
 *
 * This file contains definitions for tracking content sections within
 * a scrolling container, emitting signals when specific sections enter
 * or leave the active view.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
#include "ui_signal.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a scroll spy instance.
 */
struct ui_scroll_spy;

/**
 * @brief Creates a new scroll spy behavior instance.
 *
 * @param out_spy Pointer to receive the allocated scroll spy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_create(struct ui_scroll_spy **out_spy);

/**
 * @brief Destroys a scroll spy instance.
 *
 * @param spy The scroll spy to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_destroy(struct ui_scroll_spy *spy);

/**
 * @brief Sets the root scrolling container and its observation margin.
 *
 * @param spy The scroll spy.
 * @param root The scrolling container DOM node (or NULL for viewport).
 * @param root_margin_px Margin to apply to the root bounds (usually negative to
 * trigger early).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_set_root(struct ui_scroll_spy *spy,
                                  struct ui_dom_node *root, int root_margin_px);

/**
 * @brief Adds a target section to be tracked by the scroll spy.
 *
 * @param spy The scroll spy.
 * @param target The target DOM node representing the content section.
 * @param section_id The unique user-defined ID for this section.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_add_target(struct ui_scroll_spy *spy,
                                    struct ui_dom_node *target, int section_id);

/**
 * @brief Removes a target section from the scroll spy.
 *
 * @param spy The scroll spy.
 * @param target The target DOM node to stop tracking.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND, or an appropriate error
 * code.
 */
ui_error_t ui_scroll_spy_remove_target(struct ui_scroll_spy *spy,
                                       struct ui_dom_node *target);

/**
 * @brief Binds a signal that will receive the active section ID.
 * The payload of the signal should be castable to (int).
 *
 * @param spy The scroll spy.
 * @param active_signal The signal to update when the active section changes.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_bind_active_section(struct ui_scroll_spy *spy,
                                             struct ui_signal *active_signal);

/**
 * @brief Triggers an evaluation of the underlying intersection observer.
 * Typically called during a layout or scroll event.
 *
 * @param spy The scroll spy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_evaluate(struct ui_scroll_spy *spy);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SCROLL_SPY_H */
