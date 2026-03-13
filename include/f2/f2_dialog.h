#ifndef F2_DIALOG_H
#define F2_DIALOG_H

/**
 * @file f2_dialog.h
 * @brief Microsoft Fluent 2 Dialog widget.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "f2/f2_color.h"
/* clang-format on */

/** @brief Default dialog corner radius. */
#define F2_DIALOG_DEFAULT_CORNER_RADIUS 8.0f
/** @brief Default dialog padding. */
#define F2_DIALOG_DEFAULT_PADDING 24.0f
/** @brief Default dialog minimum width. */
#define F2_DIALOG_DEFAULT_MIN_WIDTH 288.0f
/** @brief Default dialog maximum width. */
#define F2_DIALOG_DEFAULT_MAX_WIDTH 560.0f

struct F2Dialog;

/**
 * @brief Dialog action callback signature.
 * @param ctx User callback context pointer.
 * @param dialog Dialog instance.
 * @param action_id The action that was triggered.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *F2DialogOnAction)(void *ctx, struct F2Dialog *dialog,
                                        cmp_u32 action_id);

/**
 * @brief Fluent 2 Dialog style descriptor.
 */
typedef struct F2DialogStyle {
  CMPColor background_color; /**< Dialog surface fill color. */
  CMPColor scrim_color;      /**< Background scrim/overlay color. */
  CMPScalar corner_radius;   /**< Corner radius. */
  CMPScalar padding_x;       /**< Horizontal content padding. */
  CMPScalar padding_y;       /**< Vertical content padding. */
  CMPScalar min_width;       /**< Minimum width. */
  CMPScalar max_width;       /**< Maximum width. */
  CMPTextStyle title_style;  /**< Typography for the title. */
  CMPTextStyle body_style;   /**< Typography for the body text. */
} F2DialogStyle;

/**
 * @brief Fluent 2 Dialog widget instance.
 */
typedef struct F2Dialog {
  CMPWidget widget;           /**< Widget interface. */
  F2DialogStyle style;        /**< Current style. */
  const char *utf8_title;     /**< Optional title text. */
  const char *utf8_body;      /**< Optional body text. */
  CMPRect bounds;             /**< Layout bounds. */
  CMPBool is_open;            /**< True if currently open/visible. */
  F2DialogOnAction on_action; /**< Action callback. */
  void *on_action_ctx;        /**< Action callback context. */
} F2Dialog;

/**
 * @brief Initialize a default Fluent 2 dialog style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_dialog_style_init(F2DialogStyle *style);

/**
 * @brief Initialize a Fluent 2 dialog widget.
 * @param dialog Dialog instance.
 * @param style Style descriptor.
 * @param utf8_title Dialog title.
 * @param utf8_body Dialog body message.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_dialog_init(F2Dialog *dialog,
                                    const F2DialogStyle *style,
                                    const char *utf8_title,
                                    const char *utf8_body);

/**
 * @brief Open the dialog.
 * @param dialog Dialog instance.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_dialog_open(F2Dialog *dialog);

/**
 * @brief Close the dialog.
 * @param dialog Dialog instance.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_dialog_close(F2Dialog *dialog);

/**
 * @brief Set the dialog action callback.
 * @param dialog Dialog instance.
 * @param on_action Action callback.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_dialog_set_on_action(F2Dialog *dialog,
                                             F2DialogOnAction on_action,
                                             void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_DIALOG_H */
