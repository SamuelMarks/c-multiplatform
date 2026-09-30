/**
 * @file md3_a11y.h
 * @brief Material 3 Accessibility (a11y) safeguards, touch targets,
 * high-contrast forced colors, focus rings, roving tabindex, live announcer,
 * and ARIA synchronization.
 */

#ifndef MATERIAL3_MD3_A11Y_H
#define MATERIAL3_MD3_A11Y_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
#include "ui_event.h"
#include "ui_focus_ring.h"
#include "ui_live_announcer.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_component;
struct ui_dom_node;
struct ui_engine;
struct ui_focus_manager;
struct ui_focus_trap;
struct md3_roving_tabindex;
struct md3_announcer;

/**
 * @brief Computes padding needed to enforce the 48x48dp minimum touch bounds.
 *
 * @param width Input element width in dp.
 * @param height Input element height in dp.
 * @param out_pad_x Pointer to receive horizontal padding in dp.
 * @param out_pad_y Pointer to receive vertical padding in dp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad inputs.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_a11y_enforce_touch_target(
    float width, float height, float *out_pad_x, float *out_pad_y);

/**
 * @brief Validates if an element's bounding box satisfies the 48x48dp minimum.
 *
 * @param width Input element width in dp.
 * @param height Input element height in dp.
 * @param out_is_valid Pointer to receive 1 if valid, 0 if sub-minimum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad inputs.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_a11y_validate_touch_target(float width, float height, int *out_is_valid);

/**
 * @brief Checks if high-contrast / forced-colors mode is enabled on the engine.
 *
 * @param engine Pointer to the UI engine instance.
 * @param out_enabled Pointer to receive 1 if enabled, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad inputs.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_a11y_is_high_contrast_enabled(struct ui_engine *engine, int *out_enabled);

/**
 * @brief Enables or disables high-contrast / forced-colors mode on the engine.
 *
 * @param engine Pointer to the UI engine instance.
 * @param enabled Non-zero to enable, zero to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad inputs.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_a11y_set_high_contrast_enabled(struct ui_engine *engine, int enabled);

/**
 * @brief Retrieves forced-colors border substitution for high contrast mode.
 *
 * @param is_high_contrast Non-zero if high contrast mode is active.
 * @param out_border_width Pointer to receive border width (2dp in high
 * contrast).
 * @param out_border_color Pointer to receive high contrast border color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad inputs.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_a11y_get_forced_colors_border(int is_high_contrast, float *out_border_width,
                                  ui_color_t *out_border_color);

/**
 * @brief Generates a WCAG 2.1 AAA compliant dual-color high-contrast focus
 * ring.
 *
 * @param bg_color Background color the ring will be rendered against.
 * @param out_ring Pointer to receive the focus ring configuration.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad inputs.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_a11y_get_focus_ring(ui_color_t bg_color, struct ui_focus_ring *out_ring);

/**
 * @brief Creates a roving tabindex coordinator for a group of interactive
 * items.
 *
 * @param item_count Total number of items in the group.
 * @param out_roving Pointer to receive the allocated coordinator handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_roving_tabindex_create(
    size_t item_count, struct md3_roving_tabindex **out_roving);

/**
 * @brief Destroys a roving tabindex coordinator.
 *
 * @param roving Coordinator handle to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_roving_tabindex_destroy(struct md3_roving_tabindex *roving);

/**
 * @brief Handles directional navigation keys to update roving focus.
 *
 * @param roving Coordinator handle.
 * @param key Key code (UI_KEY_ARROW_UP, UI_KEY_ARROW_DOWN, UI_KEY_ARROW_LEFT,
 * UI_KEY_ARROW_RIGHT, UI_KEY_HOME, UI_KEY_END).
 * @param out_focused_index Pointer to receive the newly focused item index.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_roving_tabindex_handle_key(struct md3_roving_tabindex *roving,
                               enum ui_key_code key, size_t *out_focused_index);

/**
 * @brief Retrieves current focused index in roving tabindex.
 *
 * @param roving Coordinator handle.
 * @param out_focused_index Pointer to receive currently focused index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_roving_tabindex_get_focused(
    const struct md3_roving_tabindex *roving, size_t *out_focused_index);

/**
 * @brief Explicitly sets the focused index in roving tabindex.
 *
 * @param roving Coordinator handle.
 * @param index Index to set as focused.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_roving_tabindex_set_focused(
    struct md3_roving_tabindex *roving, size_t index);

/**
 * @brief Creates a live region announcer for dynamic screen reader updates.
 *
 * @param out_announcer Pointer to receive the allocated announcer handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_announcer_create(struct md3_announcer **out_announcer);

/**
 * @brief Destroys a live region announcer.
 *
 * @param announcer Announcer handle to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_announcer_destroy(struct md3_announcer *announcer);

/**
 * @brief Queues a message announcement with polite or assertive politeness.
 *
 * @param announcer Announcer handle.
 * @param message Message text to announce.
 * @param politeness Politeness level (UI_LIVE_POLITE or UI_LIVE_ASSERTIVE).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_announcer_announce(struct md3_announcer *announcer, const char *message,
                       enum ui_live_politeness politeness);

/**
 * @brief Clears pending announcements in the live announcer queue.
 *
 * @param announcer Announcer handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_announcer_clear(struct md3_announcer *announcer);

/**
 * @brief Synchronizes ARIA attributes onto a UI component.
 *
 * @param comp UI component to update.
 * @param role Accessibility role name (e.g. "button", "checkbox", "dialog").
 * @param is_checked 1 if checked, 0 if unchecked, -1 if indeterminate/omitted.
 * @param is_expanded 1 if expanded, 0 if collapsed, -1 if omitted.
 * @param is_selected 1 if selected, 0 if unselected, -1 if omitted.
 * @param is_disabled 1 if disabled, 0 if enabled.
 * @param value_now Current value for range widgets (-1.0 if not applicable).
 * @param value_min Minimum value for range widgets (-1.0 if not applicable).
 * @param value_max Maximum value for range widgets (-1.0 if not applicable).
 * @param has_popup Popup type (e.g. "menu", "dialog", "listbox", or NULL).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_a11y_sync_aria(
    struct ui_component *comp, const char *role, int is_checked,
    int is_expanded, int is_selected, int is_disabled, double value_now,
    double value_min, double value_max, const char *has_popup);

/**
 * @brief Traps keyboard focus within an overlay container (modal dialog/sheet).
 *
 * @param out_trap Pointer to receive the active focus trap.
 * @param manager Focus manager instance.
 * @param root Root DOM node of modal area.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_a11y_trap_focus(struct ui_focus_trap **out_trap,
                    struct ui_focus_manager *manager, struct ui_dom_node *root);

/**
 * @brief Releases a previously trapped focus context.
 *
 * @param trap Focus trap handle to release.
 * @param manager Focus manager instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_a11y_release_focus(
    struct ui_focus_trap *trap, struct ui_focus_manager *manager);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_A11Y_H */
