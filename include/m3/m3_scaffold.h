#ifndef M3_SCAFFOLD_H
#define M3_SCAFFOLD_H

/**
 * @file m3_scaffold.h
 * @brief Scaffold layout orchestrator for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_anim.h"
#include "m3_api_ui.h"
#include "m3_layout.h"

/** @brief Default scaffold horizontal padding in pixels. */
#define M3_SCAFFOLD_DEFAULT_PADDING_X 0.0f
/** @brief Default scaffold vertical padding in pixels. */
#define M3_SCAFFOLD_DEFAULT_PADDING_Y 0.0f
/** @brief Default FAB margin from safe edges in pixels. */
#define M3_SCAFFOLD_DEFAULT_FAB_MARGIN 16.0f
/** @brief Default snackbar horizontal margin in pixels. */
#define M3_SCAFFOLD_DEFAULT_SNACKBAR_MARGIN_X 16.0f
/** @brief Default snackbar vertical margin in pixels. */
#define M3_SCAFFOLD_DEFAULT_SNACKBAR_MARGIN_Y 16.0f
/** @brief Default spacing between FAB and snackbar in pixels. */
#define M3_SCAFFOLD_DEFAULT_FAB_SNACKBAR_SPACING 16.0f
/** @brief Default FAB slide animation duration in seconds. */
#define M3_SCAFFOLD_DEFAULT_FAB_SLIDE_DURATION 0.2f
/** @brief Default FAB slide easing. */
#define M3_SCAFFOLD_DEFAULT_FAB_SLIDE_EASING M3_ANIM_EASE_OUT

/**
 * @brief Scaffold style descriptor.
 */
typedef struct M3ScaffoldStyle {
  M3LayoutEdges padding; /**< Padding around the body content (>= 0). */
  M3Scalar fab_margin_x; /**< Horizontal FAB margin from safe edge (>= 0). */
  M3Scalar fab_margin_y; /**< Vertical FAB margin from safe edge (>= 0). */
  M3Scalar snackbar_margin_x; /**< Snackbar horizontal margin (>= 0). */
  M3Scalar snackbar_margin_y; /**< Snackbar vertical margin (>= 0). */
  M3Scalar
      fab_snackbar_spacing;    /**< Spacing between FAB and snackbar (>= 0). */
  M3Scalar fab_slide_duration; /**< FAB slide animation duration (>= 0). */
  m3_u32 fab_slide_easing;     /**< FAB slide easing (M3_ANIM_EASE_*). */
} M3ScaffoldStyle;

/**
 * @brief Scaffold widget instance.
 */
typedef struct M3Scaffold {
  M3Widget widget;          /**< Widget interface (points to this instance). */
  M3ScaffoldStyle style;    /**< Current scaffold style. */
  M3LayoutEdges safe_area;  /**< Safe-area insets applied to layout (>= 0). */
  M3Widget *body;           /**< Body widget (may be NULL). */
  M3Widget *top_bar;        /**< Top bar widget (may be NULL). */
  M3Widget *bottom_bar;     /**< Bottom bar widget (may be NULL). */
  M3Widget *fab;            /**< Floating action button widget (may be NULL). */
  M3Widget *snackbar;       /**< Snackbar widget (may be NULL). */
  M3Rect bounds;            /**< Assigned scaffold bounds. */
  M3Rect body_bounds;       /**< Computed body bounds. */
  M3Rect top_bar_bounds;    /**< Computed top bar bounds. */
  M3Rect bottom_bar_bounds; /**< Computed bottom bar bounds. */
  M3Rect fab_bounds;        /**< Computed FAB bounds. */
  M3Rect snackbar_bounds;   /**< Computed snackbar bounds. */
  M3AnimController fab_anim;  /**< FAB slide animation controller. */
  M3Scalar fab_offset;        /**< Current FAB offset in pixels. */
  M3Scalar fab_target_offset; /**< Target FAB offset in pixels. */
} M3Scaffold;

/**
 * @brief Initialize a scaffold style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_style_init(M3ScaffoldStyle *style);

/**
 * @brief Initialize a scaffold widget.
 * @param scaffold Scaffold instance.
 * @param style Scaffold style descriptor.
 * @param body Body widget (may be NULL).
 * @param top_bar Top bar widget (may be NULL).
 * @param bottom_bar Bottom bar widget (may be NULL).
 * @param fab Floating action button widget (may be NULL).
 * @param snackbar Snackbar widget (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_init(M3Scaffold *scaffold,
                                    const M3ScaffoldStyle *style,
                                    M3Widget *body, M3Widget *top_bar,
                                    M3Widget *bottom_bar, M3Widget *fab,
                                    M3Widget *snackbar);

/**
 * @brief Update the scaffold style.
 * @param scaffold Scaffold instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_set_style(M3Scaffold *scaffold,
                                         const M3ScaffoldStyle *style);

/**
 * @brief Assign the body widget.
 * @param scaffold Scaffold instance.
 * @param body Body widget (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_set_body(M3Scaffold *scaffold, M3Widget *body);

/**
 * @brief Assign the top bar widget.
 * @param scaffold Scaffold instance.
 * @param top_bar Top bar widget (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_set_top_bar(M3Scaffold *scaffold,
                                           M3Widget *top_bar);

/**
 * @brief Assign the bottom bar widget.
 * @param scaffold Scaffold instance.
 * @param bottom_bar Bottom bar widget (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_set_bottom_bar(M3Scaffold *scaffold,
                                              M3Widget *bottom_bar);

/**
 * @brief Assign the floating action button widget.
 * @param scaffold Scaffold instance.
 * @param fab Floating action button widget (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_set_fab(M3Scaffold *scaffold, M3Widget *fab);

/**
 * @brief Assign the snackbar widget.
 * @param scaffold Scaffold instance.
 * @param snackbar Snackbar widget (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_set_snackbar(M3Scaffold *scaffold,
                                            M3Widget *snackbar);

/**
 * @brief Update the safe-area insets.
 * @param scaffold Scaffold instance.
 * @param safe_area Safe-area inset descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_set_safe_area(M3Scaffold *scaffold,
                                             const M3LayoutEdges *safe_area);

/**
 * @brief Retrieve the current safe-area insets.
 * @param scaffold Scaffold instance.
 * @param out_safe_area Receives the safe-area inset descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_get_safe_area(const M3Scaffold *scaffold,
                                             M3LayoutEdges *out_safe_area);

/**
 * @brief Step scaffold animations.
 * @param scaffold Scaffold instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_changed Receives M3_TRUE when visual state changed.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scaffold_step(M3Scaffold *scaffold, M3Scalar dt,
                                    M3Bool *out_changed);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_SCAFFOLD_H */
