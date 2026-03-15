#ifndef CUPERTINO_MAC_SIDEBAR_H
#define CUPERTINO_MAC_SIDEBAR_H

/**
 * @file cupertino_mac_sidebar.h
 * @brief Apple macOS style Translucent Sidebar.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"
/* clang-format on */

/** @brief Maximum number of rows in a macOS sidebar. */
#define CUPERTINO_MAC_SIDEBAR_MAX_ROWS 32
/** @brief Maximum length of text for sidebar row titles. */
#define CUPERTINO_MAC_SIDEBAR_MAX_TEXT 64

/** @brief Represents a single row in a macOS sidebar. */
typedef struct CupertinoMacSidebarRow {
  char title_utf8[CUPERTINO_MAC_SIDEBAR_MAX_TEXT]; /**< Title text in UTF-8. */
  cmp_usize title_len;                             /**< Length of title text. */
  CMPBool is_selected;       /**< Whether the row is currently selected. */
  CMPBool is_section_header; /**< Whether the row is a section header. */
} CupertinoMacSidebarRow;

/** @brief Cupertino macOS Sidebar Widget */
typedef struct CupertinoMacSidebar {
  CMPWidget widget;            /**< Base widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */

  CupertinoMacSidebarRow
      rows[CUPERTINO_MAC_SIDEBAR_MAX_ROWS]; /**< List of rows. */
  cmp_usize row_count; /**< Number of rows in the sidebar. */

  CMPRect bounds;           /**< Layout bounds. */
  CMPBool is_dark_mode;     /**< Dark mode styling. */
  CMPBool is_window_active; /**< Whether the window is focused (affects
                               selection color). */
} CupertinoMacSidebar;

/**
 * @brief Initializes a macOS Sidebar.
 * @param sidebar Pointer to the sidebar instance.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_mac_sidebar_init(
    CupertinoMacSidebar *sidebar, const CMPTextBackend *text_backend);

/**
 * @brief Adds an item to the sidebar.
 * @param sidebar Pointer to the sidebar.
 * @param title_utf8 Title text.
 * @param is_section_header CMP_TRUE if this row is a section header
 * (non-selectable, grey text).
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_mac_sidebar_add_item(
    CupertinoMacSidebar *sidebar, const char *title_utf8,
    CMPBool is_section_header);

/**
 * @brief Sets the selected row in the sidebar.
 * @param sidebar Pointer to the sidebar.
 * @param index The index of the row to select.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL
cupertino_mac_sidebar_set_selected(CupertinoMacSidebar *sidebar, cmp_i32 index);

/**
 * @brief Renders the sidebar.
 * @param sidebar Pointer to the sidebar.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_mac_sidebar_paint(
    const CupertinoMacSidebar *sidebar, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_MAC_SIDEBAR_H */
