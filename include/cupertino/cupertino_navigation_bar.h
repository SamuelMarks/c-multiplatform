#ifndef CUPERTINO_NAVIGATION_BAR_H
#define CUPERTINO_NAVIGATION_BAR_H

/**
 * @file cupertino_navigation_bar.h
 * @brief Apple Cupertino Navigation Bar (UINavigationBar).
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_blur.h"
#include "cupertino/cupertino_button.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"
/* clang-format on */

/** @brief Navigation bar variants */
typedef enum CupertinoNavigationBarVariant {
  CUPERTINO_NAV_BAR_STANDARD = 0, /**< Standard inline title. */
  CUPERTINO_NAV_BAR_LARGE_TITLE   /**< Collapsible large title. */
} CupertinoNavigationBarVariant;

/** @brief Navigation bar style descriptor */
typedef struct CupertinoNavigationBarStyle {
  CupertinoNavigationBarVariant variant; /**< Bar variant. */
  CupertinoBlurStyle background_blur;    /**< Background blur effect. */
  CMPColor tint_color;                   /**< Global accent/tint color. */
  CMPBool is_dark_mode;                  /**< Active dark mode state. */
  CMPTextStyle title_style;              /**< Standard inline title style. */
  CMPTextStyle large_title_style;        /**< Expanded large title style. */
} CupertinoNavigationBarStyle;

/** @brief Navigation bar widget instance */
typedef struct CupertinoNavigationBar {
  CMPWidget widget;                  /**< Widget interface. */
  CMPTextBackend text_backend;       /**< Text backend for drawing labels. */
  CupertinoNavigationBarStyle style; /**< Configured style. */
  CMPRect bounds;                    /**< Layout bounds. */
  const char *title_utf8;            /**< Optional title string. */
  cmp_usize title_len;               /**< Title string length. */
  CMPScalar
      scroll_offset; /**< Current scroll offset for large title collapsing. */
  CMPRect
      safe_area_insets; /**< OS safe area insets (top notch/dynamic island). */
  CupertinoButton *left_item;  /**< Optional left button (e.g., Back). */
  CupertinoButton *right_item; /**< Optional right button. */
} CupertinoNavigationBar;

/**
 * @brief Initialize a Cupertino navigation bar style.
 * @param style Style to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL
cupertino_navigation_bar_style_init(CupertinoNavigationBarStyle *style);

/**
 * @brief Initialize a Cupertino navigation bar widget.
 * @param bar Navigation bar to initialize.
 * @param text_backend Text backend.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_navigation_bar_init(
    CupertinoNavigationBar *bar, const CMPTextBackend *text_backend);

/**
 * @brief Set the navigation bar title.
 * @param bar Navigation bar instance.
 * @param title_utf8 UTF-8 title text.
 * @param title_len Length of title.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_navigation_bar_set_title(
    CupertinoNavigationBar *bar, const char *title_utf8, cmp_usize title_len);

/**
 * @brief Layout the navigation bar.
 * @param bar Navigation bar instance.
 * @param bounds The layout bounds.
 * @param safe_area Safe area insets (top).
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_navigation_bar_layout(
    CupertinoNavigationBar *bar, CMPRect bounds, CMPRect safe_area);

/**
 * @brief Paint the navigation bar.
 * @param bar Navigation bar instance.
 * @param ctx Paint context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_navigation_bar_paint(
    const CupertinoNavigationBar *bar, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_NAVIGATION_BAR_H */
