#ifndef CUPERTINO_TAB_BAR_H
#define CUPERTINO_TAB_BAR_H

/**
 * @file cupertino_tab_bar.h
 * @brief Apple Cupertino Tab Bar (UITabBar) with bottom navigation
 * capabilities.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_blur.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"
/* clang-format on */

#define CUPERTINO_TAB_BAR_MAX_ITEMS                                            \
  5 /**< Maximum allowed items in a tab bar                                    \
     */

/** @brief Tab Bar item descriptor */
typedef struct CupertinoTabBarItem {
  const char *title_utf8; /**< Item label text */
  cmp_usize title_len;    /**< Item label length */
  /* In a real implementation this would hold an icon/SF Symbol handle */
  CMPColor badge_color; /**< Notification badge color */
  CMPBool has_badge;    /**< CMP_TRUE if a badge should be shown */
} CupertinoTabBarItem;

/** @brief Tab Bar style descriptor */
typedef struct CupertinoTabBarStyle {
  CupertinoBlurStyle background_blur; /**< Background blur material */
  CMPColor tint_color;                /**< Selected item color */
  CMPColor unselected_color;          /**< Unselected item color */
  CMPBool is_dark_mode;               /**< Active dark mode state */
  CMPTextStyle label_style;           /**< Item label text style */
} CupertinoTabBarStyle;

/** @brief Tab Bar widget instance */
typedef struct CupertinoTabBar {
  CMPWidget widget;            /**< Widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */
  CupertinoTabBarStyle style;  /**< Configured style. */
  CMPRect bounds;              /**< Layout bounds. */
  CMPRect safe_area_insets; /**< OS safe area insets (bottom home indicator). */

  CupertinoTabBarItem items[CUPERTINO_TAB_BAR_MAX_ITEMS]; /**< Child items */
  cmp_u32 item_count;     /**< Number of active items */
  cmp_u32 selected_index; /**< Currently selected item index */
} CupertinoTabBar;

/**
 * @brief Initialize a Cupertino tab bar style.
 * @param style Style to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_tab_bar_style_init(CupertinoTabBarStyle *style);

/**
 * @brief Initialize a Cupertino tab bar widget.
 * @param bar Tab bar to initialize.
 * @param text_backend Text backend.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_tab_bar_init(CupertinoTabBar *bar,
                                            const CMPTextBackend *text_backend);

/**
 * @brief Set the items for the tab bar.
 * @param bar Tab bar instance.
 * @param items Array of items.
 * @param count Number of items (max CUPERTINO_TAB_BAR_MAX_ITEMS).
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_tab_bar_set_items(
    CupertinoTabBar *bar, const CupertinoTabBarItem *items, cmp_u32 count);

/**
 * @brief Layout the tab bar.
 * @param bar Tab bar instance.
 * @param bounds The layout bounds.
 * @param safe_area Safe area insets (bottom).
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_tab_bar_layout(CupertinoTabBar *bar,
                                              CMPRect bounds,
                                              CMPRect safe_area);

/**
 * @brief Paint the tab bar.
 * @param bar Tab bar instance.
 * @param ctx Paint context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_tab_bar_paint(const CupertinoTabBar *bar,
                                             CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_TAB_BAR_H */
