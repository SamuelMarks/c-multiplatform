/**
 * @file cupertino_split_view.h
 * @brief Cupertino Navigation Split View (UISplitViewController /
 * NavigationSplitView) conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_SPLIT_VIEW_H
#define CUPERTINO_CUPERTINO_SPLIT_VIEW_H

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include "ui_split_pane_base.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_SPLIT_VIEW_MIN_SIDEBAR_WIDTH 200.0f
#define CUPERTINO_SPLIT_VIEW_DEFAULT_SIDEBAR_WIDTH 260.0f
#define CUPERTINO_SPLIT_VIEW_MAX_SIDEBAR_WIDTH 360.0f

#define CUPERTINO_SPLIT_VIEW_MIN_CONTENT_WIDTH 220.0f
#define CUPERTINO_SPLIT_VIEW_DEFAULT_CONTENT_WIDTH 300.0f
#define CUPERTINO_SPLIT_VIEW_MAX_CONTENT_WIDTH 400.0f

#define CUPERTINO_SPLIT_VIEW_COMPACT_BREAKPOINT 768.0f

/**
 * @enum cupertino_split_view_style
 * @brief Column hierarchy layout configuration.
 */
enum cupertino_split_view_style {
  CUPERTINO_SPLIT_VIEW_TWO_COLUMN = 0, /**< Sidebar + Detail layout. */
  CUPERTINO_SPLIT_VIEW_THREE_COLUMN =
      1 /**< Sidebar + Content + Detail layout. */
};

/**
 * @enum cupertino_split_view_display_mode
 * @brief Active display presentation of columns.
 */
enum cupertino_split_view_display_mode {
  CUPERTINO_SPLIT_VIEW_AUTOMATIC = 0,      /**< Adapts to available width. */
  CUPERTINO_SPLIT_VIEW_SECONDARY_ONLY = 1, /**< Shows only detail view. */
  CUPERTINO_SPLIT_VIEW_ONE_BESIDE_SECONDARY =
      2, /**< Shows one leading column beside detail. */
  CUPERTINO_SPLIT_VIEW_TWO_BESIDE_SECONDARY =
      3 /**< Shows all columns side-by-side. */
};

/**
 * @struct cupertino_split_view_descriptor
 * @brief Configuration descriptor for creating a Cupertino Navigation Split
 * View.
 */
struct cupertino_split_view_descriptor {
  enum cupertino_split_view_style style; /**< Column style. */
  enum cupertino_split_view_display_mode
      display_mode;      /**< Initial display mode. */
  float sidebar_width;   /**< Initial sidebar width (or <=0 for default). */
  float content_width;   /**< Initial content width for 3-column (or <=0 for
                            default). */
  float viewport_width;  /**< Initial viewport width in points. */
  float viewport_height; /**< Initial viewport height in points. */
};

/**
 * @struct cupertino_split_view
 * @brief Cupertino Navigation Split View instance wrapping ui_split_pane_base.
 */
struct cupertino_split_view {
  struct ui_split_pane_base *base;       /**< CDK split pane primitive. */
  enum cupertino_split_view_style style; /**< Two-column vs Three-column. */
  enum cupertino_split_view_display_mode display_mode; /**< Display mode. */
  float sidebar_width;   /**< Current sidebar width in points. */
  float content_width;   /**< Current content column width in points. */
  float viewport_width;  /**< Current viewport width. */
  float viewport_height; /**< Current viewport height. */
  int is_collapsed; /**< Non-zero if collapsed into compact single stack. */
  int show_sidebar; /**< Non-zero if sidebar column visible. */
  int show_content; /**< Non-zero if content column visible. */
  int show_detail;  /**< Non-zero if detail column visible. */
};

/**
 * @brief Creates a new Cupertino Navigation Split View.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_view Pointer to receive newly created split view.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_split_view_create(struct ui_engine *engine,
                            const struct cupertino_split_view_descriptor *desc,
                            struct cupertino_split_view **out_view);

/**
 * @brief Destroys a Cupertino Navigation Split View.
 *
 * @param view Split view instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_split_view_destroy(struct cupertino_split_view *view);

/**
 * @brief Sets column style (Two-Column vs Three-Column).
 *
 * @param view Target split view.
 * @param style New column style.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_set_style(
    struct cupertino_split_view *view, enum cupertino_split_view_style style);

/**
 * @brief Gets current column style.
 *
 * @param view Target split view.
 * @param out_style Pointer to receive column style.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_split_view_get_style(const struct cupertino_split_view *view,
                               enum cupertino_split_view_style *out_style);

/**
 * @brief Sets display mode.
 *
 * @param view Target split view.
 * @param mode New display mode.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_set_display_mode(
    struct cupertino_split_view *view,
    enum cupertino_split_view_display_mode mode);

/**
 * @brief Gets current display mode.
 *
 * @param view Target split view.
 * @param out_mode Pointer to receive display mode.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_get_display_mode(
    const struct cupertino_split_view *view,
    enum cupertino_split_view_display_mode *out_mode);

/**
 * @brief Sets sidebar column width (clamped to [MIN, MAX] boundaries).
 *
 * @param view Target split view.
 * @param width Desired width in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_set_sidebar_width(
    struct cupertino_split_view *view, float width);

/**
 * @brief Gets current sidebar width in points.
 *
 * @param view Target split view.
 * @param out_width Pointer to receive sidebar width.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_get_sidebar_width(
    const struct cupertino_split_view *view, float *out_width);

/**
 * @brief Sets content column width for three-column layout.
 *
 * @param view Target split view.
 * @param width Desired width in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_set_content_width(
    struct cupertino_split_view *view, float width);

/**
 * @brief Gets current content column width.
 *
 * @param view Target split view.
 * @param out_width Pointer to receive content column width.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_get_content_width(
    const struct cupertino_split_view *view, float *out_width);

/**
 * @brief Updates viewport size and recalculates auto-collapse and column
 * visibilities.
 *
 * @param view Target split view.
 * @param width Viewport width in points (must be >= 0.0f).
 * @param height Viewport height in points (must be >= 0.0f).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_set_viewport_size(
    struct cupertino_split_view *view, float width, float height);

/**
 * @brief Gets current viewport size.
 *
 * @param view Target split view.
 * @param out_width Pointer to receive width in points.
 * @param out_height Pointer to receive height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_split_view_get_viewport_size(const struct cupertino_split_view *view,
                                       float *out_width, float *out_height);

/**
 * @brief Sets manual collapsed state override.
 *
 * @param view Target split view.
 * @param collapsed Non-zero to collapse to single stack, 0 to uncollapse.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_set_collapsed(
    struct cupertino_split_view *view, int collapsed);

/**
 * @brief Checks if split view is currently collapsed into a compact single
 * stack.
 *
 * @param view Target split view.
 * @param out_collapsed Pointer to receive collapsed state flag (1 or 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_is_collapsed(
    const struct cupertino_split_view *view, int *out_collapsed);

/**
 * @brief Gets active visibility of all three columns.
 *
 * @param view Target split view.
 * @param out_sidebar Pointer to receive sidebar visibility (1 or 0).
 * @param out_content Pointer to receive content visibility (1 or 0).
 * @param out_detail Pointer to receive detail visibility (1 or 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_split_view_get_column_visibility(
    const struct cupertino_split_view *view, int *out_sidebar, int *out_content,
    int *out_detail);

/**
 * @brief Forwards drag and pointer events to interactive divider.
 *
 * @param view Target split view.
 * @param event Input event to process.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_process_event(
    struct cupertino_split_view *view, const struct ui_event *event);

/**
 * @brief Retrieves underlying CDK split pane base primitive.
 *
 * @param view Target split view.
 * @param out_base Pointer to receive ui_split_pane_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_split_view_get_base(
    struct cupertino_split_view *view, struct ui_split_pane_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SPLIT_VIEW_H */
