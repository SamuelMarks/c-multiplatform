/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_code_block_clipboard_lifecycle(void) {
  cmp_clipboard_overlay_t *overlay = NULL;
  int res;

  res = cmp_clipboard_overlay_create(&overlay);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, overlay);

  res = cmp_clipboard_overlay_destroy(overlay);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_code_block_clipboard_null_args(void) {
  cmp_clipboard_overlay_t *overlay = NULL;
  cmp_code_block_t *block = NULL;
  cmp_window_t *window = NULL;
  int res;

  res = cmp_clipboard_overlay_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_clipboard_overlay_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_clipboard_overlay_create(&overlay);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_code_block_create(&block);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_window_config_t config;
  memset(&config, 0, sizeof(config));
  config.title = "Test";
  config.width = 100;
  config.height = 100;
  cmp_window_system_init();
  res = cmp_window_create(&config, &window);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_clipboard_overlay_render(NULL, block, CMP_CLIPBOARD_STATE_IDLE);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_clipboard_overlay_render(overlay, NULL, CMP_CLIPBOARD_STATE_IDLE);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_clipboard_overlay_copy(NULL, window, "code");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_clipboard_overlay_copy(overlay, NULL, "code");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_clipboard_overlay_copy(overlay, window, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_window_destroy(window);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_clipboard_overlay_destroy(overlay);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_code_block_clipboard_operations(void) {
  cmp_clipboard_overlay_t *overlay = NULL;
  cmp_code_block_t *block = NULL;
  cmp_window_t *window = NULL;
  int res;

  res = cmp_clipboard_overlay_create(&overlay);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_code_block_create(&block);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_window_config_t config;
  memset(&config, 0, sizeof(config));
  config.title = "Test";
  config.width = 100;
  config.height = 100;
  cmp_window_system_init();
  res = cmp_window_create(&config, &window);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_clipboard_overlay_render(overlay, block, CMP_CLIPBOARD_STATE_IDLE);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_clipboard_overlay_copy(overlay, window, "print('hello')");
  /* We might not have a real OS clipboard context in a headless test, but
   * CMP_SUCCESS or a specific OS err is expected */
  /* Here we check that the function call itself doesn't crash and returns an
   * int */
  if (res != CMP_SUCCESS && res != CMP_ERROR_GENERAL &&
      res != CMP_ERROR_INVALID_STATE && res != CMP_ERROR_NOT_FOUND) {
    ASSERT_EQ(CMP_SUCCESS, res); /* Force failure if it's an unexpected error */
  }

  res = cmp_window_destroy(window);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_code_block_destroy(block);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_clipboard_overlay_destroy(overlay);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(code_block_clipboard_suite) {
  RUN_TEST(test_code_block_clipboard_lifecycle);
  RUN_TEST(test_code_block_clipboard_null_args);
  RUN_TEST(test_code_block_clipboard_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(code_block_clipboard_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
