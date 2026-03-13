#ifndef CUPERTINO_ACTION_SHEET_H
#define CUPERTINO_ACTION_SHEET_H

/**
 * @file cupertino_action_sheet.h
 * @brief Apple Cupertino Action Sheet (UIAlertControllerStyleActionSheet).
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_blur.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"
/* clang-format on */

#define CUPERTINO_ACTION_SHEET_MAX_ACTIONS                                     \
  8 /**< Max actions allowed in one sheet */

/** @brief Action sheet button style */
typedef enum CupertinoActionStyle {
  CUPERTINO_ACTION_STYLE_DEFAULT = 0,
  CUPERTINO_ACTION_STYLE_CANCEL,
  CUPERTINO_ACTION_STYLE_DESTRUCTIVE
} CupertinoActionStyle;

/** @brief Descriptor for a single action in the sheet */
typedef struct CupertinoAction {
  const char *title_utf8;     /**< Action label text */
  cmp_usize title_len;        /**< Label length */
  CupertinoActionStyle style; /**< Action visual style */
  void *user_data;            /**< User data passed to callback */
} CupertinoAction;

/** @brief Action sheet style descriptor */
typedef struct CupertinoActionSheetStyle {
  CupertinoBlurStyle background_blur; /**< Main backdrop material */
  CMPColor tint_color;                /**< Standard action text color */
  CMPColor destructive_color;         /**< Destructive action text color */
  CMPBool is_dark_mode;               /**< Dark mode active state */
  CMPTextStyle title_style;           /**< Header title text style */
  CMPTextStyle message_style;         /**< Header message text style */
  CMPTextStyle action_style;          /**< Action button text style */
} CupertinoActionSheetStyle;

/** @brief Action sheet widget instance */
typedef struct CupertinoActionSheet {
  CMPWidget widget;                /**< Widget interface. */
  CMPTextBackend text_backend;     /**< Text backend for drawing labels. */
  CupertinoActionSheetStyle style; /**< Configured style. */
  CMPRect bounds;                  /**< Full screen layout bounds. */
  CMPRect safe_area_insets;        /**< Screen safe area insets. */

  const char *title_utf8;   /**< Optional header title */
  cmp_usize title_len;      /**< Length of header title */
  const char *message_utf8; /**< Optional header message */
  cmp_usize message_len;    /**< Length of header message */

  CupertinoAction
      actions[CUPERTINO_ACTION_SHEET_MAX_ACTIONS]; /**< Configured actions */
  cmp_u32 action_count; /**< Number of active actions */

  CMPScalar animation_progress; /**< 0.0 (hidden) to 1.0 (fully visible) */
} CupertinoActionSheet;

/**
 * @brief Initialize a Cupertino action sheet style.
 * @param style Style to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL
cupertino_action_sheet_style_init(CupertinoActionSheetStyle *style);

/**
 * @brief Initialize a Cupertino action sheet widget.
 * @param sheet Action sheet to initialize.
 * @param text_backend Text backend.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_action_sheet_init(
    CupertinoActionSheet *sheet, const CMPTextBackend *text_backend);

/**
 * @brief Set the optional title and message for the action sheet.
 * @param sheet Action sheet instance.
 * @param title_utf8 Title string (or NULL).
 * @param title_len Title length.
 * @param msg_utf8 Message string (or NULL).
 * @param msg_len Message length.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_action_sheet_set_text(
    CupertinoActionSheet *sheet, const char *title_utf8, cmp_usize title_len,
    const char *msg_utf8, cmp_usize msg_len);

/**
 * @brief Add an action button to the sheet.
 * @param sheet Action sheet instance.
 * @param action Action descriptor.
 * @return CMP_OK on success, CMP_ERR_RANGE if maximum actions reached.
 */
CMP_API int CMP_CALL cupertino_action_sheet_add_action(
    CupertinoActionSheet *sheet, const CupertinoAction *action);

/**
 * @brief Layout the action sheet within the full screen bounds.
 * @param sheet Action sheet instance.
 * @param bounds Full screen bounds.
 * @param safe_area Safe area insets (bottom inset is critical).
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_action_sheet_layout(CupertinoActionSheet *sheet,
                                                   CMPRect bounds,
                                                   CMPRect safe_area);

/**
 * @brief Paint the action sheet.
 * @param sheet Action sheet instance.
 * @param ctx Paint context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_action_sheet_paint(
    const CupertinoActionSheet *sheet, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_ACTION_SHEET_H */
