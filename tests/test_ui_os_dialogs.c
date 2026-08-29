/* clang-format off */
#include "../include/ui_os_dialogs.h"
#include "../src/ui_internal_mem.h"
#include "../include/ui_error.h"
#include "../include/ui_file_uploader_base.h"
#include "../include/ui_color_picker_base.h"
#include "../include/ui_reactor.h"
#include "../include/ui_thread_pool.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_os_dialogs(void) {
  struct ui_file_uploader_base uploader;
  struct ui_color_picker_base *color_picker = NULL;
  struct ui_reactor *reactor = NULL;
  struct ui_thread_pool *pool = NULL;
  struct ui_os_file_picker_config config = {0};

  struct ui_os_file_picker_config config_fail = {0};
  struct ui_os_file_picker_config config_ok = {0};

  ui_file_uploader_init(&uploader, 1, 0, 0, 100, 100, NULL);
  ui_color_picker_base_create(&color_picker, NULL);
  ui_reactor_create(&reactor);
  ui_thread_pool_create(1, &pool);

  config_fail.title = "FAIL";
  config_ok.title = "OK";

  ui_os_dialog_show_message_box("Title", "Message", UI_OS_MESSAGE_BOX_INFO);
  ui_os_dialog_show_message_box("Title", "Message", UI_OS_MESSAGE_BOX_WARNING);
  ui_os_dialog_show_message_box("Title", "Message", UI_OS_MESSAGE_BOX_ERROR);

  ui_os_dialog_open_file_picker_async(&uploader, &config, reactor, pool);
  ui_os_dialog_open_file_picker_async(&uploader, &config_ok, reactor, pool);
  ui_os_dialog_open_file_picker_async(&uploader, &config_fail, reactor, pool);
  ui_os_dialog_open_file_picker_async(&uploader, NULL, reactor, pool);
  ui_os_dialog_open_color_picker_async(color_picker, reactor, pool);

  ui_thread_pool_destroy(pool); /* this waits */

  /* Dispatch reactor */
  ui_reactor_poll(reactor, 0);

  /* NULLs */
  ui_os_dialog_show_message_box(NULL, NULL, UI_OS_MESSAGE_BOX_INFO);
  ui_os_dialog_show_message_box("T", NULL, UI_OS_MESSAGE_BOX_INFO);
  ui_os_dialog_show_message_box(NULL, "M", UI_OS_MESSAGE_BOX_INFO);

  ui_os_dialog_open_file_picker_async(NULL, NULL, NULL, NULL);
  ui_os_dialog_open_file_picker_async(&uploader, NULL, NULL, NULL);
  ui_os_dialog_open_file_picker_async(&uploader, NULL, reactor, NULL);

  ui_os_dialog_open_color_picker_async(NULL, NULL, NULL);
  ui_os_dialog_open_color_picker_async(color_picker, NULL, NULL);
  ui_os_dialog_open_color_picker_async(color_picker, reactor, NULL);

  ui_thread_pool_create(1, &pool);
  g_malloc_fail_countdown = 0;
  if (ui_os_dialog_open_file_picker_async(&uploader, &config, reactor, pool) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = 0;
  if (ui_os_dialog_open_color_picker_async(color_picker, reactor, pool) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_thread_pool_destroy(pool);
  ui_reactor_destroy(reactor);
  (void)ui_color_picker_base_destroy(color_picker);
  ui_file_uploader_destroy(&uploader);

  return 0;
}

#include <stdlib.h>
#include <string.h>

struct ui_os_file_task {
  struct ui_file_uploader_base *uploader;
  char result_path[256];
  struct ui_reactor *reactor;
  struct ui_os_file_picker_config config;
};

extern ui_error_t ui_os_file_completion(void *user_data);

static int test_os_file_completion(void) {
  struct ui_os_file_task *task;
  struct ui_file_uploader_base uploader;

  ui_file_uploader_init(&uploader, 1, 0, 0, 100, 100, NULL);

  task = malloc(sizeof(struct ui_os_file_task));
  task->uploader = &uploader;
  task->result_path[0] = '\0';
  ui_os_file_completion(task);

  task = malloc(sizeof(struct ui_os_file_task));
  task->uploader = &uploader;
  UI_STRCPY(task->result_path, sizeof(task->result_path), "mock_path.txt");
  ui_os_file_completion(task);

  ui_file_uploader_destroy(&uploader);
  return 0;
}

int main(void) {
  if (test_os_file_completion() != 0)
    return 1;

  if (test_os_dialogs())
    return 1;
  return 0;
}
