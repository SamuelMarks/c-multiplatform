#ifndef M3_BADGE_H
#define M3_BADGE_H

/**
 * @file m3_badge.h
 * @brief Material 3 Badge widget for notifications and status.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_text.h"

/** @brief Default dot badge diameter in pixels. */
#define M3_BADGE_DEFAULT_DOT_DIAMETER 6.0f
/** @brief Default labeled badge minimum width in pixels. */
#define M3_BADGE_DEFAULT_MIN_WIDTH 16.0f
/** @brief Default labeled badge height in pixels. */
#define M3_BADGE_DEFAULT_HEIGHT 16.0f
/** @brief Default labeled badge horizontal padding in pixels. */
#define M3_BADGE_DEFAULT_PADDING_X 4.0f

/**
 * @brief Badge style descriptor.
 */
typedef struct M3BadgeStyle {
  CMPColor background_color; /**< Badge fill color. */
  CMPColor text_color;       /**< Label text color. */
  CMPTextStyle text_style;   /**< Text style for labeled badges. */
  CMPScalar dot_diameter;    /**< Diameter for a dot badge (> 0). */
  CMPScalar min_width;       /**< Min width for a labeled badge (> 0). */
  CMPScalar height;          /**< Height for a labeled badge (> 0). */
  CMPScalar padding_x; /**< Horizontal padding for labeled badges (>= 0). */
} M3BadgeStyle;

/**
 * @brief Badge widget instance.
 */
typedef struct M3Badge {
  CMPWidget widget;            /**< Base widget object. */
  M3BadgeStyle style;          /**< Current badge style. */
  CMPTextBackend text_backend; /**< Text backend for measurement/rendering. */
  const char *utf8_label;      /**< UTF-8 label text (may be NULL for a dot). */
  cmp_usize utf8_len;          /**< Length of the label in bytes. */
  CMPRect bounds;              /**< Computed layout bounds. */
  CMPHandle font;              /**< Font handle for label text. */
  CMPTextMetrics metrics;      /**< Cached text metrics. */
} M3Badge;

/**
 * @brief Initialize a badge style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_badge_style_init(M3BadgeStyle *style);

/**
 * @brief Initialize a badge widget.
 * @param badge Badge instance.
 * @param backend Text backend instance (needed if label provided).
 * @param style Badge style descriptor.
 * @param utf8_label UTF-8 label text (NULL for a dot badge).
 * @param utf8_len Length of the label in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_badge_init(M3Badge *badge,
                                   const CMPTextBackend *backend,
                                   const M3BadgeStyle *style,
                                   const char *utf8_label, cmp_usize utf8_len);

/**
 * @brief Update the badge label. If NULL, converts to a dot badge.
 * @param badge Badge instance.
 * @param utf8_label UTF-8 label text (NULL for a dot badge).
 * @param utf8_len Length of the label in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_badge_set_label(M3Badge *badge, const char *utf8_label,
                                        cmp_usize utf8_len);

/**
 * @brief Update the badge style.
 * @param badge Badge instance.
 * @param style New badge style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_badge_set_style(M3Badge *badge,
                                        const M3BadgeStyle *style);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BADGE_H */
