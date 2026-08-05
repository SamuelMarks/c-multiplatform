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
 * @brief Type of message box to show.
 */
enum ui_os_message_box_type {
  UI_OS_MESSAGE_BOX_INFO = 0,
  UI_OS_MESSAGE_BOX_WARNING = 1,
  UI_OS_MESSAGE_BOX_ERROR = 2
};

/**
 * @brief Configuration for native file pickers.
 */
struct ui_os_file_picker_config {
  ui_bool_t allow_multiple;
  const char *title;
  const char *filter; /* OS-specific filter string, or NULL */
};

/**
 * @brief Shows a native OS message box (blocking).
 *
 * @param title The dialog title.
 * @param message The dialog message body.
 * @param type The type/icon of the message box.
 * @return UI_ERROR_NONE on success.
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
 * @return UI_ERROR_NONE on success.
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
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_os_dialog_open_color_picker_async(struct ui_color_picker_base *picker,
                                     struct ui_reactor *reactor,
                                     struct ui_thread_pool *pool);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_OS_DIALOGS_H */
