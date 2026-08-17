#ifndef UI_OS_DIALOGS_H
#define UI_OS_DIALOGS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_file_uploader_base.h"
#include "ui_color_picker_base.h"
#include "ui_reactor.h"
#include "ui_thread_pool.h"
/* clang-format on */

/**
 * @enum ui_os_message_box_type
 * @brief Type of message box to show.
 */
enum ui_os_message_box_type {
  /** @brief Informational message box. */
  UI_OS_MESSAGE_BOX_INFO = 0,
  /** @brief Warning message box. */
  UI_OS_MESSAGE_BOX_WARNING = 1,
  /** @brief Error message box. */
  UI_OS_MESSAGE_BOX_ERROR = 2
};

/**
 * @struct ui_os_file_picker_config
 * @brief Configuration for native file pickers.
 */
struct ui_os_file_picker_config {
  /** @brief Whether to allow multiple file selections. */
  ui_bool_t allow_multiple;
  /** @brief The title of the file picker dialog. */
  const char *title;
  /** @brief OS-specific filter string, or NULL if none. */
  const char *filter;
};

/**
 * @brief Shows a native OS message box (blocking).
 *
 * @param title The dialog title.
 * @param message The dialog message body.
 * @param type The type/icon of the message box.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_os_dialog_show_message_box(const char *title, const char *message,
                                         enum ui_os_message_box_type type);

/**
 * @brief Opens a native file picker asynchronously, bridging results directly
 * into the uploader.
 *
 * @param uploader The uploader base to feed paths into upon selection.
 * @param config The picker configuration.
 * @param reactor The reactor to schedule the completion callback on.
 * @param pool The thread pool to run the blocking dialog on.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_os_dialog_open_file_picker_async(
    struct ui_file_uploader_base *uploader,
    const struct ui_os_file_picker_config *config, struct ui_reactor *reactor,
    struct ui_thread_pool *pool);

/**
 * @brief Opens a native color picker asynchronously, bridging results directly
 * into the picker base.
 *
 * @param picker The color picker base to feed the selected color into.
 * @param reactor The reactor to schedule the completion callback on.
 * @param pool The thread pool to run the blocking dialog on.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_os_dialog_open_color_picker_async(struct ui_color_picker_base *picker,
                                     struct ui_reactor *reactor,
                                     struct ui_thread_pool *pool);

/**
 * @brief Callback function used when a file operation completes.
 *
 * @param user_data Opaque pointer to user data passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_os_file_completion(void *user_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_OS_DIALOGS_H */
