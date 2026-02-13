#ifndef M3_CARD_H
#define M3_CARD_H

/**
 * @file m3_card.h
 * @brief Card widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_visuals.h"

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
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPCardOnClick)(void *ctx, struct M3Card *card);

/**
 * @brief Card style descriptor.
 */
typedef struct M3CardStyle {
  cmp_u32 variant;                    /**< Card variant (CMP_CARD_VARIANT_*). */
  CMPLayoutEdges padding;             /**< Padding around card contents. */
  CMPColor background_color;          /**< Background fill color. */
  CMPColor outline_color;             /**< Outline color. */
  CMPColor ripple_color;              /**< Ripple overlay color. */
  CMPColor disabled_background_color; /**< Background color when disabled. */
  CMPColor disabled_outline_color;    /**< Outline color when disabled. */
  CMPScalar min_width;              /**< Minimum card width in pixels (>= 0). */
  CMPScalar min_height;             /**< Minimum card height in pixels (>= 0). */
  CMPScalar corner_radius;          /**< Corner radius in pixels (>= 0). */
  CMPScalar outline_width;          /**< Outline width in pixels (>= 0). */
  CMPScalar ripple_expand_duration; /**< Ripple expansion duration in seconds. */
  CMPScalar ripple_fade_duration;   /**< Ripple fade-out duration in seconds. */
  CMPShadow shadow;                 /**< Shadow descriptor for elevated cards. */
  CMPBool shadow_enabled; /**< CMP_TRUE when shadow rendering is enabled. */
} M3CardStyle;

/**
 * @brief Card widget instance.
 */
typedef struct M3Card {
  CMPWidget widget;        /**< Widget interface (points to this instance). */
  M3CardStyle style;      /**< Current card style. */
  CMPRect bounds;          /**< Layout bounds. */
  CMPRipple ripple;        /**< Ripple state. */
  CMPBool pressed;         /**< CMP_TRUE when pressed. */
  CMPCardOnClick on_click; /**< Click callback (may be NULL). */
  void *on_click_ctx;     /**< Click callback context pointer. */
} M3Card;

/**
 * @brief Initialize an elevated card style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_card_style_init_elevated(M3CardStyle *style);

/**
 * @brief Initialize a filled card style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_card_style_init_filled(M3CardStyle *style);

/**
 * @brief Initialize an outlined card style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_card_style_init_outlined(M3CardStyle *style);

/**
 * @brief Initialize a card widget.
 * @param card Card instance.
 * @param style Card style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_card_init(M3Card *card, const M3CardStyle *style);

/**
 * @brief Update the card style.
 * @param card Card instance.
 * @param style New card style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_card_set_style(M3Card *card, const M3CardStyle *style);

/**
 * @brief Assign a click callback to the card.
 * @param card Card instance.
 * @param on_click Click callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_card_set_on_click(M3Card *card, CMPCardOnClick on_click,
                                        void *ctx);

/**
 * @brief Compute the content bounds inside the card.
 * @param card Card instance.
 * @param out_bounds Receives the content bounds.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_card_get_content_bounds(const M3Card *card,
                                              CMPRect *out_bounds);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_CARD_H */
