#ifndef M3_SHEET_H
#define M3_SHEET_H

/**
 * @file m3_sheet.h
 * @brief Bottom sheet widgets for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_ui.h"
#include "m3_layout.h"
#include "m3_visuals.h"

/** @brief Standard bottom sheet variant. */
#define M3_SHEET_VARIANT_STANDARD 1
/** @brief Modal bottom sheet variant. */
#define M3_SHEET_VARIANT_MODAL 2

/** @brief Bottom sheet action: dismissed. */
#define M3_SHEET_ACTION_DISMISS 1u

/** @brief Default sheet height in pixels. */
#define M3_SHEET_DEFAULT_HEIGHT 256.0f
/** @brief Default minimum sheet height in pixels. */
#define M3_SHEET_DEFAULT_MIN_HEIGHT 64.0f
/** @brief Default maximum sheet height in pixels (0 = no limit). */
#define M3_SHEET_DEFAULT_MAX_HEIGHT 0.0f
/** @brief Default maximum sheet width in pixels (0 = no limit). */
#define M3_SHEET_DEFAULT_MAX_WIDTH 0.0f
/** @brief Default sheet horizontal padding in pixels. */
#define M3_SHEET_DEFAULT_PADDING_X 24.0f
/** @brief Default sheet vertical padding in pixels. */
#define M3_SHEET_DEFAULT_PADDING_Y 24.0f
/** @brief Default sheet corner radius in pixels. */
#define M3_SHEET_DEFAULT_CORNER_RADIUS 28.0f
/** @brief Default dismiss threshold fraction (0..1). */
#define M3_SHEET_DEFAULT_DISMISS_THRESHOLD 0.5f
/** @brief Default minimum fling velocity in pixels per second. */
#define M3_SHEET_DEFAULT_MIN_FLING_VELOCITY 1000.0f
/** @brief Default spring stiffness for sheet animations. */
#define M3_SHEET_DEFAULT_SPRING_STIFFNESS 200.0f
/** @brief Default spring damping for sheet animations. */
#define M3_SHEET_DEFAULT_SPRING_DAMPING 25.0f
/** @brief Default spring mass for sheet animations. */
#define M3_SHEET_DEFAULT_SPRING_MASS 1.0f
/** @brief Default spring tolerance for sheet animations. */
#define M3_SHEET_DEFAULT_SPRING_TOLERANCE 0.5f
/** @brief Default spring rest velocity for sheet animations. */
#define M3_SHEET_DEFAULT_SPRING_REST_VELOCITY 1.0f

struct M3Sheet;

/**
 * @brief Bottom sheet action callback signature.
 * @param ctx User callback context pointer.
 * @param sheet Bottom sheet instance that triggered the action.
 * @param action Action identifier (M3_SHEET_ACTION_*).
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3SheetOnAction)(void *ctx, struct M3Sheet *sheet,
                                      m3_u32 action);

/**
 * @brief Bottom sheet style descriptor.
 */
typedef struct M3SheetStyle {
  m3_u32 variant;        /**< Sheet variant (M3_SHEET_VARIANT_*). */
  M3LayoutEdges padding; /**< Padding around sheet contents. */
  M3Scalar height;       /**< Preferred sheet height in pixels (>= 0). */
  M3Scalar min_height;   /**< Minimum sheet height in pixels (>= 0). */
  M3Scalar max_height; /**< Maximum sheet height in pixels (>= 0, 0 = none). */
  M3Scalar max_width;  /**< Maximum sheet width in pixels (>= 0, 0 = none). */
  M3Scalar corner_radius; /**< Sheet corner radius in pixels (>= 0). */
  M3Scalar
      dismiss_threshold; /**< Fraction of height needed to dismiss (0..1). */
  M3Scalar min_fling_velocity; /**< Minimum downward fling velocity for dismiss
                                  (>= 0). */
  M3Scalar spring_stiffness;   /**< Spring stiffness (> 0). */
  M3Scalar spring_damping;     /**< Spring damping (>= 0). */
  M3Scalar spring_mass;        /**< Spring mass (> 0). */
  M3Scalar spring_tolerance;   /**< Spring rest position tolerance (>= 0). */
  M3Scalar spring_rest_velocity; /**< Spring rest velocity tolerance (>= 0). */
  M3Color background_color;      /**< Sheet background color. */
  M3Color scrim_color;           /**< Scrim color used behind the sheet. */
  M3Shadow shadow;               /**< Shadow descriptor. */
  M3Bool shadow_enabled; /**< M3_TRUE when shadow rendering is enabled. */
  M3Bool scrim_enabled;  /**< M3_TRUE when scrim rendering is enabled. */
} M3SheetStyle;

/**
 * @brief Bottom sheet widget instance.
 */
typedef struct M3Sheet {
  M3Widget widget;       /**< Widget interface (points to this instance). */
  M3SheetStyle style;    /**< Current sheet style. */
  M3Rect overlay_bounds; /**< Bounds of the overlay region. */
  M3Rect sheet_bounds;   /**< Bounds of the sheet at rest (open state). */
  M3Scalar offset;       /**< Current vertical offset from the open state. */
  M3Scalar drag_start_offset; /**< Offset at the start of a drag gesture. */
  M3Bool dragging;            /**< M3_TRUE when a drag gesture is active. */
  M3Bool open;                /**< M3_TRUE when the sheet is open. */
  M3AnimController anim;      /**< Spring animation controller. */
  M3SheetOnAction on_action;  /**< Action callback (may be NULL). */
  void *on_action_ctx;        /**< Action callback context pointer. */
} M3Sheet;

/**
 * @brief Initialize a standard bottom sheet style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_style_init_standard(M3SheetStyle *style);

/**
 * @brief Initialize a modal bottom sheet style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_style_init_modal(M3SheetStyle *style);

/**
 * @brief Initialize a bottom sheet widget.
 * @param sheet Sheet instance.
 * @param style Sheet style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_init(M3Sheet *sheet, const M3SheetStyle *style);

/**
 * @brief Update the sheet style.
 * @param sheet Sheet instance.
 * @param style New sheet style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_set_style(M3Sheet *sheet,
                                      const M3SheetStyle *style);

/**
 * @brief Assign an action callback to the sheet.
 * @param sheet Sheet instance.
 * @param on_action Action callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_set_on_action(M3Sheet *sheet,
                                          M3SheetOnAction on_action, void *ctx);

/**
 * @brief Set the open state of the sheet.
 * @param sheet Sheet instance.
 * @param open M3_TRUE to open, M3_FALSE to close.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_set_open(M3Sheet *sheet, M3Bool open);

/**
 * @brief Retrieve the open state of the sheet.
 * @param sheet Sheet instance.
 * @param out_open Receives M3_TRUE when open.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_get_open(const M3Sheet *sheet, M3Bool *out_open);

/**
 * @brief Step sheet animations.
 * @param sheet Sheet instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_changed Receives M3_TRUE when visual state changed.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_step(M3Sheet *sheet, M3Scalar dt,
                                 M3Bool *out_changed);

/**
 * @brief Retrieve the current sheet bounds.
 * @param sheet Sheet instance.
 * @param out_bounds Receives the current sheet bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_get_bounds(const M3Sheet *sheet,
                                       M3Rect *out_bounds);

/**
 * @brief Compute content bounds inside the sheet.
 * @param sheet Sheet instance.
 * @param out_bounds Receives the content bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_sheet_get_content_bounds(const M3Sheet *sheet,
                                               M3Rect *out_bounds);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_SHEET_H */
