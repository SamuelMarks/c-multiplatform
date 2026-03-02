#ifndef M3_DIVIDER_H
#define M3_DIVIDER_H

/**
 * @file m3_divider.h
 * @brief Material 3 Divider widget.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_layout.h"

/** @brief Default divider thickness in pixels. */
#define M3_DIVIDER_DEFAULT_THICKNESS 1.0f

/**
 * @brief Divider style descriptor.
 */
typedef struct M3DividerStyle {
  cmp_u32 direction;     /**< Direction (CMP_LAYOUT_DIRECTION_ROW or COLUMN). */
  CMPColor color;        /**< Divider color. */
  CMPScalar thickness;   /**< Divider thickness in pixels (>= 0). */
  CMPScalar inset_start; /**< Leading inset in pixels (>= 0). */
  CMPScalar inset_end;   /**< Trailing inset in pixels (>= 0). */
} M3DividerStyle;

/**
 * @brief Divider widget instance.
 */
typedef struct M3Divider {
  CMPWidget widget;     /**< Base widget object. */
  M3DividerStyle style; /**< Current divider style. */
  CMPRect bounds;       /**< Computed layout bounds. */
} M3Divider;

/**
 * @brief Initialize a divider style with defaults.
 * @param style Divider style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_divider_style_init(M3DividerStyle *style);

/**
 * @brief Initialize a divider widget.
 * @param divider Divider instance.
 * @param style Divider style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_divider_init(M3Divider *divider,
                                     const M3DividerStyle *style);

/**
 * @brief Update the divider style.
 * @param divider Divider instance.
 * @param style New divider style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_divider_set_style(M3Divider *divider,
                                          const M3DividerStyle *style);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_DIVIDER_H */
