/**
 * @file cupertino_tab_scaffold.h
 * @brief Cupertino Tab Scaffold (CupertinoTabScaffold) and Tab Controller
 * (CupertinoTabController) coordinating iOS tabbed navigation conforming to
 * Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_TAB_SCAFFOLD_H
#define CUPERTINO_CUPERTINO_TAB_SCAFFOLD_H

/* clang-format off */
#include "cupertino/cupertino_tab_bar.h"
#include "ui_component.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @struct cupertino_tab_controller
 * @brief Signal-aware coordinator for Cupertino tab switching state.
 */
struct cupertino_tab_controller {
  size_t selected_index; /**< Currently active tab index. */
  size_t tab_count;      /**< Total number of tabs managed. */
};

/**
 * @struct cupertino_tab_scaffold_descriptor
 * @brief Configuration descriptor for initializing a Cupertino Tab Scaffold.
 */
struct cupertino_tab_scaffold_descriptor {
  size_t initial_index; /**< Initial active tab index. */
  int bar_hidden;       /**< 1 if tab bar starts hidden (e.g. fullscreen). */
  int is_dark;          /**< 1 for dark mode canvas. */
};

/**
 * @struct cupertino_tab_scaffold
 * @brief Cupertino Tab Scaffold instance hosting a bottom tab bar and
 * multi-view content container.
 */
struct cupertino_tab_scaffold {
  struct cupertino_tab_bar *tab_bar;           /**< Hosted bottom tab bar. */
  struct cupertino_tab_controller *controller; /**< Tab controller. */
  int owns_controller; /**< 1 if controller was created internally. */
  int bar_hidden;      /**< Tab bar hidden state. */
  int is_dark;         /**< Dark mode appearance state. */
  struct ui_component *
      tab_views[CUPERTINO_TAB_BAR_MAX_ITEMS]; /**< Slotted tab content views. */
  int is_tab_instantiated[CUPERTINO_TAB_BAR_MAX_ITEMS]; /**< Lazy instantiation
                                                           flags. */
};

/**
 * @brief Creates a Cupertino Tab Controller.
 *
 * @param initial_index Initial selected tab index.
 * @param tab_count Total number of tabs (must be > 0 and <=
 * CUPERTINO_TAB_BAR_MAX_ITEMS).
 * @param out_controller Pointer to receive newly created controller.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_controller_create(
    size_t initial_index, size_t tab_count,
    struct cupertino_tab_controller **out_controller);

/**
 * @brief Destroys a Cupertino Tab Controller.
 *
 * @param controller Controller instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_controller_destroy(struct cupertino_tab_controller *controller);

/**
 * @brief Sets active tab index in controller.
 *
 * @param controller Target controller.
 * @param index New active index.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if index >=
 * tab_count, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_controller_set_index(
    struct cupertino_tab_controller *controller, size_t index);

/**
 * @brief Gets active tab index from controller.
 *
 * @param controller Target controller.
 * @param out_index Pointer to receive active index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_controller_get_index(
    const struct cupertino_tab_controller *controller, size_t *out_index);

/**
 * @brief Gets total tab count managed by controller.
 *
 * @param controller Target controller.
 * @param out_count Pointer to receive tab count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_controller_get_tab_count(
    const struct cupertino_tab_controller *controller, size_t *out_count);

/**
 * @brief Creates a new Cupertino Tab Scaffold instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_scaffold Pointer to receive newly created scaffold.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_create(
    struct ui_engine *engine,
    const struct cupertino_tab_scaffold_descriptor *desc,
    struct cupertino_tab_scaffold **out_scaffold);

/**
 * @brief Destroys a Cupertino Tab Scaffold.
 *
 * @param scaffold Scaffold instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_scaffold_destroy(struct cupertino_tab_scaffold *scaffold);

/**
 * @brief Associates a Cupertino Tab Bar with the scaffold.
 *
 * @param scaffold Target scaffold.
 * @param tab_bar Tab bar instance to host.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_set_tab_bar(
    struct cupertino_tab_scaffold *scaffold, struct cupertino_tab_bar *tab_bar);

/**
 * @brief Gets hosted Cupertino Tab Bar.
 *
 * @param scaffold Target scaffold.
 * @param out_tab_bar Pointer to receive hosted tab bar pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_get_tab_bar(
    const struct cupertino_tab_scaffold *scaffold,
    struct cupertino_tab_bar **out_tab_bar);

/**
 * @brief Binds a custom tab controller to the scaffold.
 *
 * @param scaffold Target scaffold.
 * @param controller Controller instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_set_controller(
    struct cupertino_tab_scaffold *scaffold,
    struct cupertino_tab_controller *controller);

/**
 * @brief Gets active tab controller.
 *
 * @param scaffold Target scaffold.
 * @param out_controller Pointer to receive controller pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_get_controller(
    const struct cupertino_tab_scaffold *scaffold,
    struct cupertino_tab_controller **out_controller);

/**
 * @brief Slots a view component for a specific tab index (lazy instantiation
 * support).
 *
 * @param scaffold Target scaffold.
 * @param tab_index Index of the tab slot.
 * @param view Component view to associate.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if index invalid,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_set_tab_view(
    struct cupertino_tab_scaffold *scaffold, size_t tab_index,
    struct ui_component *view);

/**
 * @brief Retrieves slotted view component for a tab index.
 *
 * @param scaffold Target scaffold.
 * @param tab_index Index of the tab slot.
 * @param out_view Pointer to receive view component pointer.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if index invalid,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_get_tab_view(
    const struct cupertino_tab_scaffold *scaffold, size_t tab_index,
    struct ui_component **out_view);

/**
 * @brief Retrieves currently visible active tab view component.
 *
 * @param scaffold Target scaffold.
 * @param out_view Pointer to receive view component pointer (or NULL if
 * unslotted).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_get_active_view(
    const struct cupertino_tab_scaffold *scaffold,
    struct ui_component **out_view);

/**
 * @brief Sets dynamic auto-hiding state of the bottom tab bar (e.g. for
 * fullscreen presentation).
 *
 * @param scaffold Target scaffold.
 * @param hidden 1 to hide tab bar, 0 to show.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_set_bar_hidden(
    struct cupertino_tab_scaffold *scaffold, int hidden);

/**
 * @brief Gets tab bar hidden status.
 *
 * @param scaffold Target scaffold.
 * @param out_hidden Pointer to receive hidden flag (1 or 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_scaffold_get_bar_hidden(
    const struct cupertino_tab_scaffold *scaffold, int *out_hidden);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_TAB_SCAFFOLD_H */
