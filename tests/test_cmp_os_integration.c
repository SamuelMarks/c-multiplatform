/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_os_integration_clipboard(void) {
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;
  int rc;

  cmp_window_system_init();
  if (cmp_window_create(&cfg, &win) == CMP_SUCCESS) {
    rc = cmp_os_copy_to_clipboard(win, "hello world");
    ASSERT(rc == CMP_SUCCESS || rc == CMP_ERROR_NOT_FOUND);
    cmp_window_destroy(win);
  }

  cmp_window_system_shutdown();
  PASS();
}

TEST test_os_integration_drag_drop(void) {
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;

  cmp_window_system_init();
  if (cmp_window_create(&cfg, &win) == CMP_SUCCESS) {
    ASSERT_EQ(CMP_SUCCESS, cmp_os_enable_file_drag_drop(win));
    cmp_window_destroy(win);
  }

  cmp_window_system_shutdown();
  PASS();
}

TEST test_os_integration_voice(void) {
  int supported = 0;
  ASSERT_EQ(CMP_SUCCESS, cmp_os_is_voice_dictation_supported(&supported));
  ASSERT_EQ(1, supported);
  ASSERT_EQ(CMP_SUCCESS, cmp_os_start_voice_dictation());
  PASS();
}

TEST test_os_integration_null(void) {
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_copy_to_clipboard(NULL, "test"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_enable_file_drag_drop(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_is_voice_dictation_supported(NULL));

  cmp_window_system_init();
  if (cmp_window_create(&cfg, &win) == CMP_SUCCESS) {
    ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_copy_to_clipboard(win, NULL));
    cmp_window_destroy(win);
  }
  cmp_window_system_shutdown();
  PASS();
}

SUITE(cmp_os_integration_suite) {
  RUN_TEST(test_os_integration_clipboard);
  RUN_TEST(test_os_integration_drag_drop);
  RUN_TEST(test_os_integration_voice);
  RUN_TEST(test_os_integration_null);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_os_integration_suite);
  GREATEST_MAIN_END();
}
