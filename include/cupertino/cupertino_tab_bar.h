/**
 * @file cupertino_tab_bar.h
 * @brief Cupertino Tab Bar component conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_TAB_BAR_H
#define CUPERTINO_CUPERTINO_TAB_BAR_H

/* clang-format off */
#include "ui_bottom_nav_base.h"
#include "ui_color_space.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_TAB_BAR_MAX_ITEMS 8
#define CUPERTINO_TAB_BAR_HEIGHT_STANDARD 49.0f
#define CUPERTINO_TAB_BAR_HEIGHT_HOME_INDICATOR 83.0f

/**
 * @struct cupertino_tab_item_descriptor
 * @brief Descriptor for a single tab item in a Cupertino tab bar.
 */
struct cupertino_tab_item_descriptor {
  const char *label;      /**< Localized title text. */
  const char *icon_name;  /**< SF Symbol or icon asset name. */
  const char *badge_text; /**< Optional badge text (e.g. "3", "99+"). */
};

/**
 * @struct cupertino_tab_bar_descriptor
 * @brief Configuration descriptor for creating a Cupertino Tab Bar.
 */
struct cupertino_tab_bar_descriptor {
  int is_home_indicator_present; /**< 1 for 83pt height (iPhone X+), 0 for 49pt.
                                  */
  const struct cupertino_tab_item_descriptor *items; /**< Initial items. */
  size_t item_count;                                 /**< Initial item count. */
  size_t initial_index;                              /**< Selected index. */
};

/**
 * @struct cupertino_tab_item
 * @brief Internal data for a tab bar item.
 */
struct cupertino_tab_item {
  struct ui_bottom_nav_item_base *base_item; /**< CDK item primitive. */
  char label[64];                            /**< Item label string. */
  char icon_name[64];                        /**< Icon identifier. */
  char badge[16];                            /**< Badge pill text. */
  int has_badge;                             /**< 1 if badge present. */
};

/**
 * @struct cupertino_tab_bar
 * @brief Cupertino Tab Bar instance wrapping ui_bottom_nav_base.
 */
struct cupertino_tab_bar {
  struct ui_bottom_nav_base *base; /**< Wrapped CDK bottom nav primitive. */
  int is_home_indicator_present;   /**< Insets for bottom home indicator. */
  size_t selected_index;           /**< Currently active tab index. */
  size_t item_count;               /**< Number of tab items. */
  int adaptive_sidebar_enabled;    /**< 1 if adaptive sidebar enabled. */
  float sidebar_breakpoint; /**< Viewport width threshold (e.g. 768pt). */
  float sidebar_width;      /**< Sidebar width (e.g. 260pt). */
  int is_sidebar_mode;      /**< 1 if active as sidebar, 0 if bottom bar. */
  struct cupertino_tab_item items[CUPERTINO_TAB_BAR_MAX_ITEMS]; /**< Items. */
};

/**
 * @brief Creates a new Cupertino tab bar instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_tab_bar Pointer to receive newly created tab bar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_create(
    struct ui_engine *engine, const struct cupertino_tab_bar_descriptor *desc,
    struct cupertino_tab_bar **out_tab_bar);

/**
 * @brief Destroys a Cupertino tab bar and all constituent items.
 *
 * @param tab_bar Tab bar instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_bar_destroy(struct cupertino_tab_bar *tab_bar);

/**
 * @brief Appends a tab item to the tab bar.
 *
 * @param tab_bar Target tab bar.
 * @param item_desc Item configuration descriptor.
 * @param out_index Pointer to receive index of newly added item.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_MEMORY.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_add_item(
    struct cupertino_tab_bar *tab_bar,
    const struct cupertino_tab_item_descriptor *item_desc, size_t *out_index);

/**
 * @brief Updates the selected tab index.
 *
 * @param tab_bar Target tab bar.
 * @param index New active index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_bar_set_selected(struct cupertino_tab_bar *tab_bar, size_t index);

/**
 * @brief Retrieves the currently selected tab index.
 *
 * @param tab_bar Target tab bar.
 * @param out_index Pointer to receive active index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_get_selected(
    const struct cupertino_tab_bar *tab_bar, size_t *out_index);

/**
 * @brief Sets or clears the badge text on a tab item.
 *
 * @param tab_bar Target tab bar.
 * @param index Item index.
 * @param badge_text Badge string (e.g. "3", "99+"), or NULL to clear badge.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_set_badge(
    struct cupertino_tab_bar *tab_bar, size_t index, const char *badge_text);

/**
 * @brief Retrieves the badge text on a tab item.
 *
 * @param tab_bar Target tab bar.
 * @param index Item index.
 * @param out_badge_text Pointer to receive const pointer to badge string, or
 * NULL if absent.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_bar_get_badge(const struct cupertino_tab_bar *tab_bar,
                            size_t index, const char **out_badge_text);

/**
 * @brief Sets whether the home indicator space is inset into the tab bar.
 *
 * @param tab_bar Target tab bar.
 * @param present 1 for 83pt height, 0 for 49pt standard height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_bar_set_home_indicator_present(struct cupertino_tab_bar *tab_bar,
                                             int present);

/**
 * @brief Queries whether the home indicator space is enabled.
 *
 * @param tab_bar Target tab bar.
 * @param out_present Pointer to receive 1 if present, 0 if standard.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_bar_is_home_indicator_present(
    const struct cupertino_tab_bar *tab_bar, int *out_present);

/**
 * @brief Retrieves the computed height of the tab bar in points.
 *
 * @param tab_bar Target tab bar.
 * @param out_height Pointer to receive 49.0f or 83.0f.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_get_height(
    const struct cupertino_tab_bar *tab_bar, float *out_height);

/**
 * @brief Retrieves the total count of items in the tab bar.
 *
 * @param tab_bar Target tab bar.
 * @param out_count Pointer to receive item count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_get_item_count(
    const struct cupertino_tab_bar *tab_bar, size_t *out_count);

/**
 * @brief Retrieves the underlying CDK bottom nav primitive.
 *
 * @param tab_bar Target tab bar.
 * @param out_base Pointer to receive ui_bottom_nav_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_get_base(
    struct cupertino_tab_bar *tab_bar, struct ui_bottom_nav_base **out_base);

/**
 * @brief Enables adaptive sidebar collapse for iPadOS and macOS wide viewports.
 *
 * @param tab_bar Target tab bar.
 * @param enabled 1 to enable adaptive collapse, 0 to keep bottom tab bar fixed.
 * @param breakpoint Viewport width threshold in points (or 0 for default
 * 768pt).
 * @param sidebar_width Sidebar column width in points (or 0 for default 260pt).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_set_adaptive_sidebar(
    struct cupertino_tab_bar *tab_bar, int enabled, float breakpoint,
    float sidebar_width);

/**
 * @brief Queries adaptive sidebar configuration.
 *
 * @param tab_bar Target tab bar.
 * @param out_enabled Pointer to receive enabled flag.
 * @param out_breakpoint Pointer to receive breakpoint in points.
 * @param out_sidebar_width Pointer to receive sidebar width in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_get_adaptive_sidebar(
    const struct cupertino_tab_bar *tab_bar, int *out_enabled,
    float *out_breakpoint, float *out_sidebar_width);

/**
 * @brief Updates layout mode (bottom bar vs sidebar) given the current viewport
 * width.
 *
 * @param tab_bar Target tab bar.
 * @param viewport_width Container or screen width in points.
 * @param out_is_sidebar Pointer to receive 1 if switched to sidebar, 0 if
 * bottom bar.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_bar_update_layout_for_width(struct cupertino_tab_bar *tab_bar,
                                          float viewport_width,
                                          int *out_is_sidebar);

/**
 * @brief Checks if the tab bar is currently in sidebar navigation mode.
 *
 * @param tab_bar Target tab bar.
 * @param out_is_sidebar Pointer to receive 1 if in sidebar mode, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_is_in_sidebar_mode(
    const struct cupertino_tab_bar *tab_bar, int *out_is_sidebar);

/**
 * @brief Computes layout bounds of the tab bar (either pinned bottom or left
 * sidebar).
 *
 * @param tab_bar Target tab bar.
 * @param viewport_w Total screen/window width in points.
 * @param viewport_h Total screen/window height in points.
 * @param out_x Pointer to receive top-left X coordinate in points.
 * @param out_y Pointer to receive top-left Y coordinate in points.
 * @param out_w Pointer to receive layout width in points.
 * @param out_h Pointer to receive layout height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_bar_get_layout_bounds(
    const struct cupertino_tab_bar *tab_bar, float viewport_w, float viewport_h,
    float *out_x, float *out_y, float *out_w, float *out_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_TAB_BAR_H */
