/**
 * @file md3_shell_layout.h
 * @brief Material 3 & Expressive Shell, Layout, and Workflow Components.
 *
 * Provides spec-compliant Material 3 implementations for:
 * - md3_loading_indicator (wrapping ui_progress_base)
 * - md3_stepper & md3_step (wrapping ui_stepper_base)
 * - md3_scaffold (wrapping ui_scaffold_base)
 * - md3_canonical_layout (wrapping ui_canonical_layout_base)
 * - md3_breadcrumbs (wrapping ui_breadcrumbs_base)
 * - md3_page_indicator (wrapping ui_page_control_base)
 */

#ifndef MATERIAL3_MD3_SHELL_LAYOUT_H
#define MATERIAL3_MD3_SHELL_LAYOUT_H

/* clang-format off */
#include "ui_arena.h"
#include "ui_breadcrumbs_base.h"
#include "ui_canonical_layout_base.h"
#include "ui_component.h"
#include "ui_error.h"
#include "ui_page_control_base.h"
#include "ui_progress_base.h"
#include "ui_scaffold_base.h"
#include "ui_stepper_base.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/* ========================================================================= */
/* md3_loading_indicator                                                     */
/* ========================================================================= */

/**
 * @enum md3_loading_indicator_style
 * @brief Container presentation style for Material 3 Loading Indicator.
 */
enum md3_loading_indicator_style {
  MD3_LOADING_INDICATOR_CONTAINED = 0,
  MD3_LOADING_INDICATOR_UNCONTAINED = 1
};

/**
 * @enum md3_loading_indicator_size
 * @brief Size dimensions for Material 3 Loading Indicator.
 */
enum md3_loading_indicator_size {
  MD3_LOADING_INDICATOR_SIZE_COMPACT = 0,  /**< 24dp */
  MD3_LOADING_INDICATOR_SIZE_STANDARD = 1, /**< 48dp */
  MD3_LOADING_INDICATOR_SIZE_LARGE = 2     /**< 72dp */
};

/**
 * @struct md3_loading_indicator
 * @brief Material 3 Expressive Loading Indicator wrapping ui_progress_base.
 */
struct md3_loading_indicator {
  struct ui_progress_base *base;
  enum md3_loading_indicator_style style;
  enum md3_loading_indicator_size size;
  float progress;
  int shape_stage;
  int is_reduced_motion;
  float pulse_opacity;
};

/**
 * @brief Creates a Material 3 Expressive Loading Indicator.
 *
 * @param engine Pointer to ui_engine instance.
 * @param style Contained or uncontained visual style.
 * @param out_indicator Pointer to receive allocated indicator.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_loading_indicator_create(
    struct ui_engine *engine, enum md3_loading_indicator_style style,
    struct md3_loading_indicator **out_indicator);

/**
 * @brief Destroys a Material 3 Loading Indicator.
 *
 * @param indicator Loading indicator to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_loading_indicator_destroy(struct md3_loading_indicator *indicator);

/**
 * @brief Sets indicator size (Compact, Standard, Large).
 *
 * @param indicator Loading indicator instance.
 * @param size Target size enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_loading_indicator_set_size(struct md3_loading_indicator *indicator,
                               enum md3_loading_indicator_size size);

/**
 * @brief Sets reduced motion mode for loading indicator.
 *
 * @param indicator Loading indicator instance.
 * @param enabled 1 for reduced motion (opacity pulsing), 0 for shape morphing.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_loading_indicator_set_reduced_motion(
    struct md3_loading_indicator *indicator, int enabled);

/**
 * @brief Advances loading indicator animation state by elapsed time.
 *
 * @param indicator Loading indicator instance.
 * @param elapsed_s Elapsed time in seconds.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_loading_indicator_update(
    struct md3_loading_indicator *indicator, float elapsed_s);

/* ========================================================================= */
/* md3_stepper & md3_step                                                    */
/* ========================================================================= */

/**
 * @enum md3_stepper_orientation
 * @brief Direction layout for stepper workflow.
 */
enum md3_stepper_orientation {
  MD3_STEPPER_HORIZONTAL = 0,
  MD3_STEPPER_VERTICAL = 1
};

/**
 * @struct md3_step
 * @brief Step configuration item inside a Material 3 stepper.
 */
struct md3_step {
  char step_id[64];
  char title[128];
  char subtitle[128];
  enum ui_stepper_step_state state;
  int is_editable;
};

/**
 * @struct md3_stepper
 * @brief Material 3 Stepper wrapping ui_stepper_base.
 */
struct md3_stepper {
  struct ui_stepper_base *base;
  enum ui_stepper_mode mode;
  enum md3_stepper_orientation orientation;
  struct md3_step steps[32];
  size_t step_count;
  int active_index;
  ui_stepper_validate_t validate_hook;
  void *validate_user_data;
};

/**
 * @brief Creates a Material 3 Stepper.
 *
 * @param engine Pointer to ui_engine instance.
 * @param mode Progression mode (linear or non-linear).
 * @param orientation Horizontal or vertical layout orientation.
 * @param out_stepper Pointer to receive allocated stepper.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_stepper_create(
    struct ui_engine *engine, enum ui_stepper_mode mode,
    enum md3_stepper_orientation orientation, struct md3_stepper **out_stepper);

/**
 * @brief Destroys a Material 3 Stepper.
 *
 * @param stepper Stepper instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_stepper_destroy(struct md3_stepper *stepper);

/**
 * @brief Adds a step to the stepper workflow.
 *
 * @param stepper Stepper instance.
 * @param step_id Unique step identifier string.
 * @param title Step title headline text.
 * @param subtitle Optional step subtitle supporting text.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_stepper_add_step(struct md3_stepper *stepper, const char *step_id,
                     const char *title, const char *subtitle);

/**
 * @brief Sets step state (Default, Active, Completed, Error).
 *
 * @param stepper Stepper instance.
 * @param step_index Step 0-based index.
 * @param state State enum to set.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_stepper_set_step_state(struct md3_stepper *stepper, int step_index,
                           enum ui_stepper_step_state state);

/**
 * @brief Gets step state.
 *
 * @param stepper Stepper instance.
 * @param step_index Step 0-based index.
 * @param out_state Pointer to receive step state.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_stepper_get_step_state(const struct md3_stepper *stepper, int step_index,
                           enum ui_stepper_step_state *out_state);

/**
 * @brief Sets currently active step index.
 *
 * @param stepper Stepper instance.
 * @param index Target active index.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_stepper_set_active_index(struct md3_stepper *stepper, int index);

/**
 * @brief Gets currently active step index.
 *
 * @param stepper Stepper instance.
 * @param out_index Pointer to receive active index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_stepper_get_active_index(const struct md3_stepper *stepper, int *out_index);

/**
 * @brief Advances to next step adhering to validation constraints.
 *
 * @param stepper Stepper instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_stepper_next_step(struct md3_stepper *stepper);

/**
 * @brief Navigates back to previous step.
 *
 * @param stepper Stepper instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_stepper_prev_step(struct md3_stepper *stepper);

/**
 * @brief Sets step validation hook.
 *
 * @param stepper Stepper instance.
 * @param hook Validation function pointer.
 * @param user_data User data passed to hook.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_stepper_set_validate_hook(
    struct md3_stepper *stepper, ui_stepper_validate_t hook, void *user_data);

/**
 * @brief Sets whether a specific step is editable when revisited.
 *
 * @param stepper Stepper instance.
 * @param step_index Step 0-based index.
 * @param editable 1 if editable, 0 if locked.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_stepper_set_editable(
    struct md3_stepper *stepper, int step_index, int editable);

/* ========================================================================= */
/* md3_scaffold                                                              */
/* ========================================================================= */

/**
 * @struct md3_scaffold
 * @brief Material 3 Application Scaffold wrapping ui_scaffold_base.
 */
struct md3_scaffold {
  struct ui_scaffold_base *base;
  struct ui_component *top_bar;
  struct ui_component *bottom_bar;
  struct ui_component *side_nav;
  struct ui_component *main_content;
  struct ui_component *fab;
  int fab_cradle_docked;
  float safe_area_top;
  float safe_area_bottom;
  float safe_area_left;
  float safe_area_right;
  float scroll_offset_y;
  float top_bar_elevation;
  float bottom_bar_elevation;
};

/**
 * @brief Creates a Material 3 Application Scaffold.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_scaffold Pointer to receive allocated scaffold.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_scaffold_create(
    struct ui_engine *engine, struct md3_scaffold **out_scaffold);

/**
 * @brief Destroys a Material 3 Scaffold.
 *
 * @param scaffold Scaffold instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_scaffold_destroy(struct md3_scaffold *scaffold);

/**
 * @brief Sets top app bar component slot.
 *
 * @param scaffold Scaffold instance.
 * @param top_bar Top bar component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_scaffold_set_top_bar(
    struct md3_scaffold *scaffold, struct ui_component *top_bar);

/**
 * @brief Sets bottom bar component slot.
 *
 * @param scaffold Scaffold instance.
 * @param bottom_bar Bottom bar component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_scaffold_set_bottom_bar(
    struct md3_scaffold *scaffold, struct ui_component *bottom_bar);

/**
 * @brief Sets side navigation component slot.
 *
 * @param scaffold Scaffold instance.
 * @param side_nav Side navigation component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_scaffold_set_side_nav(
    struct md3_scaffold *scaffold, struct ui_component *side_nav);

/**
 * @brief Sets main scrollable content component slot.
 *
 * @param scaffold Scaffold instance.
 * @param content Main content component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_scaffold_set_main_content(
    struct md3_scaffold *scaffold, struct ui_component *content);

/**
 * @brief Sets floating action button component and cradle docking.
 *
 * @param scaffold Scaffold instance.
 * @param fab FAB component.
 * @param cradle_docked 1 to dock into bottom app bar cradle, 0 for floating.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_scaffold_set_fab(
    struct md3_scaffold *scaffold, struct ui_component *fab, int cradle_docked);

/**
 * @brief Sets display safe area insets.
 *
 * @param scaffold Scaffold instance.
 * @param top Top safe area padding in dp.
 * @param bottom Bottom safe area padding in dp.
 * @param left Left safe area padding in dp.
 * @param right Right safe area padding in dp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_scaffold_set_safe_area(struct md3_scaffold *scaffold, float top,
                           float bottom, float left, float right);

/**
 * @brief Coordinates scroll offset elevation for app bars.
 *
 * @param scaffold Scaffold instance.
 * @param scroll_offset_y Current vertical scroll position in dp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_scaffold_on_scroll(struct md3_scaffold *scaffold, float scroll_offset_y);

/* ========================================================================= */
/* md3_canonical_layout                                                      */
/* ========================================================================= */

/**
 * @enum md3_canonical_layout_type
 * @brief Material 3 Large-screen Canonical Layout patterns.
 */
enum md3_canonical_layout_type {
  MD3_CANONICAL_LAYOUT_LIST_DETAIL = 0,
  MD3_CANONICAL_LAYOUT_SUPPORTING_PANE = 1,
  MD3_CANONICAL_LAYOUT_FEED = 2
};

/**
 * @struct md3_canonical_layout
 * @brief Material 3 Canonical Layout wrapping ui_canonical_layout_base.
 */
struct md3_canonical_layout {
  struct ui_canonical_layout_base *base;
  struct ui_arena *arena;
  enum md3_canonical_layout_type type;
  enum ui_window_size_class size_class;
  struct ui_component *body;
  struct ui_component *leading_pane;
  struct ui_component *trailing_pane;
  float split_ratio;
  int feed_columns;
};

/**
 * @brief Creates a Material 3 Canonical Layout.
 *
 * @param engine Pointer to ui_engine instance.
 * @param type Canonical layout pattern (List-Detail, Supporting Pane, Feed).
 * @param out_layout Pointer to receive allocated layout.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_canonical_layout_create(
    struct ui_engine *engine, enum md3_canonical_layout_type type,
    struct md3_canonical_layout **out_layout);

/**
 * @brief Destroys a Material 3 Canonical Layout.
 *
 * @param layout Layout instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_canonical_layout_destroy(struct md3_canonical_layout *layout);

/**
 * @brief Updates window size class and recalculates pane distribution.
 *
 * @param layout Layout instance.
 * @param size_class Compact, Medium, or Expanded size class.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_canonical_layout_set_size_class(
    struct md3_canonical_layout *layout, enum ui_window_size_class size_class);

/**
 * @brief Gets current window size class.
 *
 * @param layout Layout instance.
 * @param out_size_class Pointer to receive size class enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_canonical_layout_get_size_class(const struct md3_canonical_layout *layout,
                                    enum ui_window_size_class *out_size_class);

/**
 * @brief Sets central body content component.
 *
 * @param layout Layout instance.
 * @param body Component to place in central body.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_canonical_layout_set_body(
    struct md3_canonical_layout *layout, struct ui_component *body);

/**
 * @brief Sets leading pane component (e.g. master list in list-detail).
 *
 * @param layout Layout instance.
 * @param leading Component to place in leading pane.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_canonical_layout_set_leading_pane(
    struct md3_canonical_layout *layout, struct ui_component *leading);

/**
 * @brief Sets trailing pane component (e.g. supporting auxiliary pane).
 *
 * @param layout Layout instance.
 * @param trailing Component to place in trailing pane.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_canonical_layout_set_trailing_pane(
    struct md3_canonical_layout *layout, struct ui_component *trailing);

/**
 * @brief Sets split proportion ratio between panes (e.g. 0.4f for 40/60).
 *
 * @param layout Layout instance.
 * @param ratio Split proportion ratio between 0.1f and 0.9f.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_canonical_layout_set_split_ratio(
    struct md3_canonical_layout *layout, float ratio);

/* ========================================================================= */
/* md3_breadcrumbs                                                           */
/* ========================================================================= */

/**
 * @struct md3_breadcrumb_item
 * @brief Breadcrumb link destination item.
 */
struct md3_breadcrumb_item {
  char label[64];
  char href[128];
};

/**
 * @struct md3_breadcrumbs
 * @brief Material 3 Breadcrumbs trail wrapping ui_breadcrumbs_base.
 */
struct md3_breadcrumbs {
  struct ui_breadcrumbs_base *base;
  struct md3_breadcrumb_item items[32];
  size_t item_count;
  size_t max_items;
  char separator[16];
  size_t active_index;
};

/**
 * @brief Creates a Material 3 Breadcrumbs navigation component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_breadcrumbs Pointer to receive allocated breadcrumbs.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_breadcrumbs_create(
    struct ui_engine *engine, struct md3_breadcrumbs **out_breadcrumbs);

/**
 * @brief Destroys a Material 3 Breadcrumbs instance.
 *
 * @param breadcrumbs Breadcrumbs instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_breadcrumbs_destroy(struct md3_breadcrumbs *breadcrumbs);

/**
 * @brief Appends a path item to the breadcrumbs trail.
 *
 * @param breadcrumbs Breadcrumbs instance.
 * @param label Text display label.
 * @param href Destination URL or route path.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_breadcrumbs_add_item(
    struct md3_breadcrumbs *breadcrumbs, const char *label, const char *href);

/**
 * @brief Sets custom separator glyph (e.g. ">", "/", "•").
 *
 * @param breadcrumbs Breadcrumbs instance.
 * @param separator Separator glyph string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_breadcrumbs_set_separator(
    struct md3_breadcrumbs *breadcrumbs, const char *separator);

/**
 * @brief Sets maximum visible items before middle truncation collapses.
 *
 * @param breadcrumbs Breadcrumbs instance.
 * @param max_items Maximum items limit (minimum 2).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_breadcrumbs_set_max_items(
    struct md3_breadcrumbs *breadcrumbs, size_t max_items);

/**
 * @brief Simulates clicking a breadcrumb item by index.
 *
 * @param breadcrumbs Breadcrumbs instance.
 * @param index 0-based index of item clicked.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_breadcrumbs_simulate_click(
    struct md3_breadcrumbs *breadcrumbs, size_t index);

/* ========================================================================= */
/* md3_page_indicator                                                        */
/* ========================================================================= */

/**
 * @struct md3_page_indicator
 * @brief Material 3 Page Indicator wrapping ui_page_control_base.
 */
struct md3_page_indicator {
  struct ui_page_control_base *base;
  int page_count;
  int current_page;
  float pill_width;
};

/**
 * @brief Creates a Material 3 Page Indicator.
 *
 * @param engine Pointer to ui_engine instance.
 * @param page_count Total number of pages.
 * @param out_indicator Pointer to receive allocated indicator.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_page_indicator_create(struct ui_engine *engine, int page_count,
                          struct md3_page_indicator **out_indicator);

/**
 * @brief Destroys a Material 3 Page Indicator.
 *
 * @param indicator Page indicator instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_page_indicator_destroy(struct md3_page_indicator *indicator);

/**
 * @brief Sets total page count.
 *
 * @param indicator Page indicator instance.
 * @param page_count Number of pages (positive integer).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_page_indicator_set_page_count(
    struct md3_page_indicator *indicator, int page_count);

/**
 * @brief Sets active page index.
 *
 * @param indicator Page indicator instance.
 * @param current_page Active 0-based page index.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_page_indicator_set_current_page(
    struct md3_page_indicator *indicator, int current_page);

/**
 * @brief Gets active page index.
 *
 * @param indicator Page indicator instance.
 * @param out_page Pointer to receive active page index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_page_indicator_get_current_page(
    const struct md3_page_indicator *indicator, int *out_page);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_SHELL_LAYOUT_H */
