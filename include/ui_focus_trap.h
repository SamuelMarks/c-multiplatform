#ifndef UI_FOCUS_TRAP_H
#define UI_FOCUS_TRAP_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
#include "ui_focus_manager.h"
#include "ui_keyboard_responder.h"
/* clang-format on */

/**
 * @brief Represents a focus trap instance.
 */
struct ui_focus_trap;

/**
 * @brief Creates a new focus trap.
 * @param out_trap Pointer to receive the new focus trap instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_trap_create(struct ui_focus_trap **out_trap);

/**
 * @brief Destroys a focus trap.
 * @param trap The trap to destroy.
 */
ui_error_t ui_focus_trap_destroy(struct ui_focus_trap *trap);

/**
 * @brief Initializes the trap for a given DOM node.
 * This will push the trap root to the focus manager so focus doesn't escape.
 *
 * @param trap The focus trap instance.
 * @param manager The focus manager.
 * @param root The root DOM node of the area to trap focus within.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_trap_activate(struct ui_focus_trap *trap,
                                  struct ui_focus_manager *manager,
                                  struct ui_dom_node *root);

/**
 * @brief Deactivates the trap, popping it from the focus manager and restoring
 * focus.
 *
 * @param trap The focus trap instance.
 * @param manager The focus manager.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_focus_trap_deactivate(struct ui_focus_trap *trap,
                                    struct ui_focus_manager *manager);

/**
 * @brief Hooks the trap into the keyboard responder to listen for Tab/Shift-Tab
 * globally. If the trap is active, it intercepts the tab navigation to cycle
 * focus.
 *
 * @param trap The focus trap instance.
 * @param responder The keyboard responder.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_focus_trap_attach_keyboard(struct ui_focus_trap *trap,
                              struct ui_keyboard_responder *responder);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FOCUS_TRAP_H */
