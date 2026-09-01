/**
 * @file ui_os_dialogs.c
 * @brief Implementation of OS-native dialogs.
 * @details Wrappers for file pickers, color pickers, and message boxes.
 */
/* clang-format off */
#include "../include/ui_os_dialogs.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
#ifndef UI_WINAPI
#if defined(_MSC_VER)
/** @cond */
#define UI_WINAPI __stdcall
/** @endcond */
#elif defined(__GNUC__)
/** @cond */
#define UI_WINAPI __attribute__((stdcall))
/** @endcond */
#else
/** @cond */
#define UI_WINAPI
/** @endcond */
#endif
#endif
extern int UI_WINAPI MessageBoxA(void *hWnd, const char *lpText, const char *lpCaption, unsigned int uType);
#endif
/* clang-format on */

/**
 * @struct ui_os_file_task
 * @brief Asynchronous task context for an OS file picker dialog.
 */
struct ui_os_file_task {
  struct ui_file_uploader_base *uploader; /**< Receiving uploader widget */
  struct ui_os_file_picker_config config; /**< Picker configuration */
  struct ui_reactor *reactor;             /**< Execution reactor */
  char result_path[1024];                 /**< Buffer for resulting file path */
};

/**
 * @struct ui_os_color_task
 * @brief Asynchronous task context for an OS color picker dialog.
 */
struct ui_os_color_task {
  struct ui_color_picker_base *picker; /**< Receiving color picker widget */
  struct ui_reactor *reactor;          /**< Execution reactor */
  struct ui_color_rgb result_color;    /**< Resulting RGB color */
};

/**
 * @brief Reactor callback triggered when the file picker completes.
 * @param[in,out] user_data Pointer to the file task context.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_os_file_completion(void *user_data) {
  struct ui_os_file_task *task = (struct ui_os_file_task *)user_data;
  ui_error_t rc = UI_ERROR_NONE;
  {
    ui_error_t rc_cleanup =
        ui_file_uploader_drop_file(task->uploader, task->result_path);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_file_uploader_read_files(task->uploader);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  C_MULTIPLATFORM_FREE(task);
  return rc;
}

/**
 * @brief Background thread worker that actually invokes the OS file picker.
 * @param[in,out] user_data Pointer to the file task context.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t ui_os_file_worker(void *user_data) {
  struct ui_os_file_task *task = (struct ui_os_file_task *)user_data;

  /* Mock: Just return a dummy file path for now */
  /* In a real implementation, call IFileOpenDialog or popen("zenity
   * --file-selection") */
  if (task->config.title && strcmp(task->config.title, "FAIL") == 0) {
    task->result_path[0] = '\0';
  } else {
#if defined(_MSC_VER)
    strcpy_s(task->result_path, sizeof(task->result_path), "mock_file.txt");
#else
    strncpy(task->result_path, "mock_file.txt", sizeof(task->result_path) - 1);
    task->result_path[sizeof(task->result_path) - 1] = '\0';
#endif
  }

  {
    ui_error_t sched_rc =
        ui_reactor_schedule(task->reactor, ui_os_file_completion, task);
    if (sched_rc != UI_ERROR_NONE)
      return sched_rc;
  }
  return ui_reactor_wake(task->reactor);
}

/**
 * @brief Reactor callback triggered when the color picker completes.
 * @param[in,out] user_data Pointer to the color task context.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t ui_os_color_completion(void *user_data) {
  struct ui_os_color_task *task = (struct ui_os_color_task *)user_data;
  ui_error_t rc =
      ui_color_picker_base_set_rgb(task->picker, &task->result_color);
  C_MULTIPLATFORM_FREE(task);
  return rc;
}

/**
 * @brief Background thread worker that actually invokes the OS color picker.
 * @param[in,out] user_data Pointer to the color task context.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t ui_os_color_worker(void *user_data) {
  struct ui_os_color_task *task = (struct ui_os_color_task *)user_data;

  /* Mock: Just return a dummy color for now */
  /* In a real implementation, call ChooseColor or NSColorPanel */
  task->result_color.r = 255;
  task->result_color.g = 0;
  task->result_color.b = 0;

  {
    ui_error_t sched_rc =
        ui_reactor_schedule(task->reactor, ui_os_color_completion, task);
    if (sched_rc != UI_ERROR_NONE)
      return sched_rc;
  }
  return ui_reactor_wake(task->reactor);
}

/**
 * @brief Shows a native OS message box (blocking).
 * @param[in] title The dialog title.
 * @param[in] message The dialog message.
 * @param[in] type The style/icon of the message box.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_os_dialog_show_message_box(const char *title, const char *message,
                                         enum ui_os_message_box_type type) {
  if (!title || !message) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  {
    unsigned int uType = 0; /* MB_OK */
    if (type == UI_OS_MESSAGE_BOX_ERROR)
      uType |= 0x00000010L; /* MB_ICONERROR */
    else if (type == UI_OS_MESSAGE_BOX_WARNING)
      uType |= 0x00000030L; /* MB_ICONWARNING */
    else
      uType |= 0x00000040L; /* MB_ICONINFORMATION */

#ifndef UI_TEST_MOCK_ALLOC
    MessageBoxA(NULL, message, title, uType);
#else
    (void)uType;
    printf("[%s]: %s\n", title, message);
#endif
  }
#else
  /* Basic fallback for POSIX / Linux to console if no GTK/Zenity is used inline
   */
  (void)type;
  printf("[%s]: %s\n", title, message);
#endif

  return UI_ERROR_NONE;
}

/**
 * @brief Asynchronously opens a native OS file picker.
 * @param[in,out] uploader The file uploader base widget to receive the result.
 * @param[in] config Optional configuration (filters, multi-select).
 * @param[in,out] reactor The reactor to handle the result callback on the main
 * thread.
 * @param[in,out] pool The thread pool to execute the blocking OS call.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_os_dialog_open_file_picker_async(
    struct ui_file_uploader_base *uploader,
    const struct ui_os_file_picker_config *config, struct ui_reactor *reactor,
    struct ui_thread_pool *pool) {

  struct ui_os_file_task *task = NULL;

  if (!uploader || !reactor || !pool) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  task = (struct ui_os_file_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_os_file_task));
  if (!task) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  task->uploader = uploader;
  if (config) {
    task->config = *config;
  } else {
    memset(&task->config, 0, sizeof(struct ui_os_file_picker_config));
  }
  task->reactor = reactor;
  task->result_path[0] = '\0';

  return ui_thread_pool_schedule(pool, ui_os_file_worker, task);
}

/**
 * @brief Asynchronously opens a native OS color picker.
 * @param[in,out] picker The color picker base widget to receive the result.
 * @param[in,out] reactor The reactor to handle the result callback.
 * @param[in,out] pool The thread pool to execute the blocking OS call.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_os_dialog_open_color_picker_async(struct ui_color_picker_base *picker,
                                     struct ui_reactor *reactor,
                                     struct ui_thread_pool *pool) {

  struct ui_os_color_task *task = NULL;

  if (!picker || !reactor || !pool) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  task = (struct ui_os_color_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_os_color_task));
  if (!task) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  task->picker = picker;
  task->reactor = reactor;
  task->result_color.r = 0;
  task->result_color.g = 0;
  task->result_color.b = 0;

  return ui_thread_pool_schedule(pool, ui_os_color_worker, task);
}
