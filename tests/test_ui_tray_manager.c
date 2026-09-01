/**
 * @file test_ui_tray_manager.c
 * @brief Tests for the tray manager component.
 */
/* clang-format off */
#include "../include/ui_image_base.h"
#include "../include/ui_menu_base.h"
#include "../include/ui_tray_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

int main(void) {
  struct ui_tray_manager *tray = NULL;
  struct ui_menu_base *menu = NULL;
  struct ui_image_base image;
  ui_error_t rc;
  int failed = 0;

  rc = ui_tray_manager_create(NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = ui_tray_manager_create(&tray);
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_tray_manager_create(&tray);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_menu_base_create(&menu);
  failed |= (rc != UI_ERROR_NONE);

  memset(&image, 0, sizeof(image));

  rc = ui_tray_manager_set_tooltip(NULL, "Test Tooltip");
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);
  rc = ui_tray_manager_set_tooltip(tray, NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tray_manager_set_tooltip(tray, "Test Tooltip");
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_set_icon(NULL, &image);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);
  rc = ui_tray_manager_set_icon(tray, NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tray_manager_set_icon(tray, &image);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_set_context_menu(NULL, menu);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tray_manager_set_context_menu(tray, NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tray_manager_set_context_menu(tray, menu);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_show(NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tray_manager_show(tray);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_show(tray);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_hide(NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_tray_manager_hide(tray);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_hide(tray);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_show(tray);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_show(tray);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_tray_manager_destroy(NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  {
    ui_error_t rc_cleanup = ui_tray_manager_destroy(tray);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Test destroy while not visible */
  rc = ui_tray_manager_create(&tray);
  failed |= (rc != UI_ERROR_NONE);
  {
    ui_error_t rc_cleanup = ui_tray_manager_destroy(tray);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_menu_base_destroy(menu);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  if (!failed) {
    printf("test_ui_tray_manager passed\n");
  }
  return failed;
}
