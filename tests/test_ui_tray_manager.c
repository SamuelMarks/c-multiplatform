/* clang-format off */
#include "../include/ui_image_base.h"
#include "../include/ui_menu_base.h"
#include "../include/ui_tray_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int main(void) {
  struct ui_tray_manager *tray = NULL;
  struct ui_menu_base *menu = NULL;
  struct ui_image_base image;
  enum ui_error rc;
  int failed = 0;

  rc = ui_tray_manager_create(&tray);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_menu_base_create(&menu);
  failed |= (rc != UI_ERROR_NONE);

  memset(&image, 0, sizeof(image));

  rc = ui_tray_manager_set_tooltip(tray, "Test Tooltip");
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_set_icon(tray, &image);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_set_context_menu(tray, menu);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_show(tray);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_hide(tray);
  failed |= (rc != UI_ERROR_NONE);

  ui_tray_manager_destroy(tray);
  ui_menu_base_destroy(menu);

  if (!failed) {
    printf("test_ui_tray_manager passed\n");
  }
  return failed;
}
