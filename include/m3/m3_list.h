#ifndef M3_LIST_H
#define M3_LIST_H

/**
 * @file m3_list.h
 * @brief Material 3 List Item widgets.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "cmpc/cmp_visuals.h"

/** @brief 1-line list item variant. */
#define M3_LIST_ITEM_VARIANT_1_LINE 1
/** @brief 2-line list item variant. */
#define M3_LIST_ITEM_VARIANT_2_LINE 2
/** @brief 3-line list item variant. */
#define M3_LIST_ITEM_VARIANT_3_LINE 3

/** @brief Default horizontal padding. */
#define M3_LIST_ITEM_DEFAULT_PADDING_X 16.0f

/**
 * @brief Material 3 List Item style descriptor.
 */
typedef struct M3ListItemStyle {
  cmp_u32 variant;               /**< Variant (M3_LIST_ITEM_VARIANT_*). */
  CMPTextStyle headline_style;   /**< Headline text style. */
  CMPTextStyle supporting_style; /**< Supporting text style. */
  CMPTextStyle trailing_style;   /**< Trailing text style. */
  CMPColor background_color;     /**< Idle background color. */
  CMPColor ripple_color;         /**< Ripple color on press. */
  CMPColor icon_color;           /**< Default icon color. */
  CMPScalar min_height;          /**< Minimum height (e.g., 56, 72, 88). */
  CMPScalar padding_x;           /**< Horizontal padding. */
} M3ListItemStyle;

struct M3ListItem;

/**
 * @brief List item press callback.
 * @param ctx User context.
 * @param item The list item instance.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *M3ListItemOnPress)(void *ctx, struct M3ListItem *item);

/**
 * @brief Material 3 List Item widget.
 */
typedef struct M3ListItem {
  CMPWidget widget;            /**< Widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing text. */
  M3ListItemStyle style;       /**< Item styling. */
  const char *utf8_headline;   /**< Headline text (UTF-8). */
  const char *utf8_supporting; /**< Supporting text (UTF-8). */
  const char *utf8_trailing;   /**< Trailing text (UTF-8). */
  CMPWidget *leading_widget;   /**< Optional leading widget (e.g. icon). */
  CMPWidget *trailing_widget;  /**< Optional trailing widget (e.g. switch). */
  CMPRect bounds;              /**< Layout bounds. */
  CMPRipple ripple;            /**< Ripple state. */
  CMPBool pressed;             /**< CMP_TRUE when pressed. */
  M3ListItemOnPress on_press;  /**< Press callback. */
  void *on_press_ctx;          /**< Callback context. */
} M3ListItem;

/**
 * @brief Initialize a list item style with defaults.
 * @param style Style descriptor.
 * @param variant The list item variant (1-line, 2-line, or 3-line).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_list_item_style_init(M3ListItemStyle *style,
                                             cmp_u32 variant);

/**
 * @brief Initialize a Material 3 list item.
 * @param item List item instance.
 * @param style Style descriptor.
 * @param text_backend Text backend.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_list_item_init(M3ListItem *item,
                                       const M3ListItemStyle *style,
                                       CMPTextBackend text_backend);

/**
 * @brief Set the headline text.
 * @param item List item instance.
 * @param utf8_text Headline text.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_list_item_set_headline(M3ListItem *item,
                                               const char *utf8_text);

/**
 * @brief Set the supporting text.
 * @param item List item instance.
 * @param utf8_text Supporting text.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_list_item_set_supporting(M3ListItem *item,
                                                 const char *utf8_text);

/**
 * @brief Set the trailing text.
 * @param item List item instance.
 * @param utf8_text Trailing text.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_list_item_set_trailing(M3ListItem *item,
                                               const char *utf8_text);

/**
 * @brief Set leading and trailing widgets.
 * @param item List item instance.
 * @param leading Leading widget (may be NULL).
 * @param trailing Trailing widget (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_list_item_set_widgets(M3ListItem *item,
                                              CMPWidget *leading,
                                              CMPWidget *trailing);

/**
 * @brief Set the press callback.
 * @param item List item instance.
 * @param on_press Callback function.
 * @param ctx Callback context.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_list_item_set_on_press(M3ListItem *item,
                                               M3ListItemOnPress on_press,
                                               void *ctx);

/**
 * @brief Test wrapper helper.
 */
CMP_API int CMP_CALL m3_list_test_helper(void);

#ifdef __cplusplus
}
#endif

#endif /* M3_LIST_H */
