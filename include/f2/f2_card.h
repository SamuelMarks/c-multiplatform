#ifndef F2_CARD_H
#define F2_CARD_H

/**
 * @file f2_card.h
 * @brief Microsoft Fluent 2 Card widgets.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "f2/f2_color.h"

/** @brief Default card padding. */
#define F2_CARD_DEFAULT_PADDING 12.0f
/** @brief Default card corner radius. */
#define F2_CARD_DEFAULT_CORNER_RADIUS 8.0f
/** @brief Default card minimum size. */
#define F2_CARD_DEFAULT_MIN_SIZE 64.0f

struct F2Card;

/**
 * @brief Card click callback signature.
 * @param ctx User callback context pointer.
 * @param card Card instance that was activated.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *F2CardOnClick)(void *ctx, struct F2Card *card);

/**
 * @brief Fluent 2 Card style descriptor.
 */
typedef struct F2CardStyle {
  CMPBool is_clickable;        /**< Whether card supports interaction. */
  CMPColor background_color;   /**< Base fill color. */
  CMPColor border_color;       /**< Optional border stroke. */
  CMPColor hover_border_color; /**< Border stroke when hovered. */
  CMPColor pressed_color;      /**< Fill color when pressed. */
  CMPScalar corner_radius;     /**< Corner radius. */
  CMPScalar padding;           /**< Content padding. */
  CMPScalar min_width;         /**< Minimum width. */
  CMPScalar min_height;        /**< Minimum height. */
  CMPScalar border_width;      /**< Border thickness. */
} F2CardStyle;

/**
 * @brief Fluent 2 Card widget instance.
 */
typedef struct F2Card {
  CMPWidget widget;         /**< Widget interface. */
  F2CardStyle style;        /**< Current style. */
  CMPRect bounds;           /**< Layout bounds. */
  CMPBool hovered;          /**< Hover state. */
  CMPBool pressed;          /**< Pressed state. */
  F2CardOnClick on_click;   /**< Click callback. */
  void *on_click_ctx;       /**< Click callback context. */
} F2Card;

/**
 * @brief Initialize a default Fluent 2 card style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_card_style_init(F2CardStyle *style);

/**
 * @brief Initialize a Fluent 2 card widget.
 * @param card Card instance.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_card_init(F2Card *card, const F2CardStyle *style);

/**
 * @brief Set the card click callback.
 * @param card Card instance.
 * @param on_click Callback function.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_card_set_on_click(F2Card *card, F2CardOnClick on_click, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_CARD_H */
