/**
 * @file ui_adaptive_pane_scaffold_base.h
 * @brief Base unstyled adaptive multi-pane scaffold coordinator for responsive
 * layouts.
 */

#ifndef UI_ADAPTIVE_PANE_SCAFFOLD_BASE_H
#define UI_ADAPTIVE_PANE_SCAFFOLD_BASE_H

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum ui_adaptive_window_width_class
 * @brief Window size width class based on standard responsive breakpoints.
 */
enum ui_adaptive_window_width_class {
  /** @brief Compact width (< 600dp, typically mobile phones in portrait). */
  UI_WINDOW_WIDTH_COMPACT = 0,
  /** @brief Medium width (600dp - 839dp, small tablets / foldables). */
  UI_WINDOW_WIDTH_MEDIUM = 1,
  /** @brief Expanded width (>= 840dp, desktops / large tablets). */
  UI_WINDOW_WIDTH_EXPANDED = 2
};

/**
 * @enum ui_adaptive_pane_role
 * @brief Pane semantic role in the adaptive scaffold hierarchy.
 */
enum ui_adaptive_pane_role {
  /** @brief Primary navigation/list pane (left or primary view). */
  UI_PANE_ROLE_PRIMARY = 0,
  /** @brief Secondary detail pane (middle or detail view). */
  UI_PANE_ROLE_SECONDARY = 1,
  /** @brief Supporting supplementary pane (right or side-drawer view). */
  UI_PANE_ROLE_SUPPORTING = 2
};

/**
 * @enum ui_adaptive_scaffold_type
 * @brief Canonical adaptive layout topology.
 */
enum ui_adaptive_scaffold_type {
  /** @brief List-detail pane layout. */
  UI_ADAPTIVE_SCAFFOLD_LIST_DETAIL = 0,
  /** @brief Supporting pane layout. */
  UI_ADAPTIVE_SCAFFOLD_SUPPORTING_PANE = 1
};

/**
 * @brief Opaque handle representing the base adaptive pane scaffold
 * coordinator.
 */
struct ui_adaptive_pane_scaffold_base;

/**
 * @brief Creates a new unstyled adaptive pane scaffold coordinator.
 *
 * @param type The scaffold topology (list-detail or supporting pane).
 * @param out_scaffold Pointer receiving newly created scaffold instance.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY if allocation fails,
 *         or UI_ERROR_INVALID_ARGUMENT if out_scaffold is null.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_adaptive_pane_scaffold_base_create(
    enum ui_adaptive_scaffold_type type,
    struct ui_adaptive_pane_scaffold_base **out_scaffold);

/**
 * @brief Destroys an adaptive pane scaffold base instance.
 *
 * @param scaffold The scaffold instance to destroy. If null, does nothing.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_adaptive_pane_scaffold_base_destroy(
    struct ui_adaptive_pane_scaffold_base *scaffold);

/**
 * @brief Retrieves underlying ui_component handle for mounting and styling.
 *
 * @param scaffold The scaffold instance.
 * @param out_component Pointer receiving underlying component handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_adaptive_pane_scaffold_base_get_component(
    struct ui_adaptive_pane_scaffold_base *scaffold,
    struct ui_component **out_component);

/**
 * @brief Sets the active window width class (triggers automatic layout
 * adaptation).
 *
 * @param scaffold The scaffold instance.
 * @param width_class Compact, Medium, or Expanded.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_adaptive_pane_scaffold_base_set_window_width_class(
    struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_window_width_class width_class);

/**
 * @brief Retrieves current window width class.
 *
 * @param scaffold The scaffold instance.
 * @param out_width_class Pointer receiving current width class.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_adaptive_pane_scaffold_base_get_window_width_class(
    const struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_window_width_class *out_width_class);

/**
 * @brief Sets or removes a component in a designated pane slot.
 *
 * @param scaffold The scaffold instance.
 * @param role Primary, Secondary, or Supporting role slot.
 * @param pane_component Component to mount into the slot (or NULL to remove).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_adaptive_pane_scaffold_base_set_pane(
    struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_pane_role role, struct ui_component *pane_component);

/**
 * @brief Retrieves the component currently mounted in a designated pane slot.
 *
 * @param scaffold The scaffold instance.
 * @param role Primary, Secondary, or Supporting role slot.
 * @param out_pane_component Pointer receiving the component handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_adaptive_pane_scaffold_base_get_pane(
    const struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_pane_role role, struct ui_component **out_pane_component);

/**
 * @brief Sets the active focus pane (for compact single-pane navigation).
 *
 * @param scaffold The scaffold instance.
 * @param role Active pane to display in compact mode.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_adaptive_pane_scaffold_base_set_active_pane(
    struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_pane_role role);

/**
 * @brief Retrieves the currently active focus pane.
 *
 * @param scaffold The scaffold instance.
 * @param out_role Pointer receiving active pane role.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_adaptive_pane_scaffold_base_get_active_pane(
    const struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_pane_role *out_role);

/**
 * @brief Configures whether the supporting pane should levitate as a modal
 * dialog on compact screens.
 *
 * @param scaffold The scaffold instance.
 * @param levitate_in_dialog Nonzero to enable dialog levitation, zero to
 * disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_adaptive_pane_scaffold_base_set_dialog_levitation(
    struct ui_adaptive_pane_scaffold_base *scaffold, int levitate_in_dialog);

/**
 * @brief Queries whether dialog levitation is currently enabled.
 *
 * @param scaffold The scaffold instance.
 * @param out_levitated Pointer receiving 1 if enabled, 0 if disabled.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_adaptive_pane_scaffold_base_is_dialog_levitated(
    const struct ui_adaptive_pane_scaffold_base *scaffold, int *out_levitated);

#ifdef __cplusplus
}
#endif

#endif /* UI_ADAPTIVE_PANE_SCAFFOLD_BASE_H */
