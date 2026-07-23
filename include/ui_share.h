#ifndef UI_SHARE_H
#define UI_SHARE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_promise.h"
/* clang-format on */

/**
 * @brief Represents the payload to be shared via the OS native share sheet.
 */
struct ui_share_payload {
  const char *title; /**< The title of the document or payload being shared */
  const char *text;  /**< The body text or description being shared */
  const char *url;   /**< A URL to share */
  /* Note: file attachments can be added here in the future as an array of paths
   */
};

/**
 * @brief Checks if the native share sheet is available on the current
 * device/OS.
 *
 * @param out_is_available Pointer to receive 1 if available, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_share_is_supported(int *out_is_available);

/**
 * @brief Triggers the native OS share sheet asynchronously.
 *        The provided promise will be resolved when the user completes or
 * dismisses the share action. Some OSes may not report success vs dismissal
 * accurately, so resolution implies the flow finished.
 *
 * @param payload The data to be shared.
 * @param promise A pending promise that will be resolved on completion, or
 * rejected on failure. The resolved value will be NULL (void*).
 * @return UI_ERROR_NONE if the share request was successfully dispatched.
 */
enum ui_error ui_share_request_async(const struct ui_share_payload *payload,
                                     struct ui_promise *promise);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SHARE_H */
