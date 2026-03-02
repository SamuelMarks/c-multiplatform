#ifndef M3_TOOLTIP_H
#define M3_TOOLTIP_H

/**
 * @file m3_tooltip.h
 * @brief Tooltip widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_extras.h"
#include "cmpc/cmp_text.h"
#include "cmpc/cmp_visuals.h"

struct M3Tooltip;

/**
 * @brief Tooltip widget instance.
 */
typedef struct M3Tooltip {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend
      text_backend;      /**< Text backend for font creation and measurement. */
  CMPTooltipStyle style; /**< Tooltip style descriptor. */
  CMPTooltipAnchor anchor;       /**< Anchor descriptor for positioning. */
  CMPTooltipPlacement placement; /**< Placement descriptor. */

  CMPRect overlay;            /**< The overlay bounds to constrain tooltip. */
  CMPRect bounds;             /**< Resolved tooltip bounds. */
  cmp_u32 resolved_direction; /**< Resolved tooltip direction. */

  CMPHandle plain_font; /**< Font handle for plain tooltips. */
  CMPHandle title_font; /**< Font handle for rich tooltips (title). */
  CMPHandle body_font;  /**< Font handle for rich tooltips (body). */

  const char *utf8_title; /**< UTF-8 title text (may be NULL). */
  cmp_usize title_len;    /**< UTF-8 title length in bytes. */

  const char *utf8_body; /**< UTF-8 body text. */
  cmp_usize body_len;    /**< UTF-8 body length in bytes. */

  CMPBool owns_fonts;        /**< CMP_TRUE when the widget owns font handles. */
  CMPBool metrics_valid;     /**< CMP_TRUE when content metrics are valid. */
  CMPTooltipContent metrics; /**< Computed content metrics. */
} M3Tooltip;

/**
 * @brief Initialize a plain tooltip style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tooltip_style_init_plain(CMPTooltipStyle *style);

/**
 * @brief Initialize a rich tooltip style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tooltip_style_init_rich(CMPTooltipStyle *style);

/**
 * @brief Initialize a tooltip widget.
 * @param tooltip Tooltip instance.
 * @param text_backend Text backend interface.
 * @param style Tooltip style descriptor.
 * @param anchor Tooltip anchor descriptor.
 * @param placement Tooltip placement descriptor.
 * @param overlay Overlay bounds for placement constraint.
 * @param utf8_body UTF-8 body text (cannot be NULL).
 * @param body_len UTF-8 body text length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tooltip_init(M3Tooltip *tooltip,
                                     const CMPTextBackend *text_backend,
                                     const CMPTooltipStyle *style,
                                     const CMPTooltipAnchor *anchor,
                                     const CMPTooltipPlacement *placement,
                                     CMPRect overlay, const char *utf8_body,
                                     cmp_usize body_len);

/**
 * @brief Update the tooltip title text (for rich tooltips).
 * @param tooltip Tooltip instance.
 * @param utf8_title UTF-8 title text (may be NULL).
 * @param title_len UTF-8 title text length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tooltip_set_title(M3Tooltip *tooltip,
                                          const char *utf8_title,
                                          cmp_usize title_len);

/**
 * @brief Update the tooltip overlay constraints.
 * @param tooltip Tooltip instance.
 * @param overlay Overlay bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tooltip_set_overlay(M3Tooltip *tooltip,
                                            CMPRect overlay);

/**
 * @brief Test wrapper for tooltip metrics calculation.
 * @param tooltip Tooltip instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tooltip_test_metrics_update(M3Tooltip *tooltip);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_TOOLTIP_H */