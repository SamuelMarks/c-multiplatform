#ifndef F2_FLYOUT_H
#define F2_FLYOUT_H

/**
 * @file f2_flyout.h
 * @brief Microsoft Fluent 2 Flyout / Popover widgets.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "f2/f2_color.h"
/* clang-format on */

/** @brief Default flyout corner radius. */
#define F2_FLYOUT_DEFAULT_CORNER_RADIUS 8.0f
/** @brief Default flyout padding. */
#define F2_FLYOUT_DEFAULT_PADDING 16.0f

/**
 * @brief Fluent 2 Flyout style descriptor.
 */
typedef struct F2FlyoutStyle {
  CMPColor background_color; /**< Flyout fill color (supports Acrylic). */
  CMPColor border_color;     /**< Border color. */
  CMPScalar corner_radius;   /**< Corner radius. */
  CMPScalar padding;         /**< Content padding. */
  CMPScalar border_width;    /**< Border thickness. */
  CMPBool has_beak;          /**< True if drawing a beak (pointer). */
} F2FlyoutStyle;

/**
 * @brief Fluent 2 Flyout widget instance.
 */
typedef struct F2Flyout {
  CMPWidget widget;      /**< Widget interface. */
  F2FlyoutStyle style;   /**< Current style. */
  CMPRect bounds;        /**< Layout bounds. */
  CMPRect anchor_bounds; /**< Bounds of the anchor element. */
  CMPBool is_open;       /**< True if currently open/visible. */
} F2Flyout;

/**
 * @brief Initialize a default Fluent 2 flyout style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_flyout_style_init(F2FlyoutStyle *style);

/**
 * @brief Initialize a Fluent 2 flyout widget.
 * @param flyout Flyout instance.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_flyout_init(F2Flyout *flyout,
                                    const F2FlyoutStyle *style);

/**
 * @brief Show the flyout anchored to a specific rect.
 * @param flyout Flyout instance.
 * @param anchor_bounds Target area to point to.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_flyout_show(F2Flyout *flyout, CMPRect anchor_bounds);

/**
 * @brief Hide the flyout.
 * @param flyout Flyout instance.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_flyout_hide(F2Flyout *flyout);

#ifdef __cplusplus
}
#endif

#endif /* F2_FLYOUT_H */
