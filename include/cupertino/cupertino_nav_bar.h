/**
 * @file cupertino_nav_bar.h
 * @brief Cupertino Navigation Bar component conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_NAV_BAR_H
#define CUPERTINO_CUPERTINO_NAV_BAR_H

/* clang-format off */
#include "ui_top_app_bar_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;
struct cupertino_search_bar;

/**
 * @struct cupertino_nav_bar_descriptor
 * @brief Configuration descriptor for creating a Cupertino Navigation Bar.
 */
struct cupertino_nav_bar_descriptor {
  const char *title;          /**< Navigation bar center/large title. */
  const char *prompt;         /**< Optional prompt subtitle above title. */
  const char *previous_title; /**< Title of previous view for back button. */
  int is_large_title_enabled; /**< 1 for large collapsible title, 0 inline. */
};

/**
 * @struct cupertino_nav_bar
 * @brief Cupertino Navigation Bar instance wrapping ui_top_app_bar_base.
 */
struct cupertino_nav_bar {
  struct ui_top_app_bar_base *base; /**< Wrapped CDK top app bar primitive. */
  struct ui_arena *arena;           /**< Memory arena for base component. */
  char title[128];                  /**< Current navigation title. */
  char prompt[128];                 /**< Optional prompt text. */
  char previous_title[128];         /**< Back button destination title. */
  int is_large_title_enabled;       /**< Collapsible large title flag. */
  float scroll_offset;              /**< Current scroll offset in pt. */
  float current_height;             /**< Dynamically interpolated height. */
  float collapse_progress;          /**< 0.0 (expanded) to 1.0 (collapsed). */
  float inline_title_opacity;       /**< Opacity of collapsed center title. */
  float large_title_opacity;        /**< Opacity of large expanded title. */
  struct cupertino_search_bar *search_bar; /**< Attached search controller. */
  int has_search_controller;       /**< 1 if search controller attached. */
  int hides_search_when_scrolling; /**< 1 if search collapses on scroll. */
  float search_collapse_progress;  /**< Search collapse progress. */
  float search_bar_opacity;        /**< Search bar opacity [0.0, 1.0]. */
  float title_scale_factor;        /**< Scale factor [0.7, 1.0]. */
};

/**
 * @brief Creates a new Cupertino navigation bar instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_bar Pointer to receive newly created navigation bar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_nav_bar_create(
    struct ui_engine *engine, const struct cupertino_nav_bar_descriptor *desc,
    struct cupertino_nav_bar **out_bar);

/**
 * @brief Destroys a Cupertino navigation bar and frees all allocated resources.
 *
 * @param bar Navigation bar instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_destroy(struct cupertino_nav_bar *bar);

/**
 * @brief Updates the main title text.
 *
 * @param bar Navigation bar instance.
 * @param title New title string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_set_title(struct cupertino_nav_bar *bar, const char *title);

/**
 * @brief Retrieves the current title text.
 *
 * @param bar Navigation bar instance.
 * @param out_title Pointer to receive const pointer to title string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_nav_bar_get_title(
    const struct cupertino_nav_bar *bar, const char **out_title);

/**
 * @brief Sets or clears the prompt banner subtitle text.
 *
 * @param bar Navigation bar instance.
 * @param prompt New prompt string, or NULL to clear.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_set_prompt(struct cupertino_nav_bar *bar, const char *prompt);

/**
 * @brief Retrieves the current prompt text.
 *
 * @param bar Navigation bar instance.
 * @param out_prompt Pointer to receive const pointer to prompt string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_nav_bar_get_prompt(
    const struct cupertino_nav_bar *bar, const char **out_prompt);

/**
 * @brief Sets the title of the previous view for back button display.
 *
 * @param bar Navigation bar instance.
 * @param prev_title Previous view title, or NULL for default.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_nav_bar_set_previous_title(
    struct cupertino_nav_bar *bar, const char *prev_title);

/**
 * @brief Retrieves the previous view title.
 *
 * @param bar Navigation bar instance.
 * @param out_prev_title Pointer to receive const pointer to previous title.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_nav_bar_get_previous_title(
    const struct cupertino_nav_bar *bar, const char **out_prev_title);

/**
 * @brief Enables or disables Large Title mode.
 *
 * @param bar Navigation bar instance.
 * @param enabled 1 to enable large collapsible titles, 0 for standard inline.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_set_large_title_enabled(struct cupertino_nav_bar *bar,
                                          int enabled);

/**
 * @brief Queries whether Large Title mode is enabled.
 *
 * @param bar Navigation bar instance.
 * @param out_enabled Pointer to receive 1 if enabled, 0 if disabled.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_is_large_title_enabled(const struct cupertino_nav_bar *bar,
                                         int *out_enabled);

/**
 * @brief Feeds scroll events into the navigation bar to update title collapse.
 *
 * @param bar Navigation bar instance.
 * @param scroll_y Absolute vertical scroll offset.
 * @param delta_y Delta vertical movement.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_nav_bar_handle_scroll(
    struct cupertino_nav_bar *bar, float scroll_y, float delta_y);

/**
 * @brief Retrieves current computed height of the navigation bar.
 *
 * @param bar Navigation bar instance.
 * @param out_height Pointer to receive height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_nav_bar_get_height(
    const struct cupertino_nav_bar *bar, float *out_height);

/**
 * @brief Retrieves current collapse progress (0.0 fully expanded, 1.0
 * collapsed).
 *
 * @param bar Navigation bar instance.
 * @param out_progress Pointer to receive collapse progress in [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_get_collapse_progress(const struct cupertino_nav_bar *bar,
                                        float *out_progress);

/**
 * @brief Resolves the effective back button title according to Apple HIG rules.
 *
 * If the previous view's title fits in available width, it is used.
 * If truncated/constrained, it collapses to "Back". If width is < 40pt,
 * it collapses to empty string (chevron glyph only).
 *
 * @param bar Navigation bar instance.
 * @param available_width Available width for the back button in points.
 * @param out_buf Buffer to receive resolved back button text.
 * @param buf_len Size of out_buf in bytes.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_get_effective_back_title(const struct cupertino_nav_bar *bar,
                                           float available_width, char *out_buf,
                                           size_t buf_len);

/**
 * @brief Retrieves the underlying CDK top app bar primitive.
 *
 * @param bar Navigation bar instance.
 * @param out_base Pointer to receive ui_top_app_bar_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_nav_bar_get_base(
    struct cupertino_nav_bar *bar, struct ui_top_app_bar_base **out_base);

/**
 * @brief Attaches an integrated search controller to the navigation bar.
 *
 * @param bar Navigation bar instance.
 * @param search_bar Search bar instance (or NULL to detach).
 * @param hides_when_scrolling 1 if search bar collapses into nav bar on scroll
 * down.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_set_search_controller(struct cupertino_nav_bar *bar,
                                        struct cupertino_search_bar *search_bar,
                                        int hides_when_scrolling);

/**
 * @brief Retrieves the attached search controller.
 *
 * @param bar Navigation bar instance.
 * @param out_search_bar Pointer to receive search bar pointer.
 * @param out_hides_when_scrolling Pointer to receive hides_when_scrolling flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_get_search_controller(
    const struct cupertino_nav_bar *bar,
    struct cupertino_search_bar **out_search_bar,
    int *out_hides_when_scrolling);

/**
 * @brief Retrieves search controller collapse progress and opacity.
 *
 * @param bar Navigation bar instance.
 * @param out_progress Pointer to receive collapse progress [0.0, 1.0].
 * @param out_opacity Pointer to receive search bar opacity [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_nav_bar_get_search_collapse_progress(
    const struct cupertino_nav_bar *bar, float *out_progress,
    float *out_opacity);

/**
 * @brief Computes large title graceful scale factor to prevent truncation.
 *
 * @param bar Navigation bar instance.
 * @param available_width Available container width in points.
 * @param out_scale_factor Pointer to receive computed scale factor [0.7, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_nav_bar_compute_title_scale(
    const struct cupertino_nav_bar *bar, float available_width,
    float *out_scale_factor);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_NAV_BAR_H */
