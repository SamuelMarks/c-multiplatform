#ifndef F2_TOOLTIP_H
#define F2_TOOLTIP_H

/**
 * @file f2_tooltip.h
 * @brief Microsoft Fluent 2 Tooltip widgets.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "f2/f2_color.h"
/* clang-format on */

/** @brief Default tooltip corner radius. */
#define F2_TOOLTIP_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Default tooltip padding. */
#define F2_TOOLTIP_DEFAULT_PADDING 8.0f
/** @brief Tooltip minimum delay in seconds. */
#define F2_TOOLTIP_DEFAULT_DELAY_SECS 0.5f

/**
 * @brief Fluent 2 Tooltip style descriptor.
 */
typedef struct F2TooltipStyle {
  CMPColor background_color; /**< Tooltip fill color. */
  CMPColor border_color;     /**< Border stroke color. */
  CMPColor text_color;       /**< Text color. */
  CMPTextStyle text_style;   /**< Typography for the text. */
  CMPScalar corner_radius;   /**< Corner radius. */
  CMPScalar padding_x;       /**< Horizontal content padding. */
  CMPScalar padding_y;       /**< Vertical content padding. */
  CMPScalar border_width;    /**< Border thickness. */
} F2TooltipStyle;

/**
 * @brief Fluent 2 Tooltip widget instance.
 */
typedef struct F2Tooltip {
  CMPWidget widget;      /**< Widget interface. */
  F2TooltipStyle style;  /**< Current style. */
  const char *utf8_text; /**< Optional tooltip text. */
  CMPRect bounds;        /**< Layout bounds. */
  CMPRect anchor_bounds; /**< Bounds of the anchor element. */
  CMPBool is_visible;    /**< True if currently open/visible. */
} F2Tooltip;

/**
 * @brief Initialize a default Fluent 2 tooltip style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tooltip_style_init(F2TooltipStyle *style);

/**
 * @brief Initialize a Fluent 2 tooltip widget.
 * @param tooltip Tooltip instance.
 * @param style Style descriptor.
 * @param utf8_text Text content to show on hover.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tooltip_init(F2Tooltip *tooltip,
                                     const F2TooltipStyle *style,
                                     const char *utf8_text);

/**
 * @brief Show the tooltip anchored to a specific rect.
 * @param tooltip Tooltip instance.
 * @param anchor_bounds Target area to point to.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tooltip_show(F2Tooltip *tooltip, CMPRect anchor_bounds);

/**
 * @brief Hide the tooltip.
 * @param tooltip Tooltip instance.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_tooltip_hide(F2Tooltip *tooltip);

#ifdef __cplusplus
}
#endif

#endif /* F2_TOOLTIP_H */
