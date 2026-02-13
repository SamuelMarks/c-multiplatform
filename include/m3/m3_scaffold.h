#ifndef M3_SCAFFOLD_H
#define M3_SCAFFOLD_H

/**
 * @file m3_scaffold.h
 * @brief Scaffold layout orchestrator for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_layout.h"

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
#define M3_SCAFFOLD_DEFAULT_FAB_SLIDE_EASING CMP_ANIM_EASE_OUT

/**
 * @brief Scaffold style descriptor.
 */
typedef struct M3ScaffoldStyle {
  CMPLayoutEdges padding; /**< Padding around the body content (>= 0). */
  CMPScalar fab_margin_x; /**< Horizontal FAB margin from safe edge (>= 0). */
  CMPScalar fab_margin_y; /**< Vertical FAB margin from safe edge (>= 0). */
  CMPScalar snackbar_margin_x; /**< Snackbar horizontal margin (>= 0). */
  CMPScalar snackbar_margin_y; /**< Snackbar vertical margin (>= 0). */
  CMPScalar
      fab_snackbar_spacing;    /**< Spacing between FAB and snackbar (>= 0). */
  CMPScalar fab_slide_duration; /**< FAB slide animation duration (>= 0). */
  cmp_u32 fab_slide_easing;     /**< FAB slide easing (CMP_ANIM_EASE_*). */
} M3ScaffoldStyle;

/**
 * @brief Scaffold widget instance.
 */
typedef struct M3Scaffold {
  CMPWidget widget;          /**< Widget interface (points to this instance). */
  M3ScaffoldStyle style;    /**< Current scaffold style. */
  CMPLayoutEdges safe_area;  /**< Safe-area insets applied to layout (>= 0). */
  CMPWidget *body;           /**< Body widget (may be NULL). */
  CMPWidget *top_bar;        /**< Top bar widget (may be NULL). */
  CMPWidget *bottom_bar;     /**< Bottom bar widget (may be NULL). */
  CMPWidget *fab;            /**< Floating action button widget (may be NULL). */
  CMPWidget *snackbar;       /**< Snackbar widget (may be NULL). */
  CMPRect bounds;            /**< Assigned scaffold bounds. */
  CMPRect body_bounds;       /**< Computed body bounds. */
  CMPRect top_bar_bounds;    /**< Computed top bar bounds. */
  CMPRect bottom_bar_bounds; /**< Computed bottom bar bounds. */
  CMPRect fab_bounds;        /**< Computed FAB bounds. */
  CMPRect snackbar_bounds;   /**< Computed snackbar bounds. */
  CMPAnimController fab_anim;  /**< FAB slide animation controller. */
  CMPScalar fab_offset;        /**< Current FAB offset in pixels. */
  CMPScalar fab_target_offset; /**< Target FAB offset in pixels. */
} M3Scaffold;

/**
 * @brief Initialize a scaffold style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_style_init(M3ScaffoldStyle *style);

/**
 * @brief Initialize a scaffold widget.
 * @param scaffold Scaffold instance.
 * @param style Scaffold style descriptor.
 * @param body Body widget (may be NULL).
 * @param top_bar Top bar widget (may be NULL).
 * @param bottom_bar Bottom bar widget (may be NULL).
 * @param fab Floating action button widget (may be NULL).
 * @param snackbar Snackbar widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_init(M3Scaffold *scaffold,
                                    const M3ScaffoldStyle *style,
                                    CMPWidget *body, CMPWidget *top_bar,
                                    CMPWidget *bottom_bar, CMPWidget *fab,
                                    CMPWidget *snackbar);

/**
 * @brief Update the scaffold style.
 * @param scaffold Scaffold instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_set_style(M3Scaffold *scaffold,
                                         const M3ScaffoldStyle *style);

/**
 * @brief Assign the body widget.
 * @param scaffold Scaffold instance.
 * @param body Body widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_set_body(M3Scaffold *scaffold, CMPWidget *body);

/**
 * @brief Assign the top bar widget.
 * @param scaffold Scaffold instance.
 * @param top_bar Top bar widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_set_top_bar(M3Scaffold *scaffold,
                                           CMPWidget *top_bar);

/**
 * @brief Assign the bottom bar widget.
 * @param scaffold Scaffold instance.
 * @param bottom_bar Bottom bar widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_set_bottom_bar(M3Scaffold *scaffold,
                                              CMPWidget *bottom_bar);

/**
 * @brief Assign the floating action button widget.
 * @param scaffold Scaffold instance.
 * @param fab Floating action button widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_set_fab(M3Scaffold *scaffold, CMPWidget *fab);

/**
 * @brief Assign the snackbar widget.
 * @param scaffold Scaffold instance.
 * @param snackbar Snackbar widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_set_snackbar(M3Scaffold *scaffold,
                                            CMPWidget *snackbar);

/**
 * @brief Update the safe-area insets.
 * @param scaffold Scaffold instance.
 * @param safe_area Safe-area inset descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_set_safe_area(M3Scaffold *scaffold,
                                             const CMPLayoutEdges *safe_area);

/**
 * @brief Retrieve the current safe-area insets.
 * @param scaffold Scaffold instance.
 * @param out_safe_area Receives the safe-area inset descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_get_safe_area(const M3Scaffold *scaffold,
                                             CMPLayoutEdges *out_safe_area);

/**
 * @brief Step scaffold animations.
 * @param scaffold Scaffold instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_changed Receives CMP_TRUE when visual state changed.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scaffold_step(M3Scaffold *scaffold, CMPScalar dt,
                                    CMPBool *out_changed);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_SCAFFOLD_H */
