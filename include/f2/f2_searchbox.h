#ifndef F2_SEARCHBOX_H
#define F2_SEARCHBOX_H

/**
 * @file f2_searchbox.h
 * @brief Microsoft Fluent 2 SearchBox widget.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "f2/f2_button.h"
#include "f2/f2_color.h"
#include "f2/f2_text_field.h"

/**
 * @brief Fluent 2 SearchBox style descriptor.
 */
typedef struct F2SearchBoxStyle {
  F2TextFieldStyle text_field_style; /**< Core text field styling. */
  F2ButtonStyle clear_button_style;  /**< Styling for the clear button. */
  CMPColor icon_color;               /**< Color of the leading search icon. */
  CMPScalar icon_size;               /**< Size of the leading search icon. */
} F2SearchBoxStyle;

struct F2SearchBox;

/**
 * @brief SearchBox text change callback.
 * @param ctx User context.
 * @param search_box SearchBox instance.
 * @param utf8_text Current text.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *F2SearchBoxOnChange)(void *ctx,
                                           struct F2SearchBox *search_box,
                                           const char *utf8_text);

/**
 * @brief SearchBox explicit search callback (e.g. hitting Enter).
 * @param ctx User context.
 * @param search_box SearchBox instance.
 * @param utf8_text Current text to search for.
 * @return CMP_OK on success.
 */
typedef int(CMP_CALL *F2SearchBoxOnSearch)(void *ctx,
                                           struct F2SearchBox *search_box,
                                           const char *utf8_text);

/**
 * @brief Fluent 2 SearchBox widget.
 */
typedef struct F2SearchBox {
  CMPWidget widget;              /**< Widget interface. */
  F2SearchBoxStyle style;        /**< Current style. */
  F2TextField text_field;        /**< Internal text field widget. */
  F2Button clear_button;         /**< Internal clear button. */
  CMPRect bounds;                /**< Layout bounds. */
  CMPBool has_text;              /**< True if the search box contains text. */
  F2SearchBoxOnChange on_change; /**< Text change callback. */
  void *on_change_ctx;           /**< Text change callback context. */
  F2SearchBoxOnSearch on_search; /**< Search action callback. */
  void *on_search_ctx;           /**< Search action callback context. */
} F2SearchBox;

/**
 * @brief Initialize a default Fluent 2 search box style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_search_box_style_init(F2SearchBoxStyle *style);

/**
 * @brief Initialize a Fluent 2 search box.
 * @param search_box SearchBox instance.
 * @param backend Text backend.
 * @param style Style descriptor.
 * @param allocator Memory allocator for internal text buffers.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_search_box_init(F2SearchBox *search_box,
                                        const CMPTextBackend *backend,
                                        const F2SearchBoxStyle *style,
                                        CMPAllocator *allocator);

/**
 * @brief Set the text of the search box.
 * @param search_box SearchBox instance.
 * @param utf8_text New text.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_search_box_set_text(F2SearchBox *search_box,
                                            const char *utf8_text);

/**
 * @brief Assign callbacks for the search box.
 * @param search_box SearchBox instance.
 * @param on_change Change callback.
 * @param change_ctx Context for change callback.
 * @param on_search Search callback.
 * @param search_ctx Context for search callback.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_search_box_set_callbacks(F2SearchBox *search_box,
                                                 F2SearchBoxOnChange on_change,
                                                 void *change_ctx,
                                                 F2SearchBoxOnSearch on_search,
                                                 void *search_ctx);

/**
 * @brief Clean up the search box.
 * @param search_box SearchBox instance.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_search_box_cleanup(F2SearchBox *search_box);

#ifdef __cplusplus
}
#endif

#endif /* F2_SEARCHBOX_H */
