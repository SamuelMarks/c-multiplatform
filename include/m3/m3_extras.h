#ifndef M3_EXTRAS_H
#define M3_EXTRAS_H

/**
 * @file m3_extras.h
 * @brief Tooltip and badge helpers for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_layout.h"
#include "m3_text.h"

/** @brief Plain tooltip variant. */
#define M3_TOOLTIP_VARIANT_PLAIN 1
/** @brief Rich tooltip variant. */
#define M3_TOOLTIP_VARIANT_RICH 2

/** @brief Tooltip anchor is a rectangle. */
#define M3_TOOLTIP_ANCHOR_RECT 0
/** @brief Tooltip anchor is a point. */
#define M3_TOOLTIP_ANCHOR_POINT 1

/** @brief Tooltip direction: above the anchor. */
#define M3_TOOLTIP_DIRECTION_UP 0
/** @brief Tooltip direction: below the anchor. */
#define M3_TOOLTIP_DIRECTION_DOWN 1
/** @brief Tooltip direction: left of the anchor. */
#define M3_TOOLTIP_DIRECTION_LEFT 2
/** @brief Tooltip direction: right of the anchor. */
#define M3_TOOLTIP_DIRECTION_RIGHT 3

/** @brief Tooltip alignment: start edge. */
#define M3_TOOLTIP_ALIGN_START 0
/** @brief Tooltip alignment: center. */
#define M3_TOOLTIP_ALIGN_CENTER 1
/** @brief Tooltip alignment: end edge. */
#define M3_TOOLTIP_ALIGN_END 2

/** @brief Plain tooltip default horizontal padding in pixels. */
#define M3_TOOLTIP_PLAIN_DEFAULT_PADDING_X 8.0f
/** @brief Plain tooltip default vertical padding in pixels. */
#define M3_TOOLTIP_PLAIN_DEFAULT_PADDING_Y 4.0f
/** @brief Plain tooltip default minimum width in pixels. */
#define M3_TOOLTIP_PLAIN_DEFAULT_MIN_WIDTH 0.0f
/** @brief Plain tooltip default minimum height in pixels. */
#define M3_TOOLTIP_PLAIN_DEFAULT_MIN_HEIGHT 24.0f
/** @brief Plain tooltip default maximum width in pixels (0 = none). */
#define M3_TOOLTIP_PLAIN_DEFAULT_MAX_WIDTH 0.0f
/** @brief Plain tooltip default maximum height in pixels (0 = none). */
#define M3_TOOLTIP_PLAIN_DEFAULT_MAX_HEIGHT 0.0f
/** @brief Plain tooltip default corner radius in pixels. */
#define M3_TOOLTIP_PLAIN_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Plain tooltip default anchor gap in pixels. */
#define M3_TOOLTIP_PLAIN_DEFAULT_ANCHOR_GAP 4.0f

/** @brief Rich tooltip default horizontal padding in pixels. */
#define M3_TOOLTIP_RICH_DEFAULT_PADDING_X 12.0f
/** @brief Rich tooltip default vertical padding in pixels. */
#define M3_TOOLTIP_RICH_DEFAULT_PADDING_Y 8.0f
/** @brief Rich tooltip default minimum width in pixels. */
#define M3_TOOLTIP_RICH_DEFAULT_MIN_WIDTH 0.0f
/** @brief Rich tooltip default minimum height in pixels. */
#define M3_TOOLTIP_RICH_DEFAULT_MIN_HEIGHT 40.0f
/** @brief Rich tooltip default maximum width in pixels (0 = none). */
#define M3_TOOLTIP_RICH_DEFAULT_MAX_WIDTH 0.0f
/** @brief Rich tooltip default maximum height in pixels (0 = none). */
#define M3_TOOLTIP_RICH_DEFAULT_MAX_HEIGHT 0.0f
/** @brief Rich tooltip default corner radius in pixels. */
#define M3_TOOLTIP_RICH_DEFAULT_CORNER_RADIUS 8.0f
/** @brief Rich tooltip default anchor gap in pixels. */
#define M3_TOOLTIP_RICH_DEFAULT_ANCHOR_GAP 4.0f
/** @brief Rich tooltip default spacing between title and body in pixels. */
#define M3_TOOLTIP_RICH_DEFAULT_TITLE_BODY_SPACING 4.0f

/**
 * @brief Tooltip anchor descriptor.
 */
typedef struct M3TooltipAnchor {
  m3_u32 type;  /**< Anchor type (M3_TOOLTIP_ANCHOR_*). */
  M3Rect rect;  /**< Anchor rectangle (for M3_TOOLTIP_ANCHOR_RECT). */
  M3Vec2 point; /**< Anchor point (for M3_TOOLTIP_ANCHOR_POINT). */
} M3TooltipAnchor;

/**
 * @brief Tooltip placement descriptor.
 */
typedef struct M3TooltipPlacement {
  m3_u32 direction; /**< Preferred direction (M3_TOOLTIP_DIRECTION_*). */
  m3_u32 align;     /**< Alignment along the cross axis (M3_TOOLTIP_ALIGN_*). */
} M3TooltipPlacement;

/**
 * @brief Tooltip content metrics.
 */
typedef struct M3TooltipContent {
  M3TextMetrics title_metrics; /**< Title metrics (rich tooltips). */
  M3TextMetrics body_metrics;  /**< Body metrics (plain or rich tooltips). */
  M3Bool has_title;            /**< M3_TRUE when title metrics are valid. */
  M3Bool has_body;             /**< M3_TRUE when body metrics are valid. */
} M3TooltipContent;

/**
 * @brief Tooltip style descriptor.
 */
typedef struct M3TooltipStyle {
  m3_u32 variant;        /**< Tooltip variant (M3_TOOLTIP_VARIANT_*). */
  M3LayoutEdges padding; /**< Padding around tooltip content. */
  M3Scalar min_width;    /**< Minimum tooltip width in pixels (>= 0). */
  M3Scalar min_height;   /**< Minimum tooltip height in pixels (>= 0). */
  M3Scalar max_width; /**< Maximum tooltip width in pixels (>= 0, 0 = none). */
  M3Scalar
      max_height; /**< Maximum tooltip height in pixels (>= 0, 0 = none). */
  M3Scalar corner_radius; /**< Corner radius in pixels (>= 0). */
  M3Scalar anchor_gap; /**< Gap between anchor and tooltip in pixels (>= 0). */
  M3Scalar
      title_body_spacing;  /**< Spacing between title and body in pixels (>= 0).
                            */
  M3TextStyle text_style;  /**< Plain tooltip text style. */
  M3TextStyle title_style; /**< Rich tooltip title style. */
  M3TextStyle body_style;  /**< Rich tooltip body style. */
  M3Color background_color; /**< Tooltip background color. */
} M3TooltipStyle;

/**
 * @brief Initialize a plain tooltip style with defaults.
 * @param style Tooltip style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tooltip_style_init_plain(M3TooltipStyle *style);

/**
 * @brief Initialize a rich tooltip style with defaults.
 * @param style Tooltip style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tooltip_style_init_rich(M3TooltipStyle *style);

/**
 * @brief Compute the content size for a tooltip.
 * @param style Tooltip style descriptor.
 * @param content Tooltip content metrics.
 * @param out_size Receives the computed tooltip size.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tooltip_compute_content_size(
    const M3TooltipStyle *style, const M3TooltipContent *content,
    M3Size *out_size);

/**
 * @brief Compute the tooltip bounds within an overlay region.
 * @param style Tooltip style descriptor.
 * @param anchor Tooltip anchor descriptor.
 * @param placement Placement descriptor.
 * @param overlay Overlay bounds used for clamping.
 * @param tooltip_size Tooltip size to position.
 * @param out_bounds Receives the tooltip bounds.
 * @param out_direction Receives the resolved direction.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tooltip_compute_bounds(
    const M3TooltipStyle *style, const M3TooltipAnchor *anchor,
    const M3TooltipPlacement *placement, const M3Rect *overlay,
    const M3Size *tooltip_size, M3Rect *out_bounds, m3_u32 *out_direction);

/** @brief Badge corner: top-left. */
#define M3_BADGE_CORNER_TOP_LEFT 0
/** @brief Badge corner: top-right. */
#define M3_BADGE_CORNER_TOP_RIGHT 1
/** @brief Badge corner: bottom-right. */
#define M3_BADGE_CORNER_BOTTOM_RIGHT 2
/** @brief Badge corner: bottom-left. */
#define M3_BADGE_CORNER_BOTTOM_LEFT 3

/** @brief Default badge minimum size in pixels. */
#define M3_BADGE_DEFAULT_MIN_SIZE 16.0f
/** @brief Default badge dot diameter in pixels. */
#define M3_BADGE_DEFAULT_DOT_DIAMETER 6.0f
/** @brief Default badge horizontal padding in pixels. */
#define M3_BADGE_DEFAULT_PADDING_X 4.0f
/** @brief Default badge vertical padding in pixels. */
#define M3_BADGE_DEFAULT_PADDING_Y 2.0f
/** @brief Default badge corner radius in pixels. */
#define M3_BADGE_DEFAULT_CORNER_RADIUS 8.0f
/** @brief Default icon badge offset X in pixels. */
#define M3_BADGE_DEFAULT_ICON_OFFSET_X 0.0f
/** @brief Default icon badge offset Y in pixels. */
#define M3_BADGE_DEFAULT_ICON_OFFSET_Y 0.0f
/** @brief Default navigation badge offset X in pixels. */
#define M3_BADGE_DEFAULT_NAV_OFFSET_X 0.0f
/** @brief Default navigation badge offset Y in pixels. */
#define M3_BADGE_DEFAULT_NAV_OFFSET_Y 0.0f

/**
 * @brief Badge content metrics.
 */
typedef struct M3BadgeContent {
  M3TextMetrics text_metrics; /**< Text metrics for the badge count. */
  M3Bool has_text;            /**< M3_TRUE when text metrics are valid. */
} M3BadgeContent;

/**
 * @brief Badge placement descriptor.
 */
typedef struct M3BadgePlacement {
  m3_u32 corner;     /**< Anchor corner (M3_BADGE_CORNER_*). */
  M3Scalar offset_x; /**< Horizontal offset from the anchor corner. */
  M3Scalar offset_y; /**< Vertical offset from the anchor corner. */
} M3BadgePlacement;

/**
 * @brief Badge style descriptor.
 */
typedef struct M3BadgeStyle {
  M3Scalar min_size;        /**< Minimum badge size in pixels (>= 0). */
  M3Scalar dot_diameter;    /**< Dot badge diameter in pixels (>= 0). */
  M3Scalar padding_x;       /**< Horizontal padding in pixels (>= 0). */
  M3Scalar padding_y;       /**< Vertical padding in pixels (>= 0). */
  M3Scalar corner_radius;   /**< Corner radius in pixels (>= 0). */
  M3TextStyle text_style;   /**< Text style for badge counts. */
  M3Color background_color; /**< Badge background color. */
} M3BadgeStyle;

/**
 * @brief Initialize a badge style with defaults.
 * @param style Badge style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_badge_style_init(M3BadgeStyle *style);

/**
 * @brief Initialize an icon badge placement descriptor.
 * @param placement Placement descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_badge_placement_init_icon(M3BadgePlacement *placement);

/**
 * @brief Initialize a navigation badge placement descriptor.
 * @param placement Placement descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_badge_placement_init_navigation(M3BadgePlacement *placement);

/**
 * @brief Compute the badge size.
 * @param style Badge style descriptor.
 * @param content Badge content metrics.
 * @param out_size Receives the computed badge size.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_badge_compute_size(const M3BadgeStyle *style,
                                         const M3BadgeContent *content,
                                         M3Size *out_size);

/**
 * @brief Compute the badge bounds relative to an anchor rectangle.
 * @param style Badge style descriptor.
 * @param content Badge content metrics.
 * @param anchor Anchor bounds to attach to.
 * @param placement Placement descriptor.
 * @param out_bounds Receives the badge bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_badge_compute_bounds(const M3BadgeStyle *style,
                                           const M3BadgeContent *content,
                                           const M3Rect *anchor,
                                           const M3BadgePlacement *placement,
                                           M3Rect *out_bounds);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for color validation.
 * @param color Color to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_extras_test_validate_color(const M3Color *color);

/**
 * @brief Test wrapper for edge validation.
 * @param edges Edge descriptor to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_extras_test_validate_edges(const M3LayoutEdges *edges);

/**
 * @brief Test wrapper for text style validation.
 * @param style Text style to validate.
 * @param require_family M3_TRUE to require a family name.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_extras_test_validate_text_style(const M3TextStyle *style,
                                                      M3Bool require_family);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_extras_test_validate_rect(const M3Rect *rect);

/**
 * @brief Test wrapper for tooltip anchor validation.
 * @param anchor Anchor descriptor to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_extras_test_validate_tooltip_anchor(const M3TooltipAnchor *anchor);

/**
 * @brief Test wrapper for tooltip placement validation.
 * @param placement Placement descriptor to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_extras_test_validate_tooltip_placement(const M3TooltipPlacement *placement);

/**
 * @brief Test wrapper for tooltip style validation.
 * @param style Tooltip style descriptor.
 * @param require_family M3_TRUE to require a family name.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_extras_test_validate_tooltip_style(
    const M3TooltipStyle *style, M3Bool require_family);

/**
 * @brief Test wrapper for tooltip content validation.
 * @param style Tooltip style descriptor.
 * @param content Tooltip content metrics.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_extras_test_validate_tooltip_content(
    const M3TooltipStyle *style, const M3TooltipContent *content);

/**
 * @brief Test wrapper for badge style validation.
 * @param style Badge style descriptor.
 * @param require_family M3_TRUE to require a family name.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_extras_test_validate_badge_style(
    const M3BadgeStyle *style, M3Bool require_family);

/**
 * @brief Test wrapper for badge content validation.
 * @param content Badge content metrics.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_extras_test_validate_badge_content(const M3BadgeContent *content);

/**
 * @brief Test wrapper for badge placement validation.
 * @param placement Placement descriptor to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL
m3_extras_test_validate_badge_placement(const M3BadgePlacement *placement);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_EXTRAS_H */
