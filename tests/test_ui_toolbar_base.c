/* clang-format off */
#include "ui_toolbar_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
#ifdef UI_TEST_MOCK_ALLOC
extern int g_mock_strcpy_fail;
#endif

static int run_normal_tests(void) {
  struct ui_toolbar_base *tb = NULL;
  ui_error_t rc;
  const char *title = NULL;
  enum ui_toolbar_mode mode;
  enum ui_toolbar_alignment align;
  int failed = 0;

  printf("Testing ui_toolbar_base_create...\n");

  failed |= (ui_toolbar_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_toolbar_base_create(&tb);
  failed |= (rc != UI_ERROR_NONE || !tb);

  printf("Testing getters and setters...\n");

  /* Mode */
  failed |= (ui_toolbar_base_set_mode(NULL, UI_TOOLBAR_MODE_STICKY) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_toolbar_base_get_mode(NULL, &mode) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toolbar_base_get_mode(tb, NULL) != UI_ERROR_INVALID_ARGUMENT);

  ui_toolbar_base_set_mode(tb, UI_TOOLBAR_MODE_STICKY);
  ui_toolbar_base_get_mode(tb, &mode);
  failed |= (mode != UI_TOOLBAR_MODE_STICKY);

  /* Alignment */
  failed |= (ui_toolbar_base_set_alignment(NULL, UI_TOOLBAR_ALIGN_COLUMN) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toolbar_base_get_alignment(NULL, &align) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_toolbar_base_get_alignment(tb, NULL) != UI_ERROR_INVALID_ARGUMENT);

  ui_toolbar_base_set_alignment(tb, UI_TOOLBAR_ALIGN_COLUMN);
  ui_toolbar_base_get_alignment(tb, &align);
  failed |= (align != UI_TOOLBAR_ALIGN_COLUMN);

  /* Title */
  failed |=
      (ui_toolbar_base_set_title(NULL, "Test") != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toolbar_base_set_title(tb, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_toolbar_base_get_title(NULL, &title) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toolbar_base_get_title(tb, NULL) != UI_ERROR_INVALID_ARGUMENT);

#ifdef UI_TEST_MOCK_ALLOC
  /* trigger UI_STRCPY fail if mock alloc enabled */
  g_mock_strcpy_fail = 1;
  (void)ui_toolbar_base_set_title(tb, "Test fail");
  g_mock_strcpy_fail = 0;
#endif

  failed |= (ui_toolbar_base_set_title(tb, "My Toolbar") != UI_ERROR_NONE);

  ui_toolbar_base_get_title(tb, &title);
  failed |= (!title || strcmp(title, "My Toolbar") != 0);

  /* Set title again to trigger free of old title */
  failed |= (ui_toolbar_base_set_title(tb, "Another Title") != UI_ERROR_NONE);
  ui_toolbar_base_get_title(tb, &title);
  failed |= (!title || strcmp(title, "Another Title") != 0);

  /* Bind data */
  failed |=
      (ui_toolbar_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toolbar_base_bind_data(tb, (struct ui_signal *)0x1234) !=
             UI_ERROR_NONE);

  ui_toolbar_base_destroy(tb);
  ui_toolbar_base_destroy(NULL); /* Should be safe */

  return failed;
}

static int run_oom_tests(void) {
  struct ui_toolbar_base *tb = NULL;
  ui_error_t rc;
  int failed = 0;

  printf("Testing OOM on create...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_toolbar_base_create(&tb);
  g_malloc_fail_countdown = -1;
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);

  rc = ui_toolbar_base_create(&tb);
  failed |= (rc != UI_ERROR_NONE);

  printf("Testing OOM on set_title...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_toolbar_base_set_title(tb, "Test OOM");
  g_malloc_fail_countdown = -1;
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);

  ui_toolbar_base_destroy(tb);

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (!failed) {
    printf("All test_ui_toolbar_base passed.\n");
  }
  return failed;
}
