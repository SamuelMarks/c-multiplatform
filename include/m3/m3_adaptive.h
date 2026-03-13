#ifndef M3_ADAPTIVE_H
#define M3_ADAPTIVE_H

/**
 * @file m3_adaptive.h
 * @brief Adaptive layouts and window size classes for Material 3.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
/* clang-format on */

/** @brief Window size class: Compact. */
#define M3_WINDOW_CLASS_COMPACT 0
/** @brief Window size class: Medium. */
#define M3_WINDOW_CLASS_MEDIUM 1
/** @brief Window size class: Expanded. */
#define M3_WINDOW_CLASS_EXPANDED 2
/** @brief Window size class: Extra Large. */
#define M3_WINDOW_CLASS_EXTRA_LARGE 3

/** @brief Foldable posture: Flat (no hinge or fully open). */
#define M3_POSTURE_FLAT 0
/** @brief Foldable posture: Half-opened like a book (vertical hinge). */
#define M3_POSTURE_HALF_OPENED_BOOK 1
/** @brief Foldable posture: Half-opened like a laptop (horizontal hinge). */
#define M3_POSTURE_HALF_OPENED_TABLETOP 2

/** @brief Foldable hinge metrics. */
typedef struct M3FoldableHinge {
  CMPRect bounds;  /**< Bounds of the hinge in the window. */
  cmp_u32 posture; /**< Current posture of the device. */
  CMPBool
      is_separating; /**< CMP_TRUE if the hinge visually separates content. */
} M3FoldableHinge;

/** @brief Adaptive layout properties. */
typedef struct M3AdaptiveLayout {
  cmp_u32 window_class;      /**< Current window size class. */
  CMPScalar safe_width;      /**< Safe area width available. */
  CMPScalar safe_height;     /**< Safe area height available. */
  CMPWidget *primary_pane;   /**< Primary pane widget (e.g. list). */
  CMPWidget *secondary_pane; /**< Secondary pane widget (e.g. detail). */
  CMPWidget *tertiary_pane;  /**< Optional tertiary pane widget. */
  M3FoldableHinge hinge;     /**< Hinge information (if any). */
} M3AdaptiveLayout;

/**
 * @brief Evaluates window width to determine the Material 3 size class.
 * @param width Window width in pixels.
 * @param out_class Receives the size class (M3_WINDOW_CLASS_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_window_size_class(CMPScalar width, cmp_u32 *out_class);

/**
 * @brief Evaluates an adaptive layout measure logic.
 * @param layout Layout configuration.
 * @param out_width Receives the desired total width.
 * @param out_height Receives the desired total height.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_adaptive_list_detail_measure(const M3AdaptiveLayout *layout,
                                CMPScalar *out_width, CMPScalar *out_height);

/**
 * @brief Evaluates an adaptive layout bounds resolution logic (List/Detail).
 * @param layout Layout configuration.
 * @param bounds Bounds for the layout.
 * @param out_primary Receives bounds for primary pane.
 * @param out_secondary Receives bounds for secondary pane.
 * @param is_rtl CMP_TRUE if layout should be right-to-left.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_adaptive_list_detail_layout(
    const M3AdaptiveLayout *layout, CMPRect bounds, CMPRect *out_primary,
    CMPRect *out_secondary, CMPBool is_rtl);

/**
 * @brief Evaluates an adaptive feed layout bounds resolution logic.
 * @param layout Layout configuration.
 * @param bounds Bounds for the layout.
 * @param out_primary Receives bounds for primary feed pane.
 * @param out_secondary Receives bounds for secondary support pane.
 * @param is_rtl CMP_TRUE if layout should be right-to-left.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_adaptive_feed_layout(const M3AdaptiveLayout *layout,
                                             CMPRect bounds,
                                             CMPRect *out_primary,
                                             CMPRect *out_secondary,
                                             CMPBool is_rtl);

/**
 * @brief Evaluates an adaptive supporting pane layout bounds resolution logic.
 * @param layout Layout configuration.
 * @param bounds Bounds for the layout.
 * @param out_primary Receives bounds for primary pane.
 * @param out_secondary Receives bounds for secondary supporting pane.
 * @param is_rtl CMP_TRUE if layout should be right-to-left.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_adaptive_supporting_pane_layout(
    const M3AdaptiveLayout *layout, CMPRect bounds, CMPRect *out_primary,
    CMPRect *out_secondary, CMPBool is_rtl);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_ADAPTIVE_H */