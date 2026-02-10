#ifndef M3_DIALOGS_H
#define M3_DIALOGS_H

/**
 * @file m3_dialogs.h
 * @brief Dialog and snackbar widgets for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_layout.h"
#include "m3_text.h"
#include "m3_visuals.h"

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
 * @param action_id Action identifier (M3_ALERT_DIALOG_ACTION_*).
 * @return M3_OK on success or a failure code.
 */
typedef int (M3_CALL *M3AlertDialogOnAction)(void *ctx, struct M3AlertDialog *dialog, m3_u32 action_id);

/**
 * @brief Alert dialog style descriptor.
 */
typedef struct M3AlertDialogStyle {
    M3LayoutEdges padding; /**< Padding around dialog contents. */
    M3Scalar corner_radius; /**< Corner radius in pixels (>= 0). */
    M3Scalar min_width; /**< Minimum dialog width in pixels (>= 0). */
    M3Scalar max_width; /**< Maximum dialog width in pixels (>= 0). */
    M3Scalar title_body_spacing; /**< Spacing between title and body in pixels (>= 0). */
    M3Scalar body_action_spacing; /**< Spacing between body and actions in pixels (>= 0). */
    M3Scalar action_spacing; /**< Spacing between action buttons in pixels (>= 0). */
    M3Scalar action_padding_x; /**< Horizontal padding for action buttons (>= 0). */
    M3Scalar action_padding_y; /**< Vertical padding for action buttons (>= 0). */
    M3TextStyle title_style; /**< Text style for the title. */
    M3TextStyle body_style; /**< Text style for the body. */
    M3TextStyle action_style; /**< Text style for action labels. */
    M3Color background_color; /**< Dialog background color. */
    M3Color scrim_color; /**< Scrim color used behind the dialog. */
    M3Shadow shadow; /**< Shadow descriptor. */
    M3Bool shadow_enabled; /**< M3_TRUE when shadow rendering is enabled. */
    M3Bool scrim_enabled; /**< M3_TRUE when scrim rendering is enabled. */
} M3AlertDialogStyle;

/**
 * @brief Alert dialog widget instance.
 */
typedef struct M3AlertDialog {
    M3Widget widget; /**< Widget interface (points to this instance). */
    M3TextBackend text_backend; /**< Text backend instance. */
    M3AlertDialogStyle style; /**< Current dialog style. */
    M3Handle title_font; /**< Font handle for the title text. */
    M3Handle body_font; /**< Font handle for the body text. */
    M3Handle action_font; /**< Font handle for action labels. */
    M3TextMetrics title_metrics; /**< Cached title metrics. */
    M3TextMetrics body_metrics; /**< Cached body metrics. */
    M3TextMetrics confirm_metrics; /**< Cached confirm action metrics. */
    M3TextMetrics dismiss_metrics; /**< Cached dismiss action metrics. */
    const char *utf8_title; /**< UTF-8 title text (may be NULL). */
    m3_usize title_len; /**< Title length in bytes. */
    const char *utf8_body; /**< UTF-8 body text (may be NULL). */
    m3_usize body_len; /**< Body length in bytes. */
    const char *utf8_confirm; /**< UTF-8 confirm label (may be NULL). */
    m3_usize confirm_len; /**< Confirm label length in bytes. */
    const char *utf8_dismiss; /**< UTF-8 dismiss label (may be NULL). */
    m3_usize dismiss_len; /**< Dismiss label length in bytes. */
    M3Rect bounds; /**< Layout bounds. */
    M3Rect confirm_bounds; /**< Bounds for confirm action hit testing. */
    M3Rect dismiss_bounds; /**< Bounds for dismiss action hit testing. */
    M3Bool metrics_valid; /**< M3_TRUE when cached metrics are valid. */
    M3Bool owns_fonts; /**< M3_TRUE when widget owns font handles. */
    M3Bool pressed_confirm; /**< M3_TRUE when confirm action is pressed. */
    M3Bool pressed_dismiss; /**< M3_TRUE when dismiss action is pressed. */
    M3AlertDialogOnAction on_action; /**< Action callback (may be NULL). */
    void *on_action_ctx; /**< Action callback context pointer. */
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
 * @return M3_OK on success or a failure code.
 */
typedef int (M3_CALL *M3FullScreenDialogOnAction)(void *ctx, struct M3FullScreenDialog *dialog);

/**
 * @brief Full screen dialog style descriptor.
 */
typedef struct M3FullScreenDialogStyle {
    M3LayoutEdges padding; /**< Padding around dialog contents. */
    M3Scalar corner_radius; /**< Corner radius in pixels (>= 0). */
    M3Scalar title_body_spacing; /**< Spacing between title and body in pixels (>= 0). */
    M3Scalar body_action_spacing; /**< Spacing between body and action in pixels (>= 0). */
    M3Scalar action_padding_x; /**< Horizontal padding for the action button (>= 0). */
    M3Scalar action_padding_y; /**< Vertical padding for the action button (>= 0). */
    M3TextStyle title_style; /**< Text style for the title. */
    M3TextStyle body_style; /**< Text style for the body. */
    M3TextStyle action_style; /**< Text style for the action label. */
    M3Color background_color; /**< Dialog background color. */
    M3Color scrim_color; /**< Scrim color used behind the dialog. */
    M3Shadow shadow; /**< Shadow descriptor. */
    M3Bool shadow_enabled; /**< M3_TRUE when shadow rendering is enabled. */
    M3Bool scrim_enabled; /**< M3_TRUE when scrim rendering is enabled. */
} M3FullScreenDialogStyle;

/**
 * @brief Full screen dialog widget instance.
 */
typedef struct M3FullScreenDialog {
    M3Widget widget; /**< Widget interface (points to this instance). */
    M3TextBackend text_backend; /**< Text backend instance. */
    M3FullScreenDialogStyle style; /**< Current dialog style. */
    M3Handle title_font; /**< Font handle for the title text. */
    M3Handle body_font; /**< Font handle for the body text. */
    M3Handle action_font; /**< Font handle for action label. */
    M3TextMetrics title_metrics; /**< Cached title metrics. */
    M3TextMetrics body_metrics; /**< Cached body metrics. */
    M3TextMetrics action_metrics; /**< Cached action metrics. */
    const char *utf8_title; /**< UTF-8 title text (may be NULL). */
    m3_usize title_len; /**< Title length in bytes. */
    const char *utf8_body; /**< UTF-8 body text (may be NULL). */
    m3_usize body_len; /**< Body length in bytes. */
    const char *utf8_action; /**< UTF-8 action label (may be NULL). */
    m3_usize action_len; /**< Action label length in bytes. */
    M3Rect bounds; /**< Layout bounds. */
    M3Rect action_bounds; /**< Bounds for action hit testing. */
    M3Bool metrics_valid; /**< M3_TRUE when cached metrics are valid. */
    M3Bool owns_fonts; /**< M3_TRUE when widget owns font handles. */
    M3Bool pressed_action; /**< M3_TRUE when action is pressed. */
    M3FullScreenDialogOnAction on_action; /**< Action callback (may be NULL). */
    void *on_action_ctx; /**< Action callback context pointer. */
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
 * @return M3_OK on success or a failure code.
 */
typedef int (M3_CALL *M3SnackbarOnAction)(void *ctx, struct M3Snackbar *snackbar);

/**
 * @brief Snackbar style descriptor.
 */
typedef struct M3SnackbarStyle {
    M3LayoutEdges padding; /**< Padding around snackbar contents. */
    M3Scalar corner_radius; /**< Corner radius in pixels (>= 0). */
    M3Scalar min_width; /**< Minimum snackbar width in pixels (>= 0). */
    M3Scalar max_width; /**< Maximum snackbar width in pixels (>= 0). */
    M3Scalar min_height; /**< Minimum snackbar height in pixels (>= 0). */
    M3Scalar action_spacing; /**< Spacing between message and action in pixels (>= 0). */
    M3Scalar action_padding_x; /**< Horizontal padding for the action button (>= 0). */
    M3Scalar action_padding_y; /**< Vertical padding for the action button (>= 0). */
    M3TextStyle message_style; /**< Text style for the message. */
    M3TextStyle action_style; /**< Text style for the action label. */
    M3Color background_color; /**< Snackbar background color. */
} M3SnackbarStyle;

/**
 * @brief Snackbar widget instance.
 */
typedef struct M3Snackbar {
    M3Widget widget; /**< Widget interface (points to this instance). */
    M3TextBackend text_backend; /**< Text backend instance. */
    M3SnackbarStyle style; /**< Current snackbar style. */
    M3Handle message_font; /**< Font handle for the message. */
    M3Handle action_font; /**< Font handle for the action label. */
    M3TextMetrics message_metrics; /**< Cached message metrics. */
    M3TextMetrics action_metrics; /**< Cached action metrics. */
    const char *utf8_message; /**< UTF-8 message text (may be NULL). */
    m3_usize message_len; /**< Message length in bytes. */
    const char *utf8_action; /**< UTF-8 action label (may be NULL). */
    m3_usize action_len; /**< Action label length in bytes. */
    M3Rect bounds; /**< Layout bounds. */
    M3Rect action_bounds; /**< Bounds for action hit testing. */
    M3Bool metrics_valid; /**< M3_TRUE when cached metrics are valid. */
    M3Bool owns_fonts; /**< M3_TRUE when widget owns font handles. */
    M3Bool pressed_action; /**< M3_TRUE when action is pressed. */
    M3SnackbarOnAction on_action; /**< Action callback (may be NULL). */
    void *on_action_ctx; /**< Action callback context pointer. */
} M3Snackbar;

/**
 * @brief Initialize an alert dialog style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_alert_dialog_style_init(M3AlertDialogStyle *style);

/**
 * @brief Initialize an alert dialog widget.
 * @param dialog Alert dialog instance.
 * @param backend Text backend instance.
 * @param style Style descriptor.
 * @param utf8_title UTF-8 title text (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @param utf8_body UTF-8 body text (may be NULL when body_len is 0).
 * @param body_len Body length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_alert_dialog_init(M3AlertDialog *dialog, const M3TextBackend *backend, const M3AlertDialogStyle *style,
    const char *utf8_title, m3_usize title_len, const char *utf8_body, m3_usize body_len);

/**
 * @brief Update the alert dialog title.
 * @param dialog Alert dialog instance.
 * @param utf8_title UTF-8 title text (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_alert_dialog_set_title(M3AlertDialog *dialog, const char *utf8_title, m3_usize title_len);

/**
 * @brief Update the alert dialog body text.
 * @param dialog Alert dialog instance.
 * @param utf8_body UTF-8 body text (may be NULL when body_len is 0).
 * @param body_len Body length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_alert_dialog_set_body(M3AlertDialog *dialog, const char *utf8_body, m3_usize body_len);

/**
 * @brief Update the alert dialog action labels.
 * @param dialog Alert dialog instance.
 * @param utf8_confirm UTF-8 confirm label (may be NULL when confirm_len is 0).
 * @param confirm_len Confirm label length in bytes.
 * @param utf8_dismiss UTF-8 dismiss label (may be NULL when dismiss_len is 0).
 * @param dismiss_len Dismiss label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_alert_dialog_set_actions(M3AlertDialog *dialog, const char *utf8_confirm, m3_usize confirm_len,
    const char *utf8_dismiss, m3_usize dismiss_len);

/**
 * @brief Update the alert dialog style.
 * @param dialog Alert dialog instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_alert_dialog_set_style(M3AlertDialog *dialog, const M3AlertDialogStyle *style);

/**
 * @brief Assign the alert dialog action callback.
 * @param dialog Alert dialog instance.
 * @param on_action Action callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_alert_dialog_set_on_action(M3AlertDialog *dialog, M3AlertDialogOnAction on_action, void *ctx);

/**
 * @brief Initialize a full screen dialog style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_fullscreen_dialog_style_init(M3FullScreenDialogStyle *style);

/**
 * @brief Initialize a full screen dialog widget.
 * @param dialog Full screen dialog instance.
 * @param backend Text backend instance.
 * @param style Style descriptor.
 * @param utf8_title UTF-8 title text (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @param utf8_body UTF-8 body text (may be NULL when body_len is 0).
 * @param body_len Body length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_fullscreen_dialog_init(M3FullScreenDialog *dialog, const M3TextBackend *backend,
    const M3FullScreenDialogStyle *style, const char *utf8_title, m3_usize title_len, const char *utf8_body, m3_usize body_len);

/**
 * @brief Update the full screen dialog title.
 * @param dialog Full screen dialog instance.
 * @param utf8_title UTF-8 title text (may be NULL when title_len is 0).
 * @param title_len Title length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_fullscreen_dialog_set_title(M3FullScreenDialog *dialog, const char *utf8_title, m3_usize title_len);

/**
 * @brief Update the full screen dialog body text.
 * @param dialog Full screen dialog instance.
 * @param utf8_body UTF-8 body text (may be NULL when body_len is 0).
 * @param body_len Body length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_fullscreen_dialog_set_body(M3FullScreenDialog *dialog, const char *utf8_body, m3_usize body_len);

/**
 * @brief Update the full screen dialog action label.
 * @param dialog Full screen dialog instance.
 * @param utf8_action UTF-8 action label (may be NULL when action_len is 0).
 * @param action_len Action label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_fullscreen_dialog_set_action(M3FullScreenDialog *dialog, const char *utf8_action, m3_usize action_len);

/**
 * @brief Update the full screen dialog style.
 * @param dialog Full screen dialog instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_fullscreen_dialog_set_style(M3FullScreenDialog *dialog, const M3FullScreenDialogStyle *style);

/**
 * @brief Assign the full screen dialog action callback.
 * @param dialog Full screen dialog instance.
 * @param on_action Action callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_fullscreen_dialog_set_on_action(M3FullScreenDialog *dialog, M3FullScreenDialogOnAction on_action, void *ctx);

/**
 * @brief Initialize a snackbar style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_snackbar_style_init(M3SnackbarStyle *style);

/**
 * @brief Initialize a snackbar widget.
 * @param snackbar Snackbar instance.
 * @param backend Text backend instance.
 * @param style Style descriptor.
 * @param utf8_message UTF-8 message text (may be NULL when message_len is 0).
 * @param message_len Message length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_snackbar_init(M3Snackbar *snackbar, const M3TextBackend *backend, const M3SnackbarStyle *style,
    const char *utf8_message, m3_usize message_len);

/**
 * @brief Update the snackbar message text.
 * @param snackbar Snackbar instance.
 * @param utf8_message UTF-8 message text (may be NULL when message_len is 0).
 * @param message_len Message length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_snackbar_set_message(M3Snackbar *snackbar, const char *utf8_message, m3_usize message_len);

/**
 * @brief Update the snackbar action label.
 * @param snackbar Snackbar instance.
 * @param utf8_action UTF-8 action label (may be NULL when action_len is 0).
 * @param action_len Action label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_snackbar_set_action(M3Snackbar *snackbar, const char *utf8_action, m3_usize action_len);

/**
 * @brief Update the snackbar style.
 * @param snackbar Snackbar instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_snackbar_set_style(M3Snackbar *snackbar, const M3SnackbarStyle *style);

/**
 * @brief Assign the snackbar action callback.
 * @param snackbar Snackbar instance.
 * @param on_action Action callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_snackbar_set_on_action(M3Snackbar *snackbar, M3SnackbarOnAction on_action, void *ctx);

#ifdef M3_TESTING
/**
 * @brief Test helper to evaluate point-in-rect logic.
 * @param rect Rectangle to test (may be NULL).
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param out_inside Receives M3_TRUE if the point is inside.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_dialog_test_point_in_rect(const M3Rect *rect, M3Scalar x, M3Scalar y, M3Bool *out_inside);
/**
 * @brief Test helper to validate a color struct.
 * @param color Color pointer (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_dialog_test_validate_color(const M3Color *color);
/**
 * @brief Test helper to validate a text style.
 * @param style Text style pointer (may be NULL).
 * @param require_family M3_TRUE to require a font family name.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_dialog_test_validate_text_style(const M3TextStyle *style, M3Bool require_family);
/**
 * @brief Test helper to validate layout edges.
 * @param edges Edge sizes pointer (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_dialog_test_validate_edges(const M3LayoutEdges *edges);
/**
 * @brief Test helper to validate a measure spec.
 * @param spec Measurement spec.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_dialog_test_validate_measure_spec(M3MeasureSpec spec);
/**
 * @brief Test helper to validate a rectangle.
 * @param rect Rectangle pointer (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_dialog_test_validate_rect(const M3Rect *rect);
/**
 * @brief Test helper to validate a text backend.
 * @param backend Backend pointer (may be NULL).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_dialog_test_validate_backend(const M3TextBackend *backend);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_DIALOGS_H */
