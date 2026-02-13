#ifndef M3_NAVIGATION_H
#define M3_NAVIGATION_H

/**
 * @file m3_navigation.h
 * @brief Navigation widgets (bar, rail, drawer) for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_text.h"

/** @brief Auto-select the navigation mode based on width. */
#define M3_NAV_MODE_AUTO 0
/** @brief Bottom navigation bar. */
#define M3_NAV_MODE_BAR 1
/** @brief Navigation rail. */
#define M3_NAV_MODE_RAIL 2
/** @brief Navigation drawer. */
#define M3_NAV_MODE_DRAWER 3

/** @brief Invalid navigation index sentinel. */
#define M3_NAV_INVALID_INDEX ((cmp_usize) ~(cmp_usize)0)

/** @brief Default bar height in pixels. */
#define M3_NAV_DEFAULT_BAR_HEIGHT 80.0f
/** @brief Default rail width in pixels. */
#define M3_NAV_DEFAULT_RAIL_WIDTH 80.0f
/** @brief Default drawer width in pixels. */
#define M3_NAV_DEFAULT_DRAWER_WIDTH 320.0f
/** @brief Default navigation item height in pixels. */
#define M3_NAV_DEFAULT_ITEM_HEIGHT 56.0f
/** @brief Default navigation item minimum width in pixels. */
#define M3_NAV_DEFAULT_ITEM_MIN_WIDTH 64.0f
/** @brief Default navigation item spacing in pixels. */
#define M3_NAV_DEFAULT_ITEM_SPACING 0.0f
/** @brief Default selection indicator thickness in pixels. */
#define M3_NAV_DEFAULT_INDICATOR_THICKNESS 3.0f
/** @brief Default selection indicator corner radius in pixels. */
#define M3_NAV_DEFAULT_INDICATOR_CORNER 0.0f
/** @brief Default width breakpoint for switching to rail. */
#define M3_NAV_DEFAULT_BREAKPOINT_RAIL 600.0f
/** @brief Default width breakpoint for switching to drawer. */
#define M3_NAV_DEFAULT_BREAKPOINT_DRAWER 840.0f

struct M3Navigation;

/**
 * @brief Navigation destination descriptor.
 */
typedef struct M3NavigationItem {
  const char
      *utf8_label; /**< UTF-8 label pointer (may be NULL when utf8_len is 0). */
  cmp_usize utf8_len; /**< UTF-8 label length in bytes. */
} M3NavigationItem;

/**
 * @brief Selection callback for navigation widgets.
 *
 * @param ctx User callback context pointer.
 * @param nav Navigation widget instance.
 * @param index Newly selected index.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPNavigationOnSelect)(void *ctx,
                                             struct M3Navigation *nav,
                                             cmp_usize index);

/**
 * @brief Navigation style descriptor.
 */
typedef struct M3NavigationStyle {
  cmp_u32 mode;                  /**< Navigation mode (CMP_NAV_MODE_*). */
  CMPScalar bar_height;          /**< Bar height in pixels (> 0). */
  CMPScalar rail_width;          /**< Rail width in pixels (> 0). */
  CMPScalar drawer_width;        /**< Drawer width in pixels (> 0). */
  CMPScalar item_height;         /**< Item height in pixels (> 0). */
  CMPScalar item_min_width;      /**< Minimum item width in pixels (>= 0). */
  CMPScalar item_spacing;        /**< Spacing between items in pixels (>= 0). */
  CMPLayoutEdges padding;        /**< Padding around contents. */
  CMPScalar indicator_thickness; /**< Selection indicator thickness in pixels
                                   (>= 0). */
  CMPScalar indicator_corner; /**< Selection indicator corner radius in pixels
                                (>= 0). */
  CMPScalar breakpoint_rail; /**< Width breakpoint for rail selection (>= 0). */
  CMPScalar
      breakpoint_drawer; /**< Width breakpoint for drawer selection (>= 0). */
  CMPTextStyle
      text_style; /**< Base text style (requires a valid family name). */
  CMPColor selected_text_color; /**< Selected label color. */
  CMPColor indicator_color;     /**< Selection indicator color. */
  CMPColor background_color;    /**< Background fill color. */
} M3NavigationStyle;

/**
 * @brief Navigation widget instance.
 */
typedef struct M3Navigation {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend;   /**< Text backend instance. */
  CMPHandle font;                /**< Font handle for labels. */
  M3NavigationStyle style;       /**< Current navigation style. */
  const M3NavigationItem *items; /**< Item list (not owned). */
  cmp_usize item_count;          /**< Number of items. */
  cmp_usize selected_index; /**< Selected item index or M3_NAV_INVALID_INDEX. */
  cmp_usize pressed_index;  /**< Pressed item index or M3_NAV_INVALID_INDEX. */
  cmp_u32 active_mode;      /**< Last resolved mode (CMP_NAV_MODE_*). */
  CMPRect bounds;           /**< Layout bounds. */
  CMPNavigationOnSelect on_select; /**< Selection callback (may be NULL). */
  void *on_select_ctx;             /**< Selection callback context pointer. */
  CMPBool owns_font;               /**< CMP_TRUE when widget owns the font. */
} M3Navigation;

/**
 * @brief Initialize a navigation style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_style_init(M3NavigationStyle *style);

/**
 * @brief Initialize a navigation widget.
 * @param nav Navigation widget instance.
 * @param backend Text backend instance.
 * @param style Style descriptor.
 * @param items Item array (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @param selected_index Initial selection or M3_NAV_INVALID_INDEX.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_init(M3Navigation *nav,
                                        const CMPTextBackend *backend,
                                        const M3NavigationStyle *style,
                                        const M3NavigationItem *items,
                                        cmp_usize item_count,
                                        cmp_usize selected_index);

/**
 * @brief Replace the navigation items.
 * @param nav Navigation widget instance.
 * @param items Item array (may be NULL when item_count is 0).
 * @param item_count Number of items.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_set_items(M3Navigation *nav,
                                             const M3NavigationItem *items,
                                             cmp_usize item_count);

/**
 * @brief Update the navigation style.
 * @param nav Navigation widget instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_set_style(M3Navigation *nav,
                                             const M3NavigationStyle *style);

/**
 * @brief Update the selected item.
 * @param nav Navigation widget instance.
 * @param selected_index Selected index or M3_NAV_INVALID_INDEX.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_set_selected(M3Navigation *nav,
                                                cmp_usize selected_index);

/**
 * @brief Retrieve the selected item index.
 * @param nav Navigation widget instance.
 * @param out_selected Receives the selected index.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_get_selected(const M3Navigation *nav,
                                                cmp_usize *out_selected);

/**
 * @brief Assign the selection callback.
 * @param nav Navigation widget instance.
 * @param on_select Selection callback (may be NULL).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_set_on_select(
    M3Navigation *nav, CMPNavigationOnSelect on_select, void *ctx);

/**
 * @brief Resolve the currently active navigation mode.
 * @param nav Navigation widget instance.
 * @param out_mode Receives the active mode.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_get_mode(const M3Navigation *nav,
                                            cmp_u32 *out_mode);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for navigation color validation.
 * @param color Color to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_validate_color(const CMPColor *color);

/**
 * @brief Test wrapper for navigation edge validation.
 * @param edges Padding edges.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_navigation_test_validate_edges(const CMPLayoutEdges *edges);

/**
 * @brief Test wrapper for navigation text style validation.
 * @param style Text style descriptor.
 * @param require_family CMP_TRUE to require family name.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_validate_text_style(
    const CMPTextStyle *style, CMPBool require_family);

/**
 * @brief Test wrapper for navigation style validation.
 * @param style Navigation style descriptor.
 * @param require_family CMP_TRUE to require family name.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_validate_style(
    const M3NavigationStyle *style, CMPBool require_family);

/**
 * @brief Test wrapper for navigation item validation.
 * @param items Item array.
 * @param count Item count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_validate_items(
    const M3NavigationItem *items, cmp_usize count);

/**
 * @brief Test wrapper for measure spec validation.
 * @param spec Measure spec to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_navigation_test_validate_measure_spec(CMPMeasureSpec spec);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_validate_rect(const CMPRect *rect);

/**
 * @brief Test wrapper for navigation mode resolution.
 * @param style Navigation style descriptor.
 * @param width Available width.
 * @param out_mode Receives resolved mode.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_resolve_mode(
    const M3NavigationStyle *style, CMPScalar width, cmp_u32 *out_mode);

/**
 * @brief Test wrapper for measuring navigation content.
 * @param style Navigation style descriptor.
 * @param mode Navigation mode.
 * @param item_count Number of items.
 * @param out_width Receives content width.
 * @param out_height Receives content height.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_measure_content(
    const M3NavigationStyle *style, cmp_u32 mode, cmp_usize item_count,
    CMPScalar *out_width, CMPScalar *out_height);

/**
 * @brief Test layout snapshot for navigation widgets.
 */
typedef struct M3NavigationLayoutTest {
  cmp_u32 mode;             /**< Resolved navigation mode. */
  CMPScalar start_x;        /**< Layout origin X. */
  CMPScalar start_y;        /**< Layout origin Y. */
  CMPScalar item_width;     /**< Item width in pixels. */
  CMPScalar item_height;    /**< Item height in pixels. */
  CMPScalar spacing;        /**< Item spacing in pixels. */
  CMPScalar content_width;  /**< Content width in pixels. */
  CMPScalar content_height; /**< Content height in pixels. */
} M3NavigationLayoutTest;

/**
 * @brief Test wrapper for computing navigation layout.
 * @param nav Navigation instance.
 * @param out_layout Receives computed layout.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_compute_layout(
    const M3Navigation *nav, M3NavigationLayoutTest *out_layout);

/**
 * @brief Test wrapper for navigation hit testing.
 * @param nav Navigation instance.
 * @param layout Layout snapshot.
 * @param x X coordinate in pixels.
 * @param y Y coordinate in pixels.
 * @param out_index Receives hit index or M3_NAV_INVALID_INDEX.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_hit_test(
    const M3Navigation *nav, const M3NavigationLayoutTest *layout, cmp_i32 x,
    cmp_i32 y, cmp_usize *out_index);

/**
 * @brief Set a navigation test fail point.
 * @param point Fail point identifier.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_set_fail_point(cmp_u32 point);

/**
 * @brief Clear navigation test fail points.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_navigation_test_clear_fail_points(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_NAVIGATION_H */
