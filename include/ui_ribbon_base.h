#ifndef UI_RIBBON_BASE_H
#define UI_RIBBON_BASE_H

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
 * @brief Opaque handle for the ribbon component.
 */
struct ui_ribbon_base;

/**
 * @brief Represents the display state of a ribbon group based on available
 * width.
 */
enum ui_ribbon_group_collapse_state {
  UI_RIBBON_GROUP_COLLAPSE_STATE_NORMAL =
      0, /**< All actions fully visible with labels */
  UI_RIBBON_GROUP_COLLAPSE_STATE_COMPACT =
      1, /**< Actions shown as icons only, labels hidden */
  UI_RIBBON_GROUP_COLLAPSE_STATE_COLLAPSED =
      2 /**< Group collapsed into a single dropdown button */
};

/**
 * @brief Configuration for a single command group within a ribbon tab.
 */
struct ui_ribbon_group_config {
  int group_id;
  int min_width_normal;  /**< Minimum width required to show in NORMAL state */
  int min_width_compact; /**< Minimum width required to show in COMPACT state */
  int priority; /**< Lower priority groups collapse first during resize */
};

/**
 * @brief Configuration for a contextual tab that only appears during specific
 * selections.
 */
struct ui_ribbon_contextual_tab_config {
  int tab_id;
  ui_bool_t is_active;
};

/**
 * @brief Creates a ribbon base component.
 *
 * @param arena The memory arena to use for allocation.
 * @param out_ribbon Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_ribbon_base_create(struct ui_arena *arena,
                                    struct ui_ribbon_base **out_ribbon);

/**
 * @brief Destroys a ribbon base component.
 *
 * @param ribbon The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_ribbon_base_destroy(struct ui_ribbon_base *ribbon);

/**
 * @brief Adds a command group configuration to the ribbon for width-based
 * collapse calculations.
 *
 * @param ribbon The component.
 * @param config The group configuration to add.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_ribbon_base_add_group_config(struct ui_ribbon_base *ribbon,
                                const struct ui_ribbon_group_config *config);

/**
 * @brief Calculates and updates the collapse states for all groups based on the
 * available width. This should be called by the layout pass during window
 * resize.
 *
 * @param ribbon The component.
 * @param available_width The total physical width available to the ribbon.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_ribbon_base_recalculate_overflow(struct ui_ribbon_base *ribbon,
                                                  int available_width);

/**
 * @brief Retrieves the calculated collapse state for a specific group.
 *
 * @param ribbon The component.
 * @param group_id The ID of the group.
 * @param out_state Pointer to receive the collapse state.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_ribbon_base_get_group_state(const struct ui_ribbon_base *ribbon,
                               int group_id,
                               enum ui_ribbon_group_collapse_state *out_state);

/**
 * @brief Retrieves the signal emitted when a group's collapse state changes.
 * The payload contains the int group_id that changed.
 *
 * @param ribbon The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_ribbon_base_get_group_state_changed_signal(struct ui_ribbon_base *ribbon,
                                              ui_signal_t **out_signal);

/**
 * @brief Sets the active status of a contextual tab (e.g. showing "Table Tools"
 * when a table is selected).
 *
 * @param ribbon The component.
 * @param tab_id The contextual tab ID.
 * @param is_active True to show, false to hide.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_ribbon_base_set_contextual_tab_active(struct ui_ribbon_base *ribbon,
                                         int tab_id, ui_bool_t is_active);

/**
 * @brief Retrieves the active status of a contextual tab.
 *
 * @param ribbon The component.
 * @param tab_id The contextual tab ID.
 * @param out_is_active Pointer to receive the boolean status.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_ribbon_base_get_contextual_tab_active(const struct ui_ribbon_base *ribbon,
                                         int tab_id, ui_bool_t *out_is_active);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RIBBON_BASE_H */
