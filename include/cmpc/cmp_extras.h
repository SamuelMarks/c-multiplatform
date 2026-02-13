#ifndef CMP_EXTRAS_H
#define CMP_EXTRAS_H

/**
 * @file cmp_extras.h
 * @brief Tooltip and badge helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_text.h"

/** @brief Plain tooltip variant. */
#define CMP_TOOLTIP_VARIANT_PLAIN 1
/** @brief Rich tooltip variant. */
#define CMP_TOOLTIP_VARIANT_RICH 2

/** @brief Tooltip anchor is a rectangle. */
#define CMP_TOOLTIP_ANCHOR_RECT 0
/** @brief Tooltip anchor is a point. */
#define CMP_TOOLTIP_ANCHOR_POINT 1

/** @brief Tooltip direction: above the anchor. */
#define CMP_TOOLTIP_DIRECTION_UP 0
/** @brief Tooltip direction: below the anchor. */
#define CMP_TOOLTIP_DIRECTION_DOWN 1
/** @brief Tooltip direction: left of the anchor. */
#define CMP_TOOLTIP_DIRECTION_LEFT 2
/** @brief Tooltip direction: right of the anchor. */
#define CMP_TOOLTIP_DIRECTION_RIGHT 3

/** @brief Tooltip alignment: start edge. */
#define CMP_TOOLTIP_ALIGN_START 0
/** @brief Tooltip alignment: center. */
#define CMP_TOOLTIP_ALIGN_CENTER 1
/** @brief Tooltip alignment: end edge. */
#define CMP_TOOLTIP_ALIGN_END 2

/** @brief Plain tooltip default horizontal padding in pixels. */
#define CMP_TOOLTIP_PLAIN_DEFAULT_PADDING_X 8.0f
/** @brief Plain tooltip default vertical padding in pixels. */
#define CMP_TOOLTIP_PLAIN_DEFAULT_PADDING_Y 4.0f
/** @brief Plain tooltip default minimum width in pixels. */
#define CMP_TOOLTIP_PLAIN_DEFAULT_MIN_WIDTH 0.0f
/** @brief Plain tooltip default minimum height in pixels. */
#define CMP_TOOLTIP_PLAIN_DEFAULT_MIN_HEIGHT 24.0f
/** @brief Plain tooltip default maximum width in pixels (0 = none). */
#define CMP_TOOLTIP_PLAIN_DEFAULT_MAX_WIDTH 0.0f
/** @brief Plain tooltip default maximum height in pixels (0 = none). */
#define CMP_TOOLTIP_PLAIN_DEFAULT_MAX_HEIGHT 0.0f
/** @brief Plain tooltip default corner radius in pixels. */
#define CMP_TOOLTIP_PLAIN_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Plain tooltip default anchor gap in pixels. */
#define CMP_TOOLTIP_PLAIN_DEFAULT_ANCHOR_GAP 4.0f

/** @brief Rich tooltip default horizontal padding in pixels. */
#define CMP_TOOLTIP_RICH_DEFAULT_PADDING_X 12.0f
/** @brief Rich tooltip default vertical padding in pixels. */
#define CMP_TOOLTIP_RICH_DEFAULT_PADDING_Y 8.0f
/** @brief Rich tooltip default minimum width in pixels. */
#define CMP_TOOLTIP_RICH_DEFAULT_MIN_WIDTH 0.0f
/** @brief Rich tooltip default minimum height in pixels. */
#define CMP_TOOLTIP_RICH_DEFAULT_MIN_HEIGHT 40.0f
/** @brief Rich tooltip default maximum width in pixels (0 = none). */
#define CMP_TOOLTIP_RICH_DEFAULT_MAX_WIDTH 0.0f
/** @brief Rich tooltip default maximum height in pixels (0 = none). */
#define CMP_TOOLTIP_RICH_DEFAULT_MAX_HEIGHT 0.0f
/** @brief Rich tooltip default corner radius in pixels. */
#define CMP_TOOLTIP_RICH_DEFAULT_CORNER_RADIUS 8.0f
/** @brief Rich tooltip default anchor gap in pixels. */
#define CMP_TOOLTIP_RICH_DEFAULT_ANCHOR_GAP 4.0f
/** @brief Rich tooltip default spacing between title and body in pixels. */
#define CMP_TOOLTIP_RICH_DEFAULT_TITLE_BODY_SPACING 4.0f

/**
 * @brief Tooltip anchor descriptor.
 */
typedef struct CMPTooltipAnchor {
  cmp_u32 type;  /**< Anchor type (CMP_TOOLTIP_ANCHOR_*). */
  CMPRect rect;  /**< Anchor rectangle (for CMP_TOOLTIP_ANCHOR_RECT). */
  CMPVec2 point; /**< Anchor point (for CMP_TOOLTIP_ANCHOR_POINT). */
} CMPTooltipAnchor;

/**
 * @brief Tooltip placement descriptor.
 */
typedef struct CMPTooltipPlacement {
  cmp_u32 direction; /**< Preferred direction (CMP_TOOLTIP_DIRECTION_*). */
  cmp_u32 align; /**< Alignment along the cross axis (CMP_TOOLTIP_ALIGN_*). */
} CMPTooltipPlacement;

/**
 * @brief Tooltip content metrics.
 */
typedef struct CMPTooltipContent {
  CMPTextMetrics title_metrics; /**< Title metrics (rich tooltips). */
  CMPTextMetrics body_metrics;  /**< Body metrics (plain or rich tooltips). */
  CMPBool has_title;            /**< CMP_TRUE when title metrics are valid. */
  CMPBool has_body;             /**< CMP_TRUE when body metrics are valid. */
} CMPTooltipContent;

/**
 * @brief Tooltip style descriptor.
 */
typedef struct CMPTooltipStyle {
  cmp_u32 variant;        /**< Tooltip variant (CMP_TOOLTIP_VARIANT_*). */
  CMPLayoutEdges padding; /**< Padding around tooltip content. */
  CMPScalar min_width;    /**< Minimum tooltip width in pixels (>= 0). */
  CMPScalar min_height;   /**< Minimum tooltip height in pixels (>= 0). */
  CMPScalar max_width; /**< Maximum tooltip width in pixels (>= 0, 0 = none). */
  CMPScalar
      max_height; /**< Maximum tooltip height in pixels (>= 0, 0 = none). */
  CMPScalar corner_radius; /**< Corner radius in pixels (>= 0). */
  CMPScalar anchor_gap; /**< Gap between anchor and tooltip in pixels (>= 0). */
  CMPScalar
      title_body_spacing;  /**< Spacing between title and body in pixels (>= 0).
                            */
  CMPTextStyle text_style; /**< Plain tooltip text style. */
  CMPTextStyle title_style;  /**< Rich tooltip title style. */
  CMPTextStyle body_style;   /**< Rich tooltip body style. */
  CMPColor background_color; /**< Tooltip background color. */
} CMPTooltipStyle;

/**
 * @brief Initialize a plain tooltip style with defaults.
 * @param style Tooltip style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_tooltip_style_init_plain(CMPTooltipStyle *style);

/**
 * @brief Initialize a rich tooltip style with defaults.
 * @param style Tooltip style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_tooltip_style_init_rich(CMPTooltipStyle *style);

/**
 * @brief Compute the content size for a tooltip.
 * @param style Tooltip style descriptor.
 * @param content Tooltip content metrics.
 * @param out_size Receives the computed tooltip size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_tooltip_compute_content_size(
    const CMPTooltipStyle *style, const CMPTooltipContent *content,
    CMPSize *out_size);

/**
 * @brief Compute the tooltip bounds within an overlay region.
 * @param style Tooltip style descriptor.
 * @param anchor Tooltip anchor descriptor.
 * @param placement Placement descriptor.
 * @param overlay Overlay bounds used for clamping.
 * @param tooltip_size Tooltip size to position.
 * @param out_bounds Receives the tooltip bounds.
 * @param out_direction Receives the resolved direction.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_tooltip_compute_bounds(
    const CMPTooltipStyle *style, const CMPTooltipAnchor *anchor,
    const CMPTooltipPlacement *placement, const CMPRect *overlay,
    const CMPSize *tooltip_size, CMPRect *out_bounds, cmp_u32 *out_direction);

/** @brief Badge corner: top-left. */
#define CMP_BADGE_CORNER_TOP_LEFT 0
/** @brief Badge corner: top-right. */
#define CMP_BADGE_CORNER_TOP_RIGHT 1
/** @brief Badge corner: bottom-right. */
#define CMP_BADGE_CORNER_BOTTOM_RIGHT 2
/** @brief Badge corner: bottom-left. */
#define CMP_BADGE_CORNER_BOTTOM_LEFT 3

/** @brief Default badge minimum size in pixels. */
#define CMP_BADGE_DEFAULT_MIN_SIZE 16.0f
/** @brief Default badge dot diameter in pixels. */
#define CMP_BADGE_DEFAULT_DOT_DIAMETER 6.0f
/** @brief Default badge horizontal padding in pixels. */
#define CMP_BADGE_DEFAULT_PADDING_X 4.0f
/** @brief Default badge vertical padding in pixels. */
#define CMP_BADGE_DEFAULT_PADDING_Y 2.0f
/** @brief Default badge corner radius in pixels. */
#define CMP_BADGE_DEFAULT_CORNER_RADIUS 8.0f
/** @brief Default icon badge offset X in pixels. */
#define CMP_BADGE_DEFAULT_ICON_OFFSET_X 0.0f
/** @brief Default icon badge offset Y in pixels. */
#define CMP_BADGE_DEFAULT_ICON_OFFSET_Y 0.0f
/** @brief Default navigation badge offset X in pixels. */
#define CMP_BADGE_DEFAULT_NAV_OFFSET_X 0.0f
/** @brief Default navigation badge offset Y in pixels. */
#define CMP_BADGE_DEFAULT_NAV_OFFSET_Y 0.0f

/**
 * @brief Badge content metrics.
 */
typedef struct CMPBadgeContent {
  CMPTextMetrics text_metrics; /**< Text metrics for the badge count. */
  CMPBool has_text;            /**< CMP_TRUE when text metrics are valid. */
} CMPBadgeContent;

/**
 * @brief Badge placement descriptor.
 */
typedef struct CMPBadgePlacement {
  cmp_u32 corner;     /**< Anchor corner (CMP_BADGE_CORNER_*). */
  CMPScalar offset_x; /**< Horizontal offset from the anchor corner. */
  CMPScalar offset_y; /**< Vertical offset from the anchor corner. */
} CMPBadgePlacement;

/**
 * @brief Badge style descriptor.
 */
typedef struct CMPBadgeStyle {
  CMPScalar min_size;        /**< Minimum badge size in pixels (>= 0). */
  CMPScalar dot_diameter;    /**< Dot badge diameter in pixels (>= 0). */
  CMPScalar padding_x;       /**< Horizontal padding in pixels (>= 0). */
  CMPScalar padding_y;       /**< Vertical padding in pixels (>= 0). */
  CMPScalar corner_radius;   /**< Corner radius in pixels (>= 0). */
  CMPTextStyle text_style;   /**< Text style for badge counts. */
  CMPColor background_color; /**< Badge background color. */
} CMPBadgeStyle;

/**
 * @brief Initialize a badge style with defaults.
 * @param style Badge style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_badge_style_init(CMPBadgeStyle *style);

/**
 * @brief Initialize an icon badge placement descriptor.
 * @param placement Placement descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_badge_placement_init_icon(CMPBadgePlacement *placement);

/**
 * @brief Initialize a navigation badge placement descriptor.
 * @param placement Placement descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_badge_placement_init_navigation(CMPBadgePlacement *placement);

/**
 * @brief Compute the badge size.
 * @param style Badge style descriptor.
 * @param content Badge content metrics.
 * @param out_size Receives the computed badge size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_badge_compute_size(const CMPBadgeStyle *style,
                                            const CMPBadgeContent *content,
                                            CMPSize *out_size);

/**
 * @brief Compute the badge bounds relative to an anchor rectangle.
 * @param style Badge style descriptor.
 * @param content Badge content metrics.
 * @param anchor Anchor bounds to attach to.
 * @param placement Placement descriptor.
 * @param out_bounds Receives the badge bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_badge_compute_bounds(
    const CMPBadgeStyle *style, const CMPBadgeContent *content,
    const CMPRect *anchor, const CMPBadgePlacement *placement,
    CMPRect *out_bounds);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for color validation.
 * @param color Color to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_extras_test_validate_color(const CMPColor *color);

/**
 * @brief Test wrapper for edge validation.
 * @param edges Edge descriptor to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_extras_test_validate_edges(const CMPLayoutEdges *edges);

/**
 * @brief Test wrapper for text style validation.
 * @param style Text style to validate.
 * @param require_family CMP_TRUE to require a family name.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_extras_test_validate_text_style(
    const CMPTextStyle *style, CMPBool require_family);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_extras_test_validate_rect(const CMPRect *rect);

/**
 * @brief Test wrapper for tooltip anchor validation.
 * @param anchor Anchor descriptor to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_extras_test_validate_tooltip_anchor(const CMPTooltipAnchor *anchor);

/**
 * @brief Test wrapper for tooltip placement validation.
 * @param placement Placement descriptor to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_extras_test_validate_tooltip_placement(
    const CMPTooltipPlacement *placement);

/**
 * @brief Test wrapper for tooltip style validation.
 * @param style Tooltip style descriptor.
 * @param require_family CMP_TRUE to require a family name.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_extras_test_validate_tooltip_style(
    const CMPTooltipStyle *style, CMPBool require_family);

/**
 * @brief Test wrapper for tooltip content validation.
 * @param style Tooltip style descriptor.
 * @param content Tooltip content metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_extras_test_validate_tooltip_content(
    const CMPTooltipStyle *style, const CMPTooltipContent *content);

/**
 * @brief Test wrapper for badge style validation.
 * @param style Badge style descriptor.
 * @param require_family CMP_TRUE to require a family name.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_extras_test_validate_badge_style(
    const CMPBadgeStyle *style, CMPBool require_family);

/**
 * @brief Test wrapper for badge content validation.
 * @param content Badge content metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_extras_test_validate_badge_content(const CMPBadgeContent *content);

/**
 * @brief Test wrapper for badge placement validation.
 * @param placement Placement descriptor to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_extras_test_validate_badge_placement(const CMPBadgePlacement *placement);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_EXTRAS_H */
