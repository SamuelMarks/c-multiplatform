#ifndef CUPERTINO_LIST_SECTION_H
#define CUPERTINO_LIST_SECTION_H

/**
 * @file cupertino_list_section.h
 * @brief Apple iOS style Grouped List Section.
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

/** @brief Maximum number of rows in a list section. */
#define CUPERTINO_LIST_MAX_ROWS 20
/** @brief Maximum length of text for list row title and value. */
#define CUPERTINO_LIST_MAX_TEXT 128

/** @brief Represents a single row in a Cupertino list section. */
typedef struct CupertinoListRow {
  char title_utf8[CUPERTINO_LIST_MAX_TEXT]; /**< Title text in UTF-8. */
  cmp_usize title_len;                      /**< Length of title text. */
  char value_utf8[CUPERTINO_LIST_MAX_TEXT]; /**< Value text in UTF-8. */
  cmp_usize value_len;                      /**< Length of value text. */
  CMPBool has_chevron; /**< Whether the row has a disclosure chevron. */
  CMPBool is_pressed;  /**< Whether the row is currently pressed. */
} CupertinoListRow;

/** @brief Cupertino List Section Widget */
typedef struct CupertinoListSection {
  CMPWidget widget;            /**< Base widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */

  char header_utf8[CUPERTINO_LIST_MAX_TEXT]; /**< Header text in UTF-8. */
  cmp_usize header_len;                      /**< Length of header text. */

  char footer_utf8[CUPERTINO_LIST_MAX_TEXT]; /**< Footer text in UTF-8. */
  cmp_usize footer_len;                      /**< Length of footer text. */

  CupertinoListRow rows[CUPERTINO_LIST_MAX_ROWS]; /**< List of rows. */
  cmp_usize row_count; /**< Number of rows in the section. */

  CMPRect bounds;       /**< Layout bounds. */
  CMPBool is_dark_mode; /**< Dark mode styling. */
} CupertinoListSection;

/**
 * @brief Initializes a Cupertino List Section.
 * @param section Pointer to the list section instance.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_list_section_init(
    CupertinoListSection *section, const CMPTextBackend *text_backend);

/**
 * @brief Sets the header and footer text.
 * @param section Pointer to the section.
 * @param header_utf8 UTF-8 header text (NULL to clear).
 * @param footer_utf8 UTF-8 footer text (NULL to clear).
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_list_section_set_headers(
    CupertinoListSection *section, const char *header_utf8,
    const char *footer_utf8);

/**
 * @brief Adds a row to the list section.
 * @param section Pointer to the section.
 * @param title_utf8 UTF-8 title text.
 * @param value_utf8 UTF-8 value text (optional, shown on right).
 * @param has_chevron CMP_TRUE to show a disclosure chevron.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_list_section_add_row(
    CupertinoListSection *section, const char *title_utf8,
    const char *value_utf8, CMPBool has_chevron);

/**
 * @brief Renders the list section.
 * @param section Pointer to the section.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_list_section_paint(
    const CupertinoListSection *section, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_LIST_SECTION_H */
