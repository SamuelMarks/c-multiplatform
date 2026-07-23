/* clang-format off */
#include "../include/ui_virtual_keyboard.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <stddef.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static int test_virtual_keyboard_lifecycle(void) {
  struct ui_virtual_keyboard *vk = NULL;
  enum ui_error rc;
  int failed = 0;

  rc = ui_virtual_keyboard_create(NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = ui_virtual_keyboard_create(&vk);
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_virtual_keyboard_create(&vk);
  failed |= (rc != UI_ERROR_NONE || vk == NULL);

  ui_virtual_keyboard_destroy(NULL);
  ui_virtual_keyboard_destroy(vk);
  return failed;
}

static int test_virtual_keyboard_properties(void) {
  struct ui_virtual_keyboard *vk = NULL;
  enum ui_error rc;
  float height;
  int failed = 0;

  rc = ui_virtual_keyboard_create(&vk);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_virtual_keyboard_get_height(NULL, &height);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_virtual_keyboard_get_height(vk, NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_virtual_keyboard_get_height(vk, &height);
  failed |= (rc != UI_ERROR_NONE || height != 0.0f);

  rc = ui_virtual_keyboard_set_height(NULL, 300.0f);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_virtual_keyboard_set_height(vk, 300.0f);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_virtual_keyboard_set_height(vk, 300.0f);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_virtual_keyboard_get_height(vk, &height);
  failed |= (rc != UI_ERROR_NONE || height != 300.0f);

  ui_virtual_keyboard_destroy(vk);
  return failed;
}

static int test_virtual_keyboard_binds(void) {
  struct ui_virtual_keyboard *vk = NULL;
  struct ui_safe_area_manager *sam = (struct ui_safe_area_manager *)0x1234;
  struct ui_signal *sig = (struct ui_signal *)0x5678;
  enum ui_error rc;
  int failed = 0;

  rc = ui_virtual_keyboard_create(&vk);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_virtual_keyboard_bind_safe_area(NULL, sam);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_virtual_keyboard_bind_safe_area(vk, NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_virtual_keyboard_bind_safe_area(vk, sam);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_virtual_keyboard_bind_height_signal(NULL, sig);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_virtual_keyboard_bind_height_signal(vk, NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_virtual_keyboard_bind_height_signal(vk, sig);
  failed |= (rc != UI_ERROR_NONE);

  /* Set height again to trigger the height_signal condition */
  rc = ui_virtual_keyboard_set_height(vk, 400.0f);
  failed |= (rc != UI_ERROR_NONE);

  ui_virtual_keyboard_destroy(vk);
  return failed;
}

int main(void) {
  int failed = 0;
  printf("Running ui_virtual_keyboard tests...\n");

  failed |= test_virtual_keyboard_lifecycle();
  failed |= test_virtual_keyboard_properties();
  failed |= test_virtual_keyboard_binds();

  if (!failed) {
    printf("All tests passed.\n");
  }
  return failed;
}
