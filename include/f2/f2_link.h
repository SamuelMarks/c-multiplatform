#ifndef F2_LINK_H
#define F2_LINK_H

/**
 * @file f2_link.h
 * @brief Microsoft Fluent 2 Link widget.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "f2/f2_color.h"

struct F2Link;

/**
 * @brief Link click callback signature.
 * @param ctx User callback context pointer.
 * @param link Link instance that was activated.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *F2LinkOnClick)(void *ctx, struct F2Link *link);

/**
 * @brief Fluent 2 Link style descriptor.
 */
typedef struct F2LinkStyle {
  CMPTextStyle text_style; /**< Base text style. */
  CMPColor text_color;     /**< Normal text color. */
  CMPColor hover_color;    /**< Hover text color. */
  CMPColor pressed_color;  /**< Pressed text color. */
  CMPColor disabled_color; /**< Disabled text color. */
  CMPBool has_underline;   /**< True if naturally underlined. */
} F2LinkStyle;

/**
 * @brief Fluent 2 Link widget instance.
 */
typedef struct F2Link {
  CMPWidget widget;       /**< Widget interface. */
  F2LinkStyle style;      /**< Current style. */
  const char *utf8_text;  /**< UTF-8 text (may be NULL). */
  cmp_usize utf8_len;     /**< Length of the text. */
  CMPRect bounds;         /**< Layout bounds. */
  CMPBool disabled;       /**< Disabled state. */
  CMPBool hovered;        /**< Hover state. */
  CMPBool pressed;        /**< Pressed state. */
  F2LinkOnClick on_click; /**< Click callback. */
  void *on_click_ctx;     /**< Click callback context. */
} F2Link;

/**
 * @brief Initialize a default Fluent 2 link style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_link_style_init(F2LinkStyle *style);

/**
 * @brief Initialize a Fluent 2 link widget.
 * @param link Link instance.
 * @param style Style descriptor.
 * @param utf8_text UTF-8 label text.
 * @param utf8_len Length of the text.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_link_init(F2Link *link, const F2LinkStyle *style,
                                  const char *utf8_text, cmp_usize utf8_len);

/**
 * @brief Set link text.
 * @param link Link instance.
 * @param utf8_text New text.
 * @param utf8_len Length.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_link_set_text(F2Link *link, const char *utf8_text,
                                      cmp_usize utf8_len);

/**
 * @brief Set link disabled state.
 * @param link Link instance.
 * @param disabled Disabled state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_link_set_disabled(F2Link *link, CMPBool disabled);

/**
 * @brief Assign click callback.
 * @param link Link instance.
 * @param on_click Callback.
 * @param ctx Context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_link_set_on_click(F2Link *link, F2LinkOnClick on_click,
                                          void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_LINK_H */
