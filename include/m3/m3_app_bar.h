#ifndef M3_APP_BAR_H
#define M3_APP_BAR_H

/**
 * @file m3_app_bar.h
 * @brief Top app bar widgets for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_ui.h"
#include "m3_layout.h"
#include "m3_scroll.h"
#include "m3_text.h"
#include "m3_visuals.h"

/** @brief Small top app bar variant. */
#define M3_APP_BAR_VARIANT_SMALL 1
/** @brief Center-aligned top app bar variant. */
#define M3_APP_BAR_VARIANT_CENTER 2
/** @brief Medium top app bar variant. */
#define M3_APP_BAR_VARIANT_MEDIUM 3
/** @brief Large top app bar variant. */
#define M3_APP_BAR_VARIANT_LARGE 4

/** @brief Default small app bar height in pixels. */
#define M3_APP_BAR_DEFAULT_SMALL_HEIGHT 64.0f
/** @brief Default medium app bar height in pixels. */
#define M3_APP_BAR_DEFAULT_MEDIUM_HEIGHT 112.0f
/** @brief Default large app bar height in pixels. */
#define M3_APP_BAR_DEFAULT_LARGE_HEIGHT 152.0f
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
  m3_u32 variant;            /**< App bar variant (M3_APP_BAR_VARIANT_*). */
  M3LayoutEdges padding;     /**< Padding around app bar contents. */
  M3Scalar collapsed_height; /**< Collapsed bar height in pixels (> 0). */
  M3Scalar expanded_height;  /**< Expanded bar height in pixels (> 0). */
  M3TextStyle title_style;   /**< Title text style (requires family name). */
  M3Color background_color;  /**< Background fill color. */
  M3Shadow shadow;           /**< Shadow descriptor. */
  M3Bool shadow_enabled;     /**< M3_TRUE when shadow rendering is enabled. */
} M3AppBarStyle;

/**
 * @brief App bar widget instance.
 */
typedef struct M3AppBar {
  M3Widget widget; /**< Widget interface (points to this instance). */
  M3TextBackend text_backend; /**< Text backend instance. */
  M3Handle title_font;        /**< Font handle for the title. */
  M3AppBarStyle style;        /**< Current app bar style. */
  const char
      *utf8_title;    /**< UTF-8 title pointer (may be NULL when len is 0). */
  m3_usize title_len; /**< Title length in bytes. */
  M3Rect bounds;      /**< Layout bounds. */
  M3Scalar collapse_offset;     /**< Collapse offset in pixels (0..range). */
  M3ScrollParent scroll_parent; /**< Scroll parent interface. */
  M3Bool owns_font;             /**< M3_TRUE when widget owns the font. */
} M3AppBar;

/**
 * @brief Initialize a small top app bar style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_style_init_small(M3AppBarStyle *style);

/**
 * @brief Initialize a center-aligned top app bar style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_style_init_center(M3AppBarStyle *style);

/**
 * @brief Initialize a medium top app bar style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_style_init_medium(M3AppBarStyle *style);

/**
 * @brief Initialize a large top app bar style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_style_init_large(M3AppBarStyle *style);

/**
 * @brief Initialize a top app bar widget.
 * @param bar App bar instance.
 * @param backend Text backend instance.
 * @param style App bar style descriptor.
 * @param utf8_title UTF-8 title pointer (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_init(M3AppBar *bar, const M3TextBackend *backend,
                                   const M3AppBarStyle *style,
                                   const char *utf8_title, m3_usize title_len);

/**
 * @brief Update the app bar title text.
 * @param bar App bar instance.
 * @param utf8_title UTF-8 title pointer (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_set_title(M3AppBar *bar, const char *utf8_title,
                                        m3_usize title_len);

/**
 * @brief Update the app bar style.
 * @param bar App bar instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_set_style(M3AppBar *bar,
                                        const M3AppBarStyle *style);

/**
 * @brief Set the collapse offset of the app bar.
 * @param bar App bar instance.
 * @param offset Collapse offset in pixels (0..range).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_set_collapse_offset(M3AppBar *bar,
                                                  M3Scalar offset);

/**
 * @brief Retrieve the collapse offset of the app bar.
 * @param bar App bar instance.
 * @param out_offset Receives the collapse offset.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_get_collapse_offset(const M3AppBar *bar,
                                                  M3Scalar *out_offset);

/**
 * @brief Retrieve the current app bar height.
 * @param bar App bar instance.
 * @param out_height Receives the current height in pixels.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_get_height(const M3AppBar *bar,
                                         M3Scalar *out_height);

/**
 * @brief Compute content bounds inside the app bar.
 * @param bar App bar instance.
 * @param out_bounds Receives the content bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_app_bar_get_content_bounds(const M3AppBar *bar,
                                                 M3Rect *out_bounds);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_APP_BAR_H */
