#ifndef CUPERTINO_ALERT_H
#define CUPERTINO_ALERT_H

/**
 * @file cupertino_alert.h
 * @brief Apple Cupertino style Alert component.
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
#include "cupertino/cupertino_button.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"
/* clang-format on */

/** @brief Maximum number of actions an alert can have. */
#define CUPERTINO_ALERT_MAX_ACTIONS 4

/** @brief Cupertino Alert Widget */
typedef struct CupertinoAlert {
  CMPWidget widget;            /**< Base widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */
  const char *title_utf8;      /**< Title text. */
  cmp_usize title_len;         /**< Title text length. */
  const char *message_utf8;    /**< Message text. */
  cmp_usize message_len;       /**< Message text length. */

  CupertinoButton
      actions[CUPERTINO_ALERT_MAX_ACTIONS]; /**< List of alert actions. */
  cmp_usize action_count;                   /**< Number of actions added. */

  CMPRect bounds;         /**< Layout bounds. */
  CMPColor bg_color;      /**< Alert background color. */
  CMPColor divider_color; /**< Separator line color. */
  CMPBool is_dark_mode;   /**< Dark mode styling. */
} CupertinoAlert;

/**
 * @brief Initializes a Cupertino Alert.
 * @param alert Pointer to the alert instance to initialize.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_alert_init(CupertinoAlert *alert,
                                          const CMPTextBackend *text_backend);

/**
 * @brief Sets the title and message of the alert.
 * @param alert Pointer to the alert instance.
 * @param title_utf8 UTF-8 encoded title string.
 * @param message_utf8 UTF-8 encoded message string.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_alert_set_content(CupertinoAlert *alert,
                                                 const char *title_utf8,
                                                 const char *message_utf8);

/**
 * @brief Adds an action button to the alert.
 * @param alert Pointer to the alert instance.
 * @param label_utf8 Label of the action.
 * @param is_destructive CMP_TRUE if this is a destructive action (usually red).
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_alert_add_action(CupertinoAlert *alert,
                                                const char *label_utf8,
                                                CMPBool is_destructive);

/**
 * @brief Renders the alert.
 * @param alert Pointer to the alert instance.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_alert_paint(const CupertinoAlert *alert,
                                           CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_ALERT_H */
