#ifndef M3_CARD_H
#define M3_CARD_H

/**
 * @file m3_card.h
 * @brief Card widgets for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_ui.h"
#include "m3_layout.h"
#include "m3_visuals.h"

/** @brief Elevated card variant. */
#define M3_CARD_VARIANT_ELEVATED 1
/** @brief Filled card variant. */
#define M3_CARD_VARIANT_FILLED 2
/** @brief Outlined card variant. */
#define M3_CARD_VARIANT_OUTLINED 3

/** @brief Default horizontal padding for cards. */
#define M3_CARD_DEFAULT_PADDING_X 16.0f
/** @brief Default vertical padding for cards. */
#define M3_CARD_DEFAULT_PADDING_Y 16.0f
/** @brief Default minimum card width. */
#define M3_CARD_DEFAULT_MIN_WIDTH 64.0f
/** @brief Default minimum card height. */
#define M3_CARD_DEFAULT_MIN_HEIGHT 64.0f
/** @brief Default card corner radius. */
#define M3_CARD_DEFAULT_CORNER_RADIUS 12.0f
/** @brief Default outline width for outlined cards. */
#define M3_CARD_DEFAULT_OUTLINE_WIDTH 1.0f
/** @brief Default ripple expansion duration in seconds. */
#define M3_CARD_DEFAULT_RIPPLE_EXPAND 0.2f
/** @brief Default ripple fade duration in seconds. */
#define M3_CARD_DEFAULT_RIPPLE_FADE 0.15f

struct M3Card;

/**
 * @brief Card click callback signature.
 * @param ctx User callback context pointer.
 * @param card Card instance that was activated.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3CardOnClick)(void *ctx, struct M3Card *card);

/**
 * @brief Card style descriptor.
 */
typedef struct M3CardStyle {
  m3_u32 variant;                    /**< Card variant (M3_CARD_VARIANT_*). */
  M3LayoutEdges padding;             /**< Padding around card contents. */
  M3Color background_color;          /**< Background fill color. */
  M3Color outline_color;             /**< Outline color. */
  M3Color ripple_color;              /**< Ripple overlay color. */
  M3Color disabled_background_color; /**< Background color when disabled. */
  M3Color disabled_outline_color;    /**< Outline color when disabled. */
  M3Scalar min_width;              /**< Minimum card width in pixels (>= 0). */
  M3Scalar min_height;             /**< Minimum card height in pixels (>= 0). */
  M3Scalar corner_radius;          /**< Corner radius in pixels (>= 0). */
  M3Scalar outline_width;          /**< Outline width in pixels (>= 0). */
  M3Scalar ripple_expand_duration; /**< Ripple expansion duration in seconds. */
  M3Scalar ripple_fade_duration;   /**< Ripple fade-out duration in seconds. */
  M3Shadow shadow;                 /**< Shadow descriptor for elevated cards. */
  M3Bool shadow_enabled; /**< M3_TRUE when shadow rendering is enabled. */
} M3CardStyle;

/**
 * @brief Card widget instance.
 */
typedef struct M3Card {
  M3Widget widget;        /**< Widget interface (points to this instance). */
  M3CardStyle style;      /**< Current card style. */
  M3Rect bounds;          /**< Layout bounds. */
  M3Ripple ripple;        /**< Ripple state. */
  M3Bool pressed;         /**< M3_TRUE when pressed. */
  M3CardOnClick on_click; /**< Click callback (may be NULL). */
  void *on_click_ctx;     /**< Click callback context pointer. */
} M3Card;

/**
 * @brief Initialize an elevated card style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_card_style_init_elevated(M3CardStyle *style);

/**
 * @brief Initialize a filled card style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_card_style_init_filled(M3CardStyle *style);

/**
 * @brief Initialize an outlined card style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_card_style_init_outlined(M3CardStyle *style);

/**
 * @brief Initialize a card widget.
 * @param card Card instance.
 * @param style Card style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_card_init(M3Card *card, const M3CardStyle *style);

/**
 * @brief Update the card style.
 * @param card Card instance.
 * @param style New card style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_card_set_style(M3Card *card, const M3CardStyle *style);

/**
 * @brief Assign a click callback to the card.
 * @param card Card instance.
 * @param on_click Click callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_card_set_on_click(M3Card *card, M3CardOnClick on_click,
                                        void *ctx);

/**
 * @brief Compute the content bounds inside the card.
 * @param card Card instance.
 * @param out_bounds Receives the content bounds.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_card_get_content_bounds(const M3Card *card,
                                              M3Rect *out_bounds);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_CARD_H */
