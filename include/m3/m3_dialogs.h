#ifndef M3_DIALOGS_H
#define M3_DIALOGS_H

/**
 * @file m3_dialogs.h
 * @brief Dialog and snackbar widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_text.h"
#include "cmpc/cmp_visuals.h"

/** @brief Alert dialog confirm action identifier. */
#define M3_ALERT_DIALOG_ACTION_CONFIRM 1u
/** @brief Alert dialog dismiss action identifier. */
#define M3_ALERT_DIALOG_ACTION_DISMISS 2u

/** @brief Default alert dialog corner radius in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_CORNER_RADIUS 12.0f
/** @brief Default alert dialog minimum width in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_MIN_WIDTH 280.0f
/** @brief Default alert dialog maximum width in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_MAX_WIDTH 560.0f
/** @brief Default alert dialog horizontal padding in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_PADDING_X 24.0f
/** @brief Default alert dialog vertical padding in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_PADDING_Y 24.0f
/** @brief Default spacing between title and body in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_TITLE_BODY_SPACING 16.0f
/** @brief Default spacing between body and actions in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_BODY_ACTION_SPACING 24.0f
/** @brief Default spacing between action buttons in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_ACTION_SPACING 8.0f
/** @brief Default action horizontal padding in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_ACTION_PADDING_X 12.0f
/** @brief Default action vertical padding in pixels. */
#define M3_ALERT_DIALOG_DEFAULT_ACTION_PADDING_Y 8.0f

struct M3AlertDialog;

/**
 * @brief Alert dialog action callback signature.
 * @param ctx User callback context pointer.
 * @param dialog Alert dialog instance that triggered the action.
 * @param action_id Action identifier (CMP_ALERT_DIALOG_ACTION_*).
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPAlertDialogOnAction)(void *ctx,
                                            struct M3AlertDialog *dialog,
                                            cmp_u32 action_id);

/**
 * @brief Alert dialog style descriptor.
 */
typedef struct M3AlertDialogStyle {
  CMPLayoutEdges padding;       /**< Padding around dialog contents. */
  CMPScalar corner_radius;      /**< Corner radius in pixels (>= 0). */
  CMPScalar min_width;          /**< Minimum dialog width in pixels (>= 0). */
  CMPScalar max_width;          /**< Maximum dialog width in pixels (>= 0). */
  CMPScalar title_body_spacing; /**< Spacing between title and body in pixels (>=
                                  0). */
  CMPScalar body_action_spacing; /**< Spacing between body and actions in pixels
                                   (>= 0). */
  CMPScalar
      action_spacing; /**< Spacing between action buttons in pixels (>= 0). */
  CMPScalar
      action_padding_x; /**< Horizontal padding for action buttons (>= 0). */
  CMPScalar action_padding_y; /**< Vertical padding for action buttons (>= 0). */
  CMPTextStyle title_style;   /**< Text style for the title. */
  CMPTextStyle body_style;    /**< Text style for the body. */
  CMPTextStyle action_style;  /**< Text style for action labels. */
  CMPColor background_color;  /**< Dialog background color. */
  CMPColor scrim_color;       /**< Scrim color used behind the dialog. */
  CMPShadow shadow;           /**< Shadow descriptor. */
  CMPBool shadow_enabled;     /**< CMP_TRUE when shadow rendering is enabled. */
  CMPBool scrim_enabled;      /**< CMP_TRUE when scrim rendering is enabled. */
} M3AlertDialogStyle;

/**
 * @brief Alert dialog widget instance.
 */
typedef struct M3AlertDialog {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend;    /**< Text backend instance. */
  M3AlertDialogStyle style;      /**< Current dialog style. */
  CMPHandle title_font;           /**< Font handle for the title text. */
  CMPHandle body_font;            /**< Font handle for the body text. */
  CMPHandle action_font;          /**< Font handle for action labels. */
  CMPTextMetrics title_metrics;   /**< Cached title metrics. */
  CMPTextMetrics body_metrics;    /**< Cached body metrics. */
  CMPTextMetrics confirm_metrics; /**< Cached confirm action metrics. */
  CMPTextMetrics dismiss_metrics; /**< Cached dismiss action metrics. */
  const char *utf8_title;        /**< UTF-8 title text (may be NULL). */
  cmp_usize title_len;            /**< Title length in bytes. */
  const char *utf8_body;         /**< UTF-8 body text (may be NULL). */
  cmp_usize body_len;             /**< Body length in bytes. */
  const char *utf8_confirm;      /**< UTF-8 confirm label (may be NULL). */
  cmp_usize confirm_len;          /**< Confirm label length in bytes. */
  const char *utf8_dismiss;      /**< UTF-8 dismiss label (may be NULL). */
  cmp_usize dismiss_len;          /**< Dismiss label length in bytes. */
  CMPRect bounds;                 /**< Layout bounds. */
  CMPRect confirm_bounds;         /**< Bounds for confirm action hit testing. */
  CMPRect dismiss_bounds;         /**< Bounds for dismiss action hit testing. */
  CMPBool metrics_valid;          /**< CMP_TRUE when cached metrics are valid. */
  CMPBool owns_fonts;             /**< CMP_TRUE when widget owns font handles. */
  CMPBool pressed_confirm;        /**< CMP_TRUE when confirm action is pressed. */
  CMPBool pressed_dismiss;        /**< CMP_TRUE when dismiss action is pressed. */
  CMPAlertDialogOnAction on_action; /**< Action callback (may be NULL). */
  void *on_action_ctx;             /**< Action callback context pointer. */
} M3AlertDialog;

/** @brief Default full screen dialog corner radius in pixels. */
#define M3_FULLSCREEN_DIALOG_DEFAULT_CORNER_RADIUS 0.0f
/** @brief Default full screen dialog horizontal padding in pixels. */
#define M3_FULLSCREEN_DIALOG_DEFAULT_PADDING_X 24.0f
/** @brief Default full screen dialog vertical padding in pixels. */
#define M3_FULLSCREEN_DIALOG_DEFAULT_PADDING_Y 24.0f
/** @brief Default spacing between title and body in pixels. */
#define M3_FULLSCREEN_DIALOG_DEFAULT_TITLE_BODY_SPACING 16.0f
/** @brief Default spacing between body and action in pixels. */
#define M3_FULLSCREEN_DIALOG_DEFAULT_BODY_ACTION_SPACING 24.0f
/** @brief Default action horizontal padding in pixels. */
#define M3_FULLSCREEN_DIALOG_DEFAULT_ACTION_PADDING_X 12.0f
/** @brief Default action vertical padding in pixels. */
#define M3_FULLSCREEN_DIALOG_DEFAULT_ACTION_PADDING_Y 8.0f

struct M3FullScreenDialog;

/**
 * @brief Full screen dialog action callback signature.
 * @param ctx User callback context pointer.
 * @param dialog Full screen dialog instance that triggered the action.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPFullScreenDialogOnAction)(
    void *ctx, struct M3FullScreenDialog *dialog);

/**
 * @brief Full screen dialog style descriptor.
 */
typedef struct M3FullScreenDialogStyle {
  CMPLayoutEdges padding;       /**< Padding around dialog contents. */
  CMPScalar corner_radius;      /**< Corner radius in pixels (>= 0). */
  CMPScalar title_body_spacing; /**< Spacing between title and body in pixels (>=
                                  0). */
  CMPScalar body_action_spacing; /**< Spacing between body and action in pixels
                                   (>= 0). */
  CMPScalar
      action_padding_x; /**< Horizontal padding for the action button (>= 0). */
  CMPScalar
      action_padding_y; /**< Vertical padding for the action button (>= 0). */
  CMPTextStyle title_style;  /**< Text style for the title. */
  CMPTextStyle body_style;   /**< Text style for the body. */
  CMPTextStyle action_style; /**< Text style for the action label. */
  CMPColor background_color; /**< Dialog background color. */
  CMPColor scrim_color;      /**< Scrim color used behind the dialog. */
  CMPShadow shadow;          /**< Shadow descriptor. */
  CMPBool shadow_enabled;    /**< CMP_TRUE when shadow rendering is enabled. */
  CMPBool scrim_enabled;     /**< CMP_TRUE when scrim rendering is enabled. */
} M3FullScreenDialogStyle;

/**
 * @brief Full screen dialog widget instance.
 */
typedef struct M3FullScreenDialog {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend;    /**< Text backend instance. */
  M3FullScreenDialogStyle style; /**< Current dialog style. */
  CMPHandle title_font;           /**< Font handle for the title text. */
  CMPHandle body_font;            /**< Font handle for the body text. */
  CMPHandle action_font;          /**< Font handle for action label. */
  CMPTextMetrics title_metrics;   /**< Cached title metrics. */
  CMPTextMetrics body_metrics;    /**< Cached body metrics. */
  CMPTextMetrics action_metrics;  /**< Cached action metrics. */
  const char *utf8_title;        /**< UTF-8 title text (may be NULL). */
  cmp_usize title_len;            /**< Title length in bytes. */
  const char *utf8_body;         /**< UTF-8 body text (may be NULL). */
  cmp_usize body_len;             /**< Body length in bytes. */
  const char *utf8_action;       /**< UTF-8 action label (may be NULL). */
  cmp_usize action_len;           /**< Action label length in bytes. */
  CMPRect bounds;                 /**< Layout bounds. */
  CMPRect action_bounds;          /**< Bounds for action hit testing. */
  CMPBool metrics_valid;          /**< CMP_TRUE when cached metrics are valid. */
  CMPBool owns_fonts;             /**< CMP_TRUE when widget owns font handles. */
  CMPBool pressed_action;         /**< CMP_TRUE when action is pressed. */
  CMPFullScreenDialogOnAction on_action; /**< Action callback (may be NULL). */
  void *on_action_ctx;                  /**< Action callback context pointer. */
} M3FullScreenDialog;

/** @brief Default snackbar corner radius in pixels. */
#define M3_SNACKBAR_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Default snackbar minimum width in pixels. */
#define M3_SNACKBAR_DEFAULT_MIN_WIDTH 160.0f
/** @brief Default snackbar maximum width in pixels. */
#define M3_SNACKBAR_DEFAULT_MAX_WIDTH 560.0f
/** @brief Default snackbar minimum height in pixels. */
#define M3_SNACKBAR_DEFAULT_MIN_HEIGHT 48.0f
/** @brief Default snackbar horizontal padding in pixels. */
#define M3_SNACKBAR_DEFAULT_PADDING_X 16.0f
/** @brief Default snackbar vertical padding in pixels. */
#define M3_SNACKBAR_DEFAULT_PADDING_Y 8.0f
/** @brief Default spacing between message and action in pixels. */
#define M3_SNACKBAR_DEFAULT_ACTION_SPACING 12.0f
/** @brief Default action horizontal padding in pixels. */
#define M3_SNACKBAR_DEFAULT_ACTION_PADDING_X 8.0f
/** @brief Default action vertical padding in pixels. */
#define M3_SNACKBAR_DEFAULT_ACTION_PADDING_Y 8.0f

struct M3Snackbar;

/**
 * @brief Snackbar action callback signature.
 * @param ctx User callback context pointer.
 * @param snackbar Snackbar instance that triggered the action.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPSnackbarOnAction)(void *ctx,
                                         struct M3Snackbar *snackbar);

/**
 * @brief Snackbar style descriptor.
 */
typedef struct M3SnackbarStyle {
  CMPLayoutEdges padding;   /**< Padding around snackbar contents. */
  CMPScalar corner_radius;  /**< Corner radius in pixels (>= 0). */
  CMPScalar min_width;      /**< Minimum snackbar width in pixels (>= 0). */
  CMPScalar max_width;      /**< Maximum snackbar width in pixels (>= 0). */
  CMPScalar min_height;     /**< Minimum snackbar height in pixels (>= 0). */
  CMPScalar action_spacing; /**< Spacing between message and action in pixels (>=
                              0). */
  CMPScalar
      action_padding_x; /**< Horizontal padding for the action button (>= 0). */
  CMPScalar
      action_padding_y; /**< Vertical padding for the action button (>= 0). */
  CMPTextStyle message_style; /**< Text style for the message. */
  CMPTextStyle action_style;  /**< Text style for the action label. */
  CMPColor background_color;  /**< Snackbar background color. */
} M3SnackbarStyle;

/**
 * @brief Snackbar widget instance.
 */
typedef struct M3Snackbar {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend;    /**< Text backend instance. */
  M3SnackbarStyle style;         /**< Current snackbar style. */
  CMPHandle message_font;         /**< Font handle for the message. */
  CMPHandle action_font;          /**< Font handle for the action label. */
  CMPTextMetrics message_metrics; /**< Cached message metrics. */
  CMPTextMetrics action_metrics;  /**< Cached action metrics. */
  const char *utf8_message;      /**< UTF-8 message text (may be NULL). */
  cmp_usize message_len;          /**< Message length in bytes. */
  const char *utf8_action;       /**< UTF-8 action label (may be NULL). */
  cmp_usize action_len;           /**< Action label length in bytes. */
  CMPRect bounds;                 /**< Layout bounds. */
  CMPRect action_bounds;          /**< Bounds for action hit testing. */
  CMPBool metrics_valid;          /**< CMP_TRUE when cached metrics are valid. */
  CMPBool owns_fonts;             /**< CMP_TRUE when widget owns font handles. */
  CMPBool pressed_action;         /**< CMP_TRUE when action is pressed. */
  CMPSnackbarOnAction on_action;  /**< Action callback (may be NULL). */
  void *on_action_ctx;           /**< Action callback context pointer. */
} M3Snackbar;

/**
 * @brief Initialize an alert dialog style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_alert_dialog_style_init(M3AlertDialogStyle *style);

/**
 * @brief Initialize an alert dialog widget.
 * @param dialog Alert dialog instance.
 * @param backend Text backend instance.
 * @param style Style descriptor.
 * @param utf8_title UTF-8 title text (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @param utf8_body UTF-8 body text (may be NULL when body_len is 0).
 * @param body_len Body length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_alert_dialog_init(
    M3AlertDialog *dialog, const CMPTextBackend *backend,
    const M3AlertDialogStyle *style, const char *utf8_title, cmp_usize title_len,
    const char *utf8_body, cmp_usize body_len);

/**
 * @brief Update the alert dialog title.
 * @param dialog Alert dialog instance.
 * @param utf8_title UTF-8 title text (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_alert_dialog_set_title(M3AlertDialog *dialog,
                                             const char *utf8_title,
                                             cmp_usize title_len);

/**
 * @brief Update the alert dialog body text.
 * @param dialog Alert dialog instance.
 * @param utf8_body UTF-8 body text (may be NULL when body_len is 0).
 * @param body_len Body length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_alert_dialog_set_body(M3AlertDialog *dialog,
                                            const char *utf8_body,
                                            cmp_usize body_len);

/**
 * @brief Update the alert dialog action labels.
 * @param dialog Alert dialog instance.
 * @param utf8_confirm UTF-8 confirm label (may be NULL when confirm_len is 0).
 * @param confirm_len Confirm label length in bytes.
 * @param utf8_dismiss UTF-8 dismiss label (may be NULL when dismiss_len is 0).
 * @param dismiss_len Dismiss label length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_alert_dialog_set_actions(M3AlertDialog *dialog,
                                               const char *utf8_confirm,
                                               cmp_usize confirm_len,
                                               const char *utf8_dismiss,
                                               cmp_usize dismiss_len);

/**
 * @brief Update the alert dialog style.
 * @param dialog Alert dialog instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_alert_dialog_set_style(M3AlertDialog *dialog,
                                             const M3AlertDialogStyle *style);

/**
 * @brief Assign the alert dialog action callback.
 * @param dialog Alert dialog instance.
 * @param on_action Action callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_alert_dialog_set_on_action(
    M3AlertDialog *dialog, CMPAlertDialogOnAction on_action, void *ctx);

/**
 * @brief Initialize a full screen dialog style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_fullscreen_dialog_style_init(M3FullScreenDialogStyle *style);

/**
 * @brief Initialize a full screen dialog widget.
 * @param dialog Full screen dialog instance.
 * @param backend Text backend instance.
 * @param style Style descriptor.
 * @param utf8_title UTF-8 title text (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @param utf8_body UTF-8 body text (may be NULL when body_len is 0).
 * @param body_len Body length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_fullscreen_dialog_init(
    M3FullScreenDialog *dialog, const CMPTextBackend *backend,
    const M3FullScreenDialogStyle *style, const char *utf8_title,
    cmp_usize title_len, const char *utf8_body, cmp_usize body_len);

/**
 * @brief Update the full screen dialog title.
 * @param dialog Full screen dialog instance.
 * @param utf8_title UTF-8 title text (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_fullscreen_dialog_set_title(M3FullScreenDialog *dialog,
                                                  const char *utf8_title,
                                                  cmp_usize title_len);

/**
 * @brief Update the full screen dialog body text.
 * @param dialog Full screen dialog instance.
 * @param utf8_body UTF-8 body text (may be NULL when body_len is 0).
 * @param body_len Body length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_fullscreen_dialog_set_body(M3FullScreenDialog *dialog,
                                                 const char *utf8_body,
                                                 cmp_usize body_len);

/**
 * @brief Update the full screen dialog action label.
 * @param dialog Full screen dialog instance.
 * @param utf8_action UTF-8 action label (may be NULL when action_len is 0).
 * @param action_len Action label length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_fullscreen_dialog_set_action(M3FullScreenDialog *dialog,
                                                   const char *utf8_action,
                                                   cmp_usize action_len);

/**
 * @brief Update the full screen dialog style.
 * @param dialog Full screen dialog instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_fullscreen_dialog_set_style(
    M3FullScreenDialog *dialog, const M3FullScreenDialogStyle *style);

/**
 * @brief Assign the full screen dialog action callback.
 * @param dialog Full screen dialog instance.
 * @param on_action Action callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_fullscreen_dialog_set_on_action(
    M3FullScreenDialog *dialog, CMPFullScreenDialogOnAction on_action,
    void *ctx);

/**
 * @brief Initialize a snackbar style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_snackbar_style_init(M3SnackbarStyle *style);

/**
 * @brief Initialize a snackbar widget.
 * @param snackbar Snackbar instance.
 * @param backend Text backend instance.
 * @param style Style descriptor.
 * @param utf8_message UTF-8 message text (may be NULL when message_len is 0).
 * @param message_len Message length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_snackbar_init(M3Snackbar *snackbar,
                                    const CMPTextBackend *backend,
                                    const M3SnackbarStyle *style,
                                    const char *utf8_message,
                                    cmp_usize message_len);

/**
 * @brief Update the snackbar message text.
 * @param snackbar Snackbar instance.
 * @param utf8_message UTF-8 message text (may be NULL when message_len is 0).
 * @param message_len Message length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_snackbar_set_message(M3Snackbar *snackbar,
                                           const char *utf8_message,
                                           cmp_usize message_len);

/**
 * @brief Update the snackbar action label.
 * @param snackbar Snackbar instance.
 * @param utf8_action UTF-8 action label (may be NULL when action_len is 0).
 * @param action_len Action label length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_snackbar_set_action(M3Snackbar *snackbar,
                                          const char *utf8_action,
                                          cmp_usize action_len);

/**
 * @brief Update the snackbar style.
 * @param snackbar Snackbar instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_snackbar_set_style(M3Snackbar *snackbar,
                                         const M3SnackbarStyle *style);

/**
 * @brief Assign the snackbar action callback.
 * @param snackbar Snackbar instance.
 * @param on_action Action callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_snackbar_set_on_action(M3Snackbar *snackbar,
                                             CMPSnackbarOnAction on_action,
                                             void *ctx);

#ifdef CMP_TESTING
/**
 * @brief Test helper to evaluate point-in-rect logic.
 * @param rect Rectangle to test (may be NULL).
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param out_inside Receives CMP_TRUE if the point is inside.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_dialog_test_point_in_rect(const CMPRect *rect, CMPScalar x,
                                                CMPScalar y, CMPBool *out_inside);
/**
 * @brief Test helper to validate a color struct.
 * @param color Color pointer (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_dialog_test_validate_color(const CMPColor *color);
/**
 * @brief Test helper to validate a text style.
 * @param style Text style pointer (may be NULL).
 * @param require_family CMP_TRUE to require a font family name.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_dialog_test_validate_text_style(const CMPTextStyle *style,
                                                      CMPBool require_family);
/**
 * @brief Test helper to validate layout edges.
 * @param edges Edge sizes pointer (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_dialog_test_validate_edges(const CMPLayoutEdges *edges);
/**
 * @brief Test helper to validate a measure spec.
 * @param spec Measurement spec.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_dialog_test_validate_measure_spec(CMPMeasureSpec spec);
/**
 * @brief Test helper to validate a rectangle.
 * @param rect Rectangle pointer (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_dialog_test_validate_rect(const CMPRect *rect);
/**
 * @brief Test helper to validate a text backend.
 * @param backend Backend pointer (may be NULL).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_dialog_test_validate_backend(const CMPTextBackend *backend);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_DIALOGS_H */
