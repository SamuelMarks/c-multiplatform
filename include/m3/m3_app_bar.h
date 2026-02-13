#ifndef M3_APP_BAR_H
#define M3_APP_BAR_H

/**
 * @file m3_app_bar.h
 * @brief Top app bar widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_scroll.h"
#include "cmpc/cmp_text.h"
#include "cmpc/cmp_visuals.h"

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
  cmp_u32 variant;            /**< App bar variant (CMP_APP_BAR_VARIANT_*). */
  CMPLayoutEdges padding;     /**< Padding around app bar contents. */
  CMPScalar collapsed_height; /**< Collapsed bar height in pixels (> 0). */
  CMPScalar expanded_height;  /**< Expanded bar height in pixels (> 0). */
  CMPTextStyle title_style;   /**< Title text style (requires family name). */
  CMPColor background_color;  /**< Background fill color. */
  CMPShadow shadow;           /**< Shadow descriptor. */
  CMPBool shadow_enabled;     /**< CMP_TRUE when shadow rendering is enabled. */
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
 * @brief Initialize a top app bar widget.
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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_APP_BAR_H */
