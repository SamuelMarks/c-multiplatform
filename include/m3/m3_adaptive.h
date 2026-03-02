#ifndef M3_ADAPTIVE_H
#define M3_ADAPTIVE_H

/**
 * @file m3_adaptive.h
 * @brief Adaptive layouts and window size classes for Material 3.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_api_ui.h"

/** @brief Window size class: Compact. */
#define M3_WINDOW_CLASS_COMPACT 0
/** @brief Window size class: Medium. */
#define M3_WINDOW_CLASS_MEDIUM 1
/** @brief Window size class: Expanded. */
#define M3_WINDOW_CLASS_EXPANDED 2

/** @brief Adaptive layout properties. */
typedef struct M3AdaptiveLayout {
  cmp_u32 window_class;     /**< Current window size class. */
  CMPScalar safe_width;     /**< Safe area width available. */
  CMPScalar safe_height;    /**< Safe area height available. */
  CMPWidget *primary_pane;  /**< Primary pane widget (e.g. list). */
  CMPWidget *secondary_pane;/**< Secondary pane widget (e.g. detail). */
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
CMP_API int CMP_CALL m3_adaptive_list_detail_measure(
    const M3AdaptiveLayout *layout, CMPScalar *out_width, CMPScalar *out_height);

/**
 * @brief Evaluates an adaptive layout bounds resolution logic.
 * @param layout Layout configuration.
 * @param bounds Bounds for the layout.
 * @param out_primary Receives bounds for primary pane.
 * @param out_secondary Receives bounds for secondary pane.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_adaptive_list_detail_layout(
    const M3AdaptiveLayout *layout, CMPRect bounds,
    CMPRect *out_primary, CMPRect *out_secondary);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_ADAPTIVE_H */