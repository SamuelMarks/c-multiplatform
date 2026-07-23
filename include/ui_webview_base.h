#ifndef UI_WEBVIEW_BASE_H
#define UI_WEBVIEW_BASE_H

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_webview_base;

/**
 * @brief IPC message callback from the web context.
 */
typedef enum ui_error (*ui_webview_ipc_callback)(
    struct ui_webview_base *webview, const char *message, void *user_data);

/**
 * @brief Creates a new unstyled webview component.
 *
 * @param out_webview Pointer to output the initialized webview component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_webview_base_create(struct ui_webview_base **out_webview);

/**
 * @brief Destroys a webview component.
 *
 * @param webview The webview component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_webview_base_destroy(struct ui_webview_base *webview);

/**
 * @brief Retrieves the base component.
 *
 * @param webview The webview component.
 * @return The base component.
 */
enum ui_error
ui_webview_base_get_component(struct ui_webview_base *webview,
                              struct ui_component **out_component);

/**
 * @brief Sets the URL to load in the webview.
 *
 * @param webview The webview component.
 * @param url The URL to load.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_webview_base_set_url(struct ui_webview_base *webview,
                                      const char *url);

/**
 * @brief Sets raw HTML content for the webview.
 *
 * @param webview The webview component.
 * @param html The HTML content.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_webview_base_set_html(struct ui_webview_base *webview,
                                       const char *html);

/**
 * @brief Binds the URL property to a signal.
 *
 * @param webview The webview component.
 * @param signal The signal.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_webview_base_bind_url(struct ui_webview_base *webview,
                                       struct ui_signal *signal);

/**
 * @brief Evaluates JavaScript asynchronously in the webview context.
 *
 * @param webview The webview component.
 * @param script The JavaScript string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_webview_base_evaluate_js(struct ui_webview_base *webview,
                                          const char *script);

/**
 * @brief Registers a callback for receiving IPC messages from JavaScript.
 *
 * @param webview The webview component.
 * @param callback The callback function.
 * @param user_data User data to pass to the callback.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_webview_base_set_ipc_callback(struct ui_webview_base *webview,
                                               ui_webview_ipc_callback callback,
                                               void *user_data);

/**
 * @brief Simulates an incoming IPC message (for internal routing/testing).
 *
 * @param webview The webview component.
 * @param message The message content.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error
ui_webview_base_dispatch_ipc_message(struct ui_webview_base *webview,
                                     const char *message);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WEBVIEW_BASE_H */
