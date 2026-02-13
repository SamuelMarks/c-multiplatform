#ifndef M3_SHEET_H
#define M3_SHEET_H

/**
 * @file m3_sheet.h
 * @brief Bottom sheet widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_visuals.h"

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
 * @param action Action identifier (CMP_SHEET_ACTION_*).
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPSheetOnAction)(void *ctx, struct M3Sheet *sheet,
                                      cmp_u32 action);

/**
 * @brief Bottom sheet style descriptor.
 */
typedef struct M3SheetStyle {
  cmp_u32 variant;        /**< Sheet variant (CMP_SHEET_VARIANT_*). */
  CMPLayoutEdges padding; /**< Padding around sheet contents. */
  CMPScalar height;       /**< Preferred sheet height in pixels (>= 0). */
  CMPScalar min_height;   /**< Minimum sheet height in pixels (>= 0). */
  CMPScalar max_height; /**< Maximum sheet height in pixels (>= 0, 0 = none). */
  CMPScalar max_width;  /**< Maximum sheet width in pixels (>= 0, 0 = none). */
  CMPScalar corner_radius; /**< Sheet corner radius in pixels (>= 0). */
  CMPScalar
      dismiss_threshold; /**< Fraction of height needed to dismiss (0..1). */
  CMPScalar min_fling_velocity; /**< Minimum downward fling velocity for dismiss
                                  (>= 0). */
  CMPScalar spring_stiffness;   /**< Spring stiffness (> 0). */
  CMPScalar spring_damping;     /**< Spring damping (>= 0). */
  CMPScalar spring_mass;        /**< Spring mass (> 0). */
  CMPScalar spring_tolerance;   /**< Spring rest position tolerance (>= 0). */
  CMPScalar spring_rest_velocity; /**< Spring rest velocity tolerance (>= 0). */
  CMPColor background_color;      /**< Sheet background color. */
  CMPColor scrim_color;           /**< Scrim color used behind the sheet. */
  CMPShadow shadow;               /**< Shadow descriptor. */
  CMPBool shadow_enabled; /**< CMP_TRUE when shadow rendering is enabled. */
  CMPBool scrim_enabled;  /**< CMP_TRUE when scrim rendering is enabled. */
} M3SheetStyle;

/**
 * @brief Bottom sheet widget instance.
 */
typedef struct M3Sheet {
  CMPWidget widget;       /**< Widget interface (points to this instance). */
  M3SheetStyle style;    /**< Current sheet style. */
  CMPRect overlay_bounds; /**< Bounds of the overlay region. */
  CMPRect sheet_bounds;   /**< Bounds of the sheet at rest (open state). */
  CMPScalar offset;       /**< Current vertical offset from the open state. */
  CMPScalar drag_start_offset; /**< Offset at the start of a drag gesture. */
  CMPBool dragging;            /**< CMP_TRUE when a drag gesture is active. */
  CMPBool open;                /**< CMP_TRUE when the sheet is open. */
  CMPAnimController anim;      /**< Spring animation controller. */
  CMPSheetOnAction on_action;  /**< Action callback (may be NULL). */
  void *on_action_ctx;        /**< Action callback context pointer. */
} M3Sheet;

/**
 * @brief Initialize a standard bottom sheet style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_style_init_standard(M3SheetStyle *style);

/**
 * @brief Initialize a modal bottom sheet style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_style_init_modal(M3SheetStyle *style);

/**
 * @brief Initialize a bottom sheet widget.
 * @param sheet Sheet instance.
 * @param style Sheet style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_init(M3Sheet *sheet, const M3SheetStyle *style);

/**
 * @brief Update the sheet style.
 * @param sheet Sheet instance.
 * @param style New sheet style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_set_style(M3Sheet *sheet,
                                      const M3SheetStyle *style);

/**
 * @brief Assign an action callback to the sheet.
 * @param sheet Sheet instance.
 * @param on_action Action callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_set_on_action(M3Sheet *sheet,
                                          CMPSheetOnAction on_action, void *ctx);

/**
 * @brief Set the open state of the sheet.
 * @param sheet Sheet instance.
 * @param open CMP_TRUE to open, CMP_FALSE to close.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_set_open(M3Sheet *sheet, CMPBool open);

/**
 * @brief Retrieve the open state of the sheet.
 * @param sheet Sheet instance.
 * @param out_open Receives CMP_TRUE when open.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_get_open(const M3Sheet *sheet, CMPBool *out_open);

/**
 * @brief Step sheet animations.
 * @param sheet Sheet instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_changed Receives CMP_TRUE when visual state changed.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_step(M3Sheet *sheet, CMPScalar dt,
                                 CMPBool *out_changed);

/**
 * @brief Retrieve the current sheet bounds.
 * @param sheet Sheet instance.
 * @param out_bounds Receives the current sheet bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_get_bounds(const M3Sheet *sheet,
                                       CMPRect *out_bounds);

/**
 * @brief Compute content bounds inside the sheet.
 * @param sheet Sheet instance.
 * @param out_bounds Receives the content bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_sheet_get_content_bounds(const M3Sheet *sheet,
                                               CMPRect *out_bounds);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_SHEET_H */
