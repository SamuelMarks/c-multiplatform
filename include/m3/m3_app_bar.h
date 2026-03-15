#ifndef M3_APP_BAR_H
#define M3_APP_BAR_H

/**
 * @file m3_app_bar.h
 * @brief Top app bar widgets for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_scroll.h"
#include "cmpc/cmp_text.h"
#include "cmpc/cmp_visuals.h"
/* clang-format on */

/** @brief Small top app bar variant. */
#define M3_APP_BAR_VARIANT_SMALL 1
/** @brief Center-aligned top app bar variant. */
#define M3_APP_BAR_VARIANT_CENTER 2
/** @brief Medium top app bar variant. */
#define M3_APP_BAR_VARIANT_MEDIUM 3
/** @brief Large top app bar variant. */
#define M3_APP_BAR_VARIANT_LARGE 4
/** @brief Bottom app bar variant. */
#define M3_APP_BAR_VARIANT_BOTTOM 5

/** @brief Default small app bar height in pixels. */
#define M3_APP_BAR_DEFAULT_SMALL_HEIGHT 64.0f
/** @brief Default medium app bar height in pixels. */
#define M3_APP_BAR_DEFAULT_MEDIUM_HEIGHT 112.0f
/** @brief Default large app bar height in pixels. */
#define M3_APP_BAR_DEFAULT_LARGE_HEIGHT 152.0f
/** @brief Default bottom app bar height in pixels. */
#define M3_APP_BAR_DEFAULT_BOTTOM_HEIGHT 80.0f
/** @brief Default horizontal padding in pixels. */
#define M3_APP_BAR_DEFAULT_PADDING_X 16.0f
/** @brief Default vertical padding in pixels. */
#define M3_APP_BAR_DEFAULT_PADDING_Y 8.0f
/** @brief Default title font size in pixels. */
#define M3_APP_BAR_DEFAULT_TITLE_SIZE 22
/** @brief Default title font weight. */
#define M3_APP_BAR_DEFAULT_TITLE_WEIGHT 500

/**
 * @brief App bar style descriptor.
 */
typedef struct M3AppBarStyle {
  cmp_u32 variant;            /**< App bar variant (CMP_APP_BAR_VARIANT_*). */
  CMPLayoutEdges padding;     /**< Padding around app bar contents. */
  CMPScalar collapsed_height; /**< Collapsed bar height in pixels (> 0). */
  CMPScalar expanded_height;  /**< Expanded bar height in pixels (> 0). */
  CMPTextStyle title_style;   /**< Title text style (requires family name). */
  CMPColor background_color;  /**< Background fill color. */
  CMPShadow shadow;           /**< Shadow descriptor. */
  CMPBool shadow_enabled;     /**< CMP_TRUE when shadow rendering is enabled. */
  CMPBool is_rtl; /**< CMP_TRUE if layout should be right-to-left. */
} M3AppBarStyle;

/**
 * @brief App bar widget instance.
 */
typedef struct M3AppBar {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend; /**< Text backend instance. */
  CMPHandle title_font;        /**< Font handle for the title. */
  M3AppBarStyle style;         /**< Current app bar style. */
  const char
      *utf8_title;     /**< UTF-8 title pointer (may be NULL when len is 0). */
  cmp_usize title_len; /**< Title length in bytes. */
  CMPRect bounds;      /**< Layout bounds. */
  CMPScalar collapse_offset;     /**< Collapse offset in pixels (0..range). */
  CMPScrollParent scroll_parent; /**< Scroll parent interface. */
  CMPBool owns_font;             /**< CMP_TRUE when widget owns the font. */
} M3AppBar;

/**
 * @brief Initialize a small top app bar style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_style_init_small(M3AppBarStyle *style);

/**
 * @brief Initialize a center-aligned top app bar style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_style_init_center(M3AppBarStyle *style);

/**
 * @brief Initialize a medium top app bar style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_style_init_medium(M3AppBarStyle *style);

/**
 * @brief Initialize a large top app bar style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_style_init_large(M3AppBarStyle *style);

/**
 * @brief Initialize a bottom app bar style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_style_init_bottom(M3AppBarStyle *style);

/**
 * @brief Initialize an app bar widget.
 * @param bar App bar instance.
 * @param backend Text backend instance.
 * @param style App bar style descriptor.
 * @param utf8_title UTF-8 title pointer (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_init(M3AppBar *bar,
                                     const CMPTextBackend *backend,
                                     const M3AppBarStyle *style,
                                     const char *utf8_title,
                                     cmp_usize title_len);

/**
 * @brief Update the app bar title text.
 * @param bar App bar instance.
 * @param utf8_title UTF-8 title pointer (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_set_title(M3AppBar *bar, const char *utf8_title,
                                          cmp_usize title_len);

/**
 * @brief Update the app bar style.
 * @param bar App bar instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_set_style(M3AppBar *bar,
                                          const M3AppBarStyle *style);

/**
 * @brief Set the collapse offset of the app bar.
 * @param bar App bar instance.
 * @param offset Collapse offset in pixels (0..range).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_set_collapse_offset(M3AppBar *bar,
                                                    CMPScalar offset);

/**
 * @brief Retrieve the collapse offset of the app bar.
 * @param bar App bar instance.
 * @param out_offset Receives the collapse offset.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_get_collapse_offset(const M3AppBar *bar,
                                                    CMPScalar *out_offset);

/**
 * @brief Retrieve the current app bar height.
 * @param bar App bar instance.
 * @param out_height Receives the current height in pixels.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_get_height(const M3AppBar *bar,
                                           CMPScalar *out_height);

/**
 * @brief Compute content bounds inside the app bar.
 * @param bar App bar instance.
 * @param out_bounds Receives the content bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_app_bar_get_content_bounds(const M3AppBar *bar,
                                                   CMPRect *out_bounds);

/**
 * @brief Helper for test coverage.
 * @param fail_point Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_set_fail_point(cmp_u32 fail_point);

/**
 * @brief Helper for test coverage.
 * @param call_count Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_set_color_fail_after(cmp_u32 call_count);

/**
 * @brief Helper for test coverage.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_clear_fail_points(void);

/**
 * @brief Helper for test coverage.
 * @param call_count Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_set_match_fail_after(cmp_u32 call_count);

/**
 * @brief Helper for test coverage.
 * @param call_count Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_set_color_error_after(cmp_u32 call_count);

/**
 * @brief Helper for test coverage.
 * @param enable Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_set_collapse_fail(CMPBool enable);

/**
 * @brief Helper for test coverage.
 * @param color Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_validate_color(const CMPColor *color);

/**
 * @brief Helper for test coverage.
 * @param color Parameter for testing.
 * @param r Parameter for testing.
 * @param g Parameter for testing.
 * @param b Parameter for testing.
 * @param a Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_color_set(CMPColor *color, CMPScalar r,
                                               CMPScalar g, CMPScalar b,
                                               CMPScalar a);

CMP_API int
    CMP_CALL
    /**
     * @brief Helper for test coverage.
     * @param edges Parameter for testing.
     * @return Test result.
     */
    m3_app_bar_test_validate_edges(const CMPLayoutEdges *edges);

/**
 * @brief Helper for test coverage.
 * @param style Parameter for testing.
 * @param require_family Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_validate_text_style(
    const CMPTextStyle *style, CMPBool require_family);

/**
 * @brief Helper for test coverage.
 * @param style Parameter for testing.
 * @param require_family Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_validate_style(const M3AppBarStyle *style,
                                                    CMPBool require_family);

/**
 * @brief Helper for test coverage.
 * @param out_fail Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_call_color_should_fail(CMPBool *out_fail);

/**
 * @brief Helper for test coverage.
 * @param point Parameter for testing.
 * @param out_match Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_call_fail_point_match(cmp_u32 point,
                                                           CMPBool *out_match);

/**
 * @brief Helper for test coverage.
 * @param spec Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_validate_measure_spec(CMPMeasureSpec spec);

/**
 * @brief Helper for test coverage.
 * @param rect Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_validate_rect(const CMPRect *rect);

/**
 * @brief Helper for test coverage.
 * @param style Parameter for testing.
 * @param out_range Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_compute_collapse_range(
    const M3AppBarStyle *style, CMPScalar *out_range);

/**
 * @brief Helper for test coverage.
 * @param bar Parameter for testing.
 * @param out_height Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_compute_current_height(
    const M3AppBar *bar, CMPScalar *out_height);

/**
 * @brief Helper for test coverage.
 * @param bar Parameter for testing.
 * @param out_bounds Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_compute_content_bounds(
    const M3AppBar *bar, CMPRect *out_bounds);

/**
 * @brief Helper for test coverage.
 * @param bar Parameter for testing.
 * @param metrics Parameter for testing.
 * @param out_x Parameter for testing.
 * @param out_y Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_compute_title_position(
    const M3AppBar *bar, const CMPTextMetrics *metrics, CMPScalar *out_x,
    CMPScalar *out_y);

/**
 * @brief Helper for test coverage.
 * @param bar Parameter for testing.
 * @param out_metrics Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_measure_title(const M3AppBar *bar,
                                                   CMPTextMetrics *out_metrics);

/**
 * @brief Helper for test coverage.
 * @param bar Parameter for testing.
 * @param delta Parameter for testing.
 * @param out_consumed Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_app_bar_test_apply_scroll(M3AppBar *bar,
                                                  CMPScalar delta,
                                                  CMPScalar *out_consumed);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_APP_BAR_H */
