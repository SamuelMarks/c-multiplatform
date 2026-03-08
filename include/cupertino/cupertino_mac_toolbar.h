#ifndef CUPERTINO_MAC_TOOLBAR_H
#define CUPERTINO_MAC_TOOLBAR_H

/**
 * @file cupertino_mac_toolbar.h
 * @brief Apple macOS style Window Toolbar.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"

/** @brief Maximum number of items in a macOS toolbar. */
#define CUPERTINO_MAC_TOOLBAR_MAX_ITEMS 16

/** @brief Represents an item in a macOS toolbar. */
typedef struct CupertinoMacToolbarItem {
    char title_utf8[64];       /**< Title text in UTF-8. */
    cmp_usize title_len;       /**< Length of title text. */
    CMPBool is_search;         /**< Whether the item is a search field. */
    CMPBool is_flexible_space; /**< Whether the item is a flexible spacer. */
} CupertinoMacToolbarItem;

/** @brief Cupertino macOS Toolbar Widget */
typedef struct CupertinoMacToolbar {
    CMPWidget widget;                             /**< Base widget interface. */
    CMPTextBackend text_backend;                  /**< Text backend for drawing labels. */
    
    CupertinoMacToolbarItem items[CUPERTINO_MAC_TOOLBAR_MAX_ITEMS]; /**< List of toolbar items. */
    cmp_usize item_count;                         /**< Number of items in the toolbar. */
    
    CMPRect bounds;                               /**< Layout bounds. */
    CMPBool is_dark_mode;                         /**< Dark mode styling. */
    CMPBool is_window_active;                     /**< Whether the window is focused. */
} CupertinoMacToolbar;

/**
 * @brief Initializes a macOS Toolbar.
 * @param toolbar Pointer to the toolbar instance.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_mac_toolbar_init(CupertinoMacToolbar *toolbar, const CMPTextBackend *text_backend);

/**
 * @brief Adds a standard button item to the toolbar.
 * @param toolbar Pointer to the toolbar.
 * @param title_utf8 Title text.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_mac_toolbar_add_item(CupertinoMacToolbar *toolbar, const char *title_utf8);

/**
 * @brief Adds a flexible space to push subsequent items to the right.
 * @param toolbar Pointer to the toolbar.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_mac_toolbar_add_flexible_space(CupertinoMacToolbar *toolbar);

/**
 * @brief Adds a search box placeholder to the toolbar.
 * @param toolbar Pointer to the toolbar.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_mac_toolbar_add_search(CupertinoMacToolbar *toolbar);

/**
 * @brief Renders the toolbar.
 * @param toolbar Pointer to the toolbar.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_mac_toolbar_paint(const CupertinoMacToolbar *toolbar, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_MAC_TOOLBAR_H */
