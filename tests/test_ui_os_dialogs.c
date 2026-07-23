/* clang-format off */
#include "../include/ui_os_dialogs.h"
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
  ui_color_picker_base_destroy(color_picker);
  ui_file_uploader_destroy(&uploader);

  return 0;
}

int main(void) {
  if (test_os_dialogs())
    return 1;
  return 0;
}
