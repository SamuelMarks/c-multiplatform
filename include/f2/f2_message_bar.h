#ifndef F2_MESSAGE_BAR_H
#define F2_MESSAGE_BAR_H

/**
 * @file f2_message_bar.h
 * @brief Microsoft Fluent 2 MessageBar and Toast widgets.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "f2/f2_color.h"

/** @brief Default message bar padding. */
#define F2_MESSAGE_BAR_DEFAULT_PADDING 12.0f
/** @brief Default message bar corner radius. */
#define F2_MESSAGE_BAR_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Default toast minimum width. */
#define F2_TOAST_DEFAULT_MIN_WIDTH 280.0f
/** @brief Default toast maximum width. */
#define F2_TOAST_DEFAULT_MAX_WIDTH 400.0f

/**
 * @brief Intent variants for MessageBar / Toast.
 */
typedef enum F2Intent {
  F2_INTENT_INFO = 0,
  F2_INTENT_SUCCESS,
  F2_INTENT_WARNING,
  F2_INTENT_ERROR
} F2Intent;

struct F2MessageBar;

/**
 * @brief MessageBar action callback.
 * @param ctx User callback context.
 * @param bar MessageBar instance.
 * @return CMP_OK on success.
 */
typedef int(CMP_CALL *F2MessageBarOnAction)(void *ctx,
                                            struct F2MessageBar *bar);

/**
 * @brief Fluent 2 MessageBar style descriptor.
 */
typedef struct F2MessageBarStyle {
  F2Intent intent;           /**< Intent defining the color semantics. */
  CMPColor background_color; /**< Computed background color. */
  CMPColor text_color;       /**< Computed text color. */
  CMPColor icon_color;       /**< Computed icon color. */
  CMPTextStyle text_style;   /**< Text typography. */
  CMPScalar padding;         /**< Content padding. */
  CMPScalar corner_radius;   /**< Corner radius. */
  CMPBool has_action;        /**< True if it has an action button. */
  CMPBool has_dismiss;       /**< True if it has a dismiss button. */
} F2MessageBarStyle;

/**
 * @brief Fluent 2 MessageBar widget instance.
 */
typedef struct F2MessageBar {
  CMPWidget widget;                /**< Widget interface. */
  F2MessageBarStyle style;         /**< Current style. */
  const char *utf8_text;           /**< Message text. */
  cmp_usize text_len;              /**< Text length. */
  const char *utf8_action;         /**< Action button text (if has_action). */
  cmp_usize action_len;            /**< Action length. */
  CMPRect bounds;                  /**< Layout bounds. */
  CMPBool is_visible;              /**< True if currently visible. */
  F2MessageBarOnAction on_action;  /**< Action callback. */
  void *on_action_ctx;             /**< Action context. */
  F2MessageBarOnAction on_dismiss; /**< Dismiss callback. */
  void *on_dismiss_ctx;            /**< Dismiss context. */
} F2MessageBar;

/**
 * @brief Initialize a default Fluent 2 message bar style based on an intent.
 * @param style Style descriptor.
 * @param intent Information intent (Info, Success, Warning, Error).
 * @param theme Active Fluent 2 theme used for extracting semantics.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_message_bar_style_init(F2MessageBarStyle *style,
                                               F2Intent intent,
                                               const F2Theme *theme);

/**
 * @brief Initialize a Fluent 2 message bar widget.
 * @param bar MessageBar instance.
 * @param style Style descriptor.
 * @param utf8_text Message text.
 * @param text_len Text length.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_message_bar_init(F2MessageBar *bar,
                                         const F2MessageBarStyle *style,
                                         const char *utf8_text,
                                         cmp_usize text_len);

/**
 * @brief Show the message bar.
 * @param bar MessageBar instance.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_message_bar_show(F2MessageBar *bar);

/**
 * @brief Hide the message bar.
 * @param bar MessageBar instance.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_message_bar_hide(F2MessageBar *bar);

/**
 * @brief Set the action button configuration.
 * @param bar MessageBar instance.
 * @param utf8_action Action text.
 * @param action_len Text length.
 * @param on_action Action callback.
 * @param ctx Action context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_message_bar_set_action(F2MessageBar *bar,
                                               const char *utf8_action,
                                               cmp_usize action_len,
                                               F2MessageBarOnAction on_action,
                                               void *ctx);

/**
 * @brief Set the dismiss button configuration.
 * @param bar MessageBar instance.
 * @param on_dismiss Dismiss callback.
 * @param ctx Dismiss context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_message_bar_set_dismiss(F2MessageBar *bar,
                                                F2MessageBarOnAction on_dismiss,
                                                void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_MESSAGE_BAR_H */
