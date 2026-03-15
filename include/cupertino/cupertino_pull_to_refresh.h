#ifndef CUPERTINO_PULL_TO_REFRESH_H
#define CUPERTINO_PULL_TO_REFRESH_H

/**
 * @file cupertino_pull_to_refresh.h
 * @brief Apple Cupertino Pull-to-Refresh indicator (UIRefreshControl).
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_scroll.h"
#include "cupertino/cupertino_color.h"
/* clang-format on */

/**
 * @brief Refresh callback signature.
 * @param ctx User callback context pointer.
 * @return CMP_OK on success.
 */
typedef int(CMP_CALL *CupertinoRefreshCallback)(void *ctx);

/** @brief Pull-to-refresh style descriptor */
typedef struct CupertinoRefreshStyle {
  CMPColor tint_color;  /**< Color of the spinning indicator. */
  CMPBool is_dark_mode; /**< True if dark mode is active. */
  CMPScalar
      trigger_offset; /**< Distance needed to pull before triggering refresh. */
} CupertinoRefreshStyle;

/** @brief Pull-to-refresh widget instance */
typedef struct CupertinoRefreshControl {
  CMPWidget widget;            /**< Widget interface. */
  CupertinoRefreshStyle style; /**< Configured style. */
  CMPScrollParent
      scroll_parent; /**< Scroll parent for intercepting over-scroll. */
  CMPWidget *child;  /**< Child scrollable widget. */
  CMPRect bounds;    /**< Layout bounds. */

  CMPScalar current_pull;  /**< How far the user has currently pulled down. */
  CMPBool is_refreshing;   /**< True if currently refreshing. */
  CMPScalar spinner_angle; /**< Current rotation angle of the spinner. */

  CupertinoRefreshCallback on_refresh; /**< Callback function. */
  void *on_refresh_ctx;                /**< Callback context. */
} CupertinoRefreshControl;

/**
 * @brief Initialize a Cupertino pull-to-refresh style.
 * @param style Style to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_refresh_style_init(CupertinoRefreshStyle *style);

/**
 * @brief Initialize a Cupertino pull-to-refresh widget.
 * @param ptr Widget to initialize.
 * @param child Scrollable child widget.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_refresh_init(CupertinoRefreshControl *ptr,
                                            CMPWidget *child);

/**
 * @brief Set the refreshing state programmatically.
 * @param ptr Refresh control instance.
 * @param is_refreshing True to spin, False to stop.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_refresh_set_refreshing(
    CupertinoRefreshControl *ptr, CMPBool is_refreshing);

/**
 * @brief Set the callback to be triggered.
 * @param ptr Refresh control instance.
 * @param cb Callback function.
 * @param ctx User data.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_refresh_set_callback(
    CupertinoRefreshControl *ptr, CupertinoRefreshCallback cb, void *ctx);

/**
 * @brief Layout the refresh control.
 * @param ptr Refresh control instance.
 * @param bounds Layout bounds.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_refresh_layout(CupertinoRefreshControl *ptr,
                                              CMPRect bounds);

/**
 * @brief Paint the refresh control.
 * @param ptr Refresh control instance.
 * @param ctx Paint context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_refresh_paint(const CupertinoRefreshControl *ptr,
                                             CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_PULL_TO_REFRESH_H */
