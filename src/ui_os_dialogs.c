/* clang-format off */
#include "../include/ui_os_dialogs.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
#ifndef UI_WINAPI
#if defined(_MSC_VER)
#define UI_WINAPI __stdcall
#elif defined(__GNUC__)
#define UI_WINAPI __attribute__((stdcall))
#else
#define UI_WINAPI
#endif
#endif
extern int UI_WINAPI MessageBoxA(void *hWnd, const char *lpText, const char *lpCaption, unsigned int uType);
#endif
/* clang-format on */

struct ui_os_file_task {
  struct ui_file_uploader_base *uploader;
  struct ui_os_file_picker_config config;
  struct ui_reactor *reactor;
  char result_path[1024];
};

/** \brief ui_os_color_task */
struct ui_os_color_task {
  struct ui_color_picker_base *picker;
  struct ui_reactor *reactor;
  struct ui_color_rgb result_color;
};

static enum ui_error ui_os_file_completion(void *user_data) {
  struct ui_os_file_task *task = (struct ui_os_file_task *)user_data;
  enum ui_error rc = UI_ERROR_NONE;
  if (task->result_path[0] != '\0') {
    rc = ui_file_uploader_drop_file(task->uploader, task->result_path);
    if (rc == UI_ERROR_NONE) {
      rc = ui_file_uploader_read_files(task->uploader);
    }
  }
  UI_FREE(task);
  return rc;
}

static enum ui_error ui_os_file_worker(void *user_data) {
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

  ui_reactor_schedule(task->reactor, ui_os_file_completion, task);
  return ui_reactor_wake(task->reactor);
}

static enum ui_error ui_os_color_completion(void *user_data) {
  struct ui_os_color_task *task = (struct ui_os_color_task *)user_data;
  enum ui_error rc = UI_ERROR_NONE;
  rc = ui_color_picker_base_set_rgb(task->picker, &task->result_color);
  UI_FREE(task);
  return rc;
}

static enum ui_error ui_os_color_worker(void *user_data) {
  struct ui_os_color_task *task = (struct ui_os_color_task *)user_data;

  /* Mock: Just return a dummy color for now */
  /* In a real implementation, call ChooseColor or NSColorPanel */
  task->result_color.r = 255;
  task->result_color.g = 0;
  task->result_color.b = 0;

  ui_reactor_schedule(task->reactor, ui_os_color_completion, task);
  return ui_reactor_wake(task->reactor);
}

enum ui_error ui_os_dialog_show_message_box(const char *title,
                                            const char *message,
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

/** \brief ui_os_dialog_open_file_picker_async */
enum ui_error ui_os_dialog_open_file_picker_async(
    struct ui_file_uploader_base *uploader,
    const struct ui_os_file_picker_config *config, struct ui_reactor *reactor,
    struct ui_thread_pool *pool) {

  struct ui_os_file_task *task = NULL;

  if (!uploader || !reactor || !pool) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  task = (struct ui_os_file_task *)UI_MALLOC(sizeof(struct ui_os_file_task));
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

/** \brief ui_error */
enum ui_error
ui_os_dialog_open_color_picker_async(struct ui_color_picker_base *picker,
                                     struct ui_reactor *reactor,
                                     struct ui_thread_pool *pool) {

  struct ui_os_color_task *task = NULL;

  if (!picker || !reactor || !pool) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  task = (struct ui_os_color_task *)UI_MALLOC(sizeof(struct ui_os_color_task));
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
