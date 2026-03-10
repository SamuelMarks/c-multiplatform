#ifndef CUPERTINO_WATCH_BUTTON_H
#define CUPERTINO_WATCH_BUTTON_H

/**
 * @file cupertino_watch_button.h
 * @brief Apple watchOS style Full-Width Pill Button.
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

/** @brief Cupertino watchOS Button Widget */
typedef struct CupertinoWatchButton {
  CMPWidget widget;            /**< Base widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */

  char title_utf8[64]; /**< Button title. */
  cmp_usize title_len; /**< Title length. */

  CMPColor tint_color; /**< Button background color. */
  CMPRect bounds;      /**< Layout bounds. */

  CMPBool is_pressed;  /**< Pressed state. */
  CMPBool is_disabled; /**< Disabled state. */
} CupertinoWatchButton;

/**
 * @brief Initializes a watchOS Button.
 * @param button Pointer to the button instance.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_watch_button_init(
    CupertinoWatchButton *button, const CMPTextBackend *text_backend);

/**
 * @brief Sets the button title.
 * @param button Pointer to the button.
 * @param title_utf8 UTF-8 title text.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_watch_button_set_title(
    CupertinoWatchButton *button, const char *title_utf8);

/**
 * @brief Renders the watchOS button.
 * @param button Pointer to the button.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_watch_button_paint(
    const CupertinoWatchButton *button, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_WATCH_BUTTON_H */
