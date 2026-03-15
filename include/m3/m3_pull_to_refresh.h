#ifndef M3_PULL_TO_REFRESH_H
#define M3_PULL_TO_REFRESH_H

/**
 * @file m3_pull_to_refresh.h
 * @brief Material 3 Pull-to-Refresh indicator and scroll parent integration.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_scroll.h"
/* clang-format on */

/**
 * @brief Refresh callback signature.
 * @param ctx User callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *M3PullToRefreshOnRefresh)(void *ctx);

/**
 * @brief Pull-to-refresh style descriptor.
 */
typedef struct M3PullToRefreshStyle {
  CMPColor container_color; /**< Indicator container background color. */
  CMPColor icon_color;      /**< Indicator icon color. */
  CMPScalar elevation;      /**< Drop shadow elevation (> 0). */
  CMPScalar size;           /**< Diameter of the indicator (e.g., 48.0). */
  CMPScalar max_drag; /**< Maximum drag distance before refresh triggers. */
} M3PullToRefreshStyle;

/**
 * @brief Pull-to-refresh widget instance.
 */
typedef struct M3PullToRefresh {
  CMPWidget widget;                    /**< Widget interface. */
  M3PullToRefreshStyle style;          /**< Current style. */
  CMPScrollParent scroll_parent;       /**< Scroll parent interface. */
  CMPWidget *child;                    /**< Child widget being scrolled. */
  CMPRect bounds;                      /**< Current layout bounds. */
  CMPScalar drag_offset;               /**< Current pull-down drag offset. */
  CMPBool is_refreshing;               /**< CMP_TRUE if currently refreshing. */
  M3PullToRefreshOnRefresh on_refresh; /**< Refresh callback. */
  void *on_refresh_ctx;                /**< Refresh callback context. */
} M3PullToRefresh;

/**
 * @brief Initialize a pull-to-refresh style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_pull_to_refresh_style_init(M3PullToRefreshStyle *style);

/**
 * @brief Initialize a pull-to-refresh widget.
 * @param ptr Pull-to-refresh instance.
 * @param style Style descriptor.
 * @param child The scrollable child widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_pull_to_refresh_init(M3PullToRefresh *ptr,
                                             const M3PullToRefreshStyle *style,
                                             CMPWidget *child);

/**
 * @brief Set the refreshing state.
 * @param ptr Pull-to-refresh instance.
 * @param is_refreshing CMP_TRUE to start refreshing, CMP_FALSE to stop.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_pull_to_refresh_set_refreshing(M3PullToRefresh *ptr,
                                                       CMPBool is_refreshing);

/**
 * @brief Assign a refresh callback.
 * @param ptr Pull-to-refresh instance.
 * @param on_refresh Callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_pull_to_refresh_set_on_refresh(
    M3PullToRefresh *ptr, M3PullToRefreshOnRefresh on_refresh, void *ctx);

/**
 * @brief Set the scrollable child widget.
 * @param ptr Pull-to-refresh instance.
 * @param child Child widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_pull_to_refresh_set_child(M3PullToRefresh *ptr,
                                                  CMPWidget *child);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_PULL_TO_REFRESH_H */