#ifndef CUPERTINO_MENU_H
#define CUPERTINO_MENU_H

/**
 * @file cupertino_menu.h
 * @brief Apple Cupertino Context Menu (NSMenu / UIContextMenuInteraction).
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

/** @brief Cupertino Menu variant */
typedef enum CupertinoMenuVariant {
  CUPERTINO_MENU_MAC_OS =
      0,             /**< Standard macOS drop shadow desktop context menu. */
  CUPERTINO_MENU_IOS /**< iOS blurred context menu. */
} CupertinoMenuVariant;

/** @brief Cupertino Menu Item Style */
typedef enum CupertinoMenuItemStyle {
  CUPERTINO_MENU_ITEM_DEFAULT = 0,
  CUPERTINO_MENU_ITEM_DESTRUCTIVE,
  CUPERTINO_MENU_ITEM_SEPARATOR
} CupertinoMenuItemStyle;

/** @brief Descriptor for a single item in the menu */
typedef struct CupertinoMenuItem {
  const char *title_utf8;    /**< UTF-8 string for the title. */
  cmp_usize title_len;       /**< Length of the title string in bytes. */
  const char *shortcut_utf8; /**< E.g., "⌘C" (macOS only) */
  cmp_usize shortcut_len;    /**< Length of the shortcut string in bytes. */
  CupertinoMenuItemStyle
      style;           /**< Item style (default, destructive, separator). */
  CMPBool is_disabled; /**< If true, the item is disabled and unselectable. */
  void *user_data;     /**< Custom user data associated with the item. */
} CupertinoMenuItem;

/** @brief Menu style descriptor */
typedef struct CupertinoMenuStyle {
  CupertinoMenuVariant variant;       /**< Menu variant (macOS or iOS). */
  CupertinoBlurStyle background_blur; /**< Used heavily on iOS, partially on
                                         macOS (BehindWindow). */
  CMPColor tint_color; /**< Hover/selection background color (usually blue). */
  CMPColor text_color; /**< Standard text color. */
  CMPColor destructive_color;  /**< Text color for destructive items. */
  CMPBool is_dark_mode;        /**< If true, renders the menu in dark mode. */
  CMPTextStyle label_style;    /**< Text style for menu item labels. */
  CMPTextStyle shortcut_style; /**< Text style for menu item shortcuts. */
} CupertinoMenuStyle;

/** @brief Menu widget instance */
typedef struct CupertinoMenu {
  CMPWidget widget;            /**< Widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */
  CupertinoMenuStyle style;    /**< Visual style configuration for the menu. */

  CMPRect bounds;      /**< The calculated bounds of the menu. */
  CMPRect anchor_rect; /**< The rect that spawned the menu. */

  CupertinoMenuItem *items; /**< Array of items. */
  cmp_u32 item_count;       /**< Number of items. */

  int hovered_index; /**< Currently hovered item index (-1 for none). */
  CMPScalar animation_progress; /**< 0.0 to 1.0 (fade/scale in). */
} CupertinoMenu;

/**
 * @brief Initialize a Cupertino menu style.
 * @param style Style to initialize.
 * @param variant iOS or macOS variant.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_menu_style_init(CupertinoMenuStyle *style,
                                               CupertinoMenuVariant variant);

/**
 * @brief Initialize a Cupertino menu widget.
 * @param menu Menu to initialize.
 * @param text_backend Text backend.
 * @param variant iOS or macOS variant.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_menu_init(CupertinoMenu *menu,
                                         const CMPTextBackend *text_backend,
                                         CupertinoMenuVariant variant);

/**
 * @brief Set the items for the menu.
 * @param menu Menu instance.
 * @param items Array of items (copied internally or pointer retained).
 * @param count Number of items.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_menu_set_items(CupertinoMenu *menu,
                                              CupertinoMenuItem *items,
                                              cmp_u32 count);

/**
 * @brief Calculate the size of the menu.
 * @param menu Menu instance.
 * @param out_size Pointer to receive the calculated size.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_menu_measure(CupertinoMenu *menu,
                                            CMPSize *out_size);

/**
 * @brief Layout the menu relative to an anchor.
 * @param menu Menu instance.
 * @param anchor_rect The screen rect of the widget that spawned this menu.
 * @param screen_bounds The full screen bounds to constrain the menu within.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_menu_layout(CupertinoMenu *menu,
                                           CMPRect anchor_rect,
                                           CMPRect screen_bounds);

/**
 * @brief Set the currently hovered item index (for macOS hover states).
 * @param menu Menu instance.
 * @param index Hovered index (-1 to clear).
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_menu_set_hover(CupertinoMenu *menu, int index);

/**
 * @brief Paint the menu.
 * @param menu Menu instance.
 * @param ctx Paint context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_menu_paint(const CupertinoMenu *menu,
                                          CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_MENU_H */
