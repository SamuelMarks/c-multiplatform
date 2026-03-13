#ifndef CUPERTINO_TV_CARD_H
#define CUPERTINO_TV_CARD_H

/**
 * @file cupertino_tv_card.h
 * @brief Apple tvOS style Focusable Card with Parallax.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"
/* clang-format on */

/** @brief Cupertino tvOS Card Widget */
typedef struct CupertinoTVCard {
  CMPWidget widget;            /**< Base widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing. */

  char title_utf8[64]; /**< Card title. */
  cmp_usize title_len; /**< Title length. */

  CMPRect bounds; /**< Layout bounds. */

  CMPBool is_focused;           /**< Focus state (triggers scale animation). */
  CMPAnimController focus_anim; /**< Animation for focus scale (0.0 to 1.0). */

  CMPScalar remote_touch_x; /**< Remote thumb X (-1.0 to 1.0) for parallax. */
  CMPScalar remote_touch_y; /**< Remote thumb Y (-1.0 to 1.0) for parallax. */

  CMPBool is_dark_mode; /**< Dark mode styling. */
} CupertinoTVCard;

/**
 * @brief Initializes a tvOS Card.
 * @param card Pointer to the card instance.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_tv_card_init(CupertinoTVCard *card,
                                            const CMPTextBackend *text_backend);

/**
 * @brief Sets the card title.
 * @param card Pointer to the card.
 * @param title_utf8 UTF-8 title text.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_tv_card_set_title(CupertinoTVCard *card,
                                                 const char *title_utf8);

/**
 * @brief Sets the focus state of the card.
 * @param card Pointer to the card.
 * @param is_focused True if focused.
 * @param animated Whether to animate the transition.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_tv_card_set_focused(CupertinoTVCard *card,
                                                   CMPBool is_focused,
                                                   CMPBool animated);

/**
 * @brief Updates the card's animations.
 * @param card Pointer to the card.
 * @param delta_time Elapsed time in seconds.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_tv_card_update(CupertinoTVCard *card,
                                              double delta_time);

/**
 * @brief Renders the tvOS card.
 * @param card Pointer to the card.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_tv_card_paint(const CupertinoTVCard *card,
                                             CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_TV_CARD_H */
