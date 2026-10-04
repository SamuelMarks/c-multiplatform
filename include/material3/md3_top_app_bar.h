/**
 * @file md3_top_app_bar.h
 * @brief Material 3 Top App Bar component.
 */
#ifndef MATERIAL3_MD3_TOP_APP_BAR_H
#define MATERIAL3_MD3_TOP_APP_BAR_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_top_app_bar_base.h"
#include <stddef.h>
/* clang-format on */

struct ui_engine;
struct ui_arena;
struct md3_top_app_bar;

/**
 * @brief Material 3 Top App Bar visual variants.
 */
enum md3_top_app_bar_variant {
  MD3_TOP_APP_BAR_SMALL,
  MD3_TOP_APP_BAR_CENTER_ALIGNED,
  MD3_TOP_APP_BAR_MEDIUM,
  MD3_TOP_APP_BAR_LARGE
};

/**
 * @brief Material 3 Top App Bar scroll behaviors.
 */
enum md3_top_app_bar_scroll_behavior {
  MD3_TOP_APP_BAR_SCROLL_PINNED,
  MD3_TOP_APP_BAR_SCROLL_ENTER_ALWAYS,
  MD3_TOP_APP_BAR_SCROLL_EXIT_UNTIL_COLLAPSED
};

/**
 * @brief Configuration for creating a Material 3 Top App Bar.
 */
struct md3_top_app_bar_config {
  enum md3_top_app_bar_variant variant;
  enum md3_top_app_bar_scroll_behavior scroll_behavior;
  const char *title;
  const char *subtitle;
};

/**
 * @brief Creates a Material 3 Top App Bar component.
 *
 * @param engine Pointer to ui_engine.
 * @param config The configuration for the top app bar.
 * @param out_bar Output pointer for the created component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_top_app_bar_create(
    struct ui_engine *engine, const struct md3_top_app_bar_config *config,
    struct md3_top_app_bar **out_bar);

/**
 * @brief Destroys a Material 3 Top App Bar component.
 *
 * @param bar The component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_top_app_bar_destroy(struct md3_top_app_bar *bar);

/**
 * @brief Handles a scroll event.
 *
 * @param bar The component.
 * @param scroll_y The absolute scroll position.
 * @param delta_y The change in scroll position.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_top_app_bar_handle_scroll(
    struct md3_top_app_bar *bar, float scroll_y, float delta_y);

/**
 * @brief Gets the underlying base top app bar.
 *
 * @param bar The material top app bar component.
 * @param out_base Output pointer for the base component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_top_app_bar_get_base(
    struct md3_top_app_bar *bar, struct ui_top_app_bar_base **out_base);

struct md3_icon_button;

/**
 * @brief Sets the navigation icon for the top app bar.
 *
 * @param bar The top app bar component.
 * @param icon The icon button to set, or NULL to remove.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_top_app_bar_set_navigation_icon(
    struct md3_top_app_bar *bar, struct md3_icon_button *icon);

/**
 * @brief Adds an action item to the top app bar.
 *
 * @param bar The top app bar component.
 * @param action The icon button action to add.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_top_app_bar_add_action_item(
    struct md3_top_app_bar *bar, struct md3_icon_button *action);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_TOP_APP_BAR_H */
