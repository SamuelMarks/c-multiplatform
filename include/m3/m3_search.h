#ifndef M3_SEARCH_H
#define M3_SEARCH_H

/**
 * @file m3_search.h
 * @brief Material 3 Search Bar and Search View widgets.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "m3_text_field.h"

/**
 * @brief Material 3 Search Bar style descriptor.
 */
typedef struct M3SearchBarStyle {
  M3TextFieldStyle field_style; /**< Core text field styling. */
  CMPColor background_color;    /**< Container background color. */
  CMPScalar corner_radius;      /**< Container corner radius. */
  CMPScalar min_height;         /**< Minimum height (e.g., 56). */
} M3SearchBarStyle;

struct M3SearchBar;

/**
 * @brief Search callback signature.
 * @param ctx User context.
 * @param search_bar The search bar instance.
 * @param utf8_query The current search query.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *M3SearchOnChange)(void *ctx,
                                        struct M3SearchBar *search_bar,
                                        const char *utf8_query);

/**
 * @brief Material 3 Search Bar widget.
 */
typedef struct M3SearchBar {
  CMPWidget widget;            /**< Widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing text. */
  M3SearchBarStyle style;      /**< Search bar styling. */
  M3TextField field;           /**< Internal text field. */
  CMPWidget *leading_icon;     /**< Leading icon (e.g., search or menu). */
  CMPWidget *trailing_icon;    /**< Trailing icon (e.g., clear or mic). */
  CMPRect bounds;              /**< Layout bounds. */
  M3SearchOnChange on_change;  /**< Query change callback. */
  void *on_change_ctx;         /**< Callback context. */
} M3SearchBar;

/**
 * @brief Initialize a search bar style with defaults.
 * @param style Style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_search_bar_style_init(M3SearchBarStyle *style);

/**
 * @brief Initialize a Material 3 search bar.
 * @param search_bar Search bar instance.
 * @param style Style descriptor.
 * @param allocator Allocator for internal text storage.
 * @param text_backend Text backend.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_search_bar_init(M3SearchBar *search_bar,
                                        const M3SearchBarStyle *style,
                                        CMPAllocator allocator,
                                        CMPTextBackend text_backend);

/**
 * @brief Set the search placeholder text.
 * @param search_bar Search bar instance.
 * @param utf8_placeholder Placeholder text.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_search_bar_set_placeholder(
    M3SearchBar *search_bar, const char *utf8_placeholder);

/**
 * @brief Set leading and trailing icons.
 * @param search_bar Search bar instance.
 * @param leading Leading icon widget (may be NULL).
 * @param trailing Trailing icon widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_search_bar_set_icons(M3SearchBar *search_bar,
                                             CMPWidget *leading,
                                             CMPWidget *trailing);

/**
 * @brief Set the change callback.
 * @param search_bar Search bar instance.
 * @param on_change Callback function.
 * @param ctx Callback context.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_search_bar_set_on_change(M3SearchBar *search_bar,
                                                 M3SearchOnChange on_change,
                                                 void *ctx);

/**
 * @brief Test wrapper helper.
 */
CMP_API int CMP_CALL m3_search_test_helper(void);

#ifdef __cplusplus
}
#endif

#endif /* M3_SEARCH_H */
