/**
 * @file md3_navigation.h
 * @brief Material 3 Navigation components (Bar, Rail, Drawer, Suite, App Bars,
 * Tabs).
 */

#ifndef MATERIAL3_MD3_NAVIGATION_H
#define MATERIAL3_MD3_NAVIGATION_H

/* clang-format off */
#include "ui_arena.h"
#include "ui_bottom_app_bar_base.h"
#include "ui_bottom_nav_base.h"
#include "ui_error.h"
#include "ui_nav_rail_base.h"
#include "ui_sidenav_base.h"
#include "ui_tabs_base.h"
#include "ui_top_app_bar_base.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/* -------------------------------------------------------------------------
 * MD3 Navigation Bar (80dp height, 3-5 destinations, active indicator pill)
 * ------------------------------------------------------------------------- */

/**
 * @struct md3_navigation_bar
 * @brief Material 3 bottom navigation bar wrapping ui_bottom_nav_base.
 */
struct md3_navigation_bar {
  struct ui_bottom_nav_base *base;
  size_t selected_index;
};

/**
 * @brief Creates a Material 3 Navigation Bar.
 *
 * @param engine Pointer to ui_engine.
 * @param out_bar Pointer to receive newly created navigation bar.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_bar_create(
    struct ui_engine *engine, struct md3_navigation_bar **out_bar);

/**
 * @brief Destroys a Material 3 Navigation Bar.
 *
 * @param bar Navigation bar to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_navigation_bar_destroy(struct md3_navigation_bar *bar);

/**
 * @brief Adds an item to the navigation bar.
 *
 * @param bar Navigation bar.
 * @param label Item label text.
 * @param icon_name Icon resource identifier.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_bar_add_item(
    struct md3_navigation_bar *bar, const char *label, const char *icon_name);

/**
 * @brief Sets selected item index.
 *
 * @param bar Navigation bar.
 * @param index Item index.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_navigation_bar_set_selected(struct md3_navigation_bar *bar, size_t index);

/**
 * @brief Gets selected item index.
 *
 * @param bar Navigation bar.
 * @param out_index Pointer to receive selected index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_bar_get_selected(
    const struct md3_navigation_bar *bar, size_t *out_index);

/* -------------------------------------------------------------------------
 * MD3 Navigation Rail (side rail for medium/expanded window classes)
 * ------------------------------------------------------------------------- */

/**
 * @struct md3_navigation_rail
 * @brief Material 3 navigation rail wrapping ui_nav_rail_base.
 */
struct md3_navigation_rail {
  struct ui_nav_rail_base *base;
  int expanded;
};

/**
 * @brief Creates a Material 3 Navigation Rail.
 *
 * @param engine Pointer to ui_engine.
 * @param out_rail Pointer to receive newly created navigation rail.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_rail_create(
    struct ui_engine *engine, struct md3_navigation_rail **out_rail);

/**
 * @brief Destroys a Material 3 Navigation Rail.
 *
 * @param rail Navigation rail to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_navigation_rail_destroy(struct md3_navigation_rail *rail);

/**
 * @brief Sets whether the navigation rail is expanded.
 *
 * @param rail Navigation rail.
 * @param expanded Non-zero if expanded.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_rail_set_expanded(
    struct md3_navigation_rail *rail, int expanded);

/* -------------------------------------------------------------------------
 * MD3 Navigation Drawer (Standard and Modal side panes)
 * ------------------------------------------------------------------------- */

/**
 * @enum md3_navigation_drawer_variant
 * @brief Material 3 Drawer variants.
 */
enum md3_navigation_drawer_variant {
  MD3_DRAWER_STANDARD = 0, /**< Permanent desktop side sheet */
  MD3_DRAWER_MODAL         /**< Scrim backdrop sliding mobile drawer */
};

/**
 * @struct md3_navigation_drawer
 * @brief Material 3 Navigation Drawer wrapping ui_sidenav_base.
 */
struct md3_navigation_drawer {
  struct ui_sidenav_base *base;
  enum md3_navigation_drawer_variant variant;
  int is_open;
};

/**
 * @brief Creates a Material 3 Navigation Drawer.
 *
 * @param engine Pointer to ui_engine.
 * @param variant Standard or Modal drawer variant.
 * @param out_drawer Pointer to receive newly created navigation drawer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_drawer_create(
    struct ui_engine *engine, enum md3_navigation_drawer_variant variant,
    struct md3_navigation_drawer **out_drawer);

/**
 * @brief Destroys a Material 3 Navigation Drawer.
 *
 * @param drawer Drawer to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_navigation_drawer_destroy(struct md3_navigation_drawer *drawer);

/**
 * @brief Opens or closes the navigation drawer.
 *
 * @param drawer Drawer instance.
 * @param is_open Non-zero to open, 0 to close.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_drawer_set_open(
    struct md3_navigation_drawer *drawer, int is_open);

/* -------------------------------------------------------------------------
 * MD3 Navigation Suite (Adaptive navigation morphing across window classes)
 * ------------------------------------------------------------------------- */

/**
 * @enum md3_window_size_class
 * @brief Material 3 Adaptive Window Size Classes.
 */
enum md3_window_size_class {
  MD3_WINDOW_SIZE_COMPACT = 0, /**< Phones / narrow: Navigation Bar */
  MD3_WINDOW_SIZE_MEDIUM,  /**< Small tablets / foldables: Navigation Rail */
  MD3_WINDOW_SIZE_EXPANDED /**< Desktop / large displays: Navigation Drawer */
};

/**
 * @struct md3_navigation_suite
 * @brief Expressive adaptive navigation switcher wrapping bar, rail, and
 * drawer.
 */
struct md3_navigation_suite {
  enum md3_window_size_class current_class;
  struct md3_navigation_bar *bar;
  struct md3_navigation_rail *rail;
  struct md3_navigation_drawer *drawer;
};

/**
 * @brief Creates a Material 3 Expressive Navigation Suite.
 *
 * @param engine Pointer to ui_engine.
 * @param out_suite Pointer to receive newly created navigation suite.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_suite_create(
    struct ui_engine *engine, struct md3_navigation_suite **out_suite);

/**
 * @brief Destroys a Material 3 Navigation Suite.
 *
 * @param suite Suite to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_navigation_suite_destroy(struct md3_navigation_suite *suite);

/**
 * @brief Updates the active window size class and morphs layout.
 *
 * @param suite Suite instance.
 * @param size_class Window size class (Compact, Medium, Expanded).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_suite_update_size_class(
    struct md3_navigation_suite *suite, enum md3_window_size_class size_class);

/* -------------------------------------------------------------------------
 * MD3 Bottom App Bar (80dp height, action icons + FAB cradle)
 * ------------------------------------------------------------------------- */

/**
 * @struct md3_bottom_app_bar
 * @brief Material 3 Bottom App Bar wrapping ui_bottom_app_bar_base.
 */
struct md3_bottom_app_bar {
  struct ui_bottom_app_bar_base *base;
  int has_fab;
};

/**
 * @brief Creates a Material 3 Bottom App Bar.
 *
 * @param engine Pointer to ui_engine.
 * @param has_fab Non-zero to reserve embedded FAB cradle notch.
 * @param out_bar Pointer to receive newly created bottom app bar.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_bottom_app_bar_create(
    struct ui_engine *engine, int has_fab, struct md3_bottom_app_bar **out_bar);

/**
 * @brief Destroys a Material 3 Bottom App Bar.
 *
 * @param bar Bottom app bar to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_bottom_app_bar_destroy(struct md3_bottom_app_bar *bar);

/* -------------------------------------------------------------------------
 * MD3 Tabs (Primary with active indicator underline; Secondary; Expressive
 * Pill)
 * ------------------------------------------------------------------------- */

/**
 * @enum md3_tabs_variant
 * @brief Material 3 Tabs variants.
 */
enum md3_tabs_variant {
  MD3_TABS_PRIMARY = 0,    /**< Underline indicator matching label width */
  MD3_TABS_SECONDARY,      /**< Full-width container bottom border */
  MD3_TABS_EXPRESSIVE_PILL /**< Pill indicator sliding behind active tab */
};

/**
 * @struct md3_tabs
 * @brief Material 3 Tabs container wrapping ui_tabs_base.
 */
struct md3_tabs {
  struct ui_tabs_base *base;
  enum md3_tabs_variant variant;
  size_t selected_tab;
};

/**
 * @brief Creates a Material 3 Tabs container.
 *
 * @param engine Pointer to ui_engine.
 * @param variant Visual variant (Primary, Secondary, Expressive Pill).
 * @param out_tabs Pointer to receive newly created tabs container.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tabs_create(struct ui_engine *engine, enum md3_tabs_variant variant,
                struct md3_tabs **out_tabs);

/**
 * @brief Destroys a Material 3 Tabs container.
 *
 * @param tabs Tabs container to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tabs_destroy(struct md3_tabs *tabs);

/**
 * @brief Sets active selected tab index.
 *
 * @param tabs Tabs container.
 * @param index Active tab index.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tabs_set_selected(struct md3_tabs *tabs, size_t index);

/**
 * @brief Gets active selected tab index.
 *
 * @param tabs Tabs container.
 * @param out_index Pointer to receive active tab index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tabs_get_selected(const struct md3_tabs *tabs, size_t *out_index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_NAVIGATION_H */
