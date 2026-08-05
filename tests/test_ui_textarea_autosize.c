/* clang-format off */
#include "ui_textarea_autosize.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  ui_error_t rc;
  struct ui_textarea_autosize_config config;
  struct ui_textarea_autosize_state state;
  int failed = 0;

  rc = ui_textarea_autosize_config_init(NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_textarea_autosize_config_init(&config);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_textarea_autosize_calculate(NULL, 1, &state);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_textarea_autosize_calculate(&config, 1, NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  config.line_height = 20.0f;
  config.padding_top = 10.0f;
  config.padding_bottom = 10.0f;
  config.min_height = 50.0f;
  config.max_height = 100.0f;

  /* Test 1: Content < min_height */
  rc = ui_textarea_autosize_calculate(&config, 1, &state);
  failed |= (rc != UI_ERROR_NONE || state.content_height != 40.0f ||
             state.target_height != 50.0f || state.has_scrollbar != 0);

  /* Test 2: min_height < Content < max_height */
  rc = ui_textarea_autosize_calculate(&config, 3, &state);
  failed |= (rc != UI_ERROR_NONE || state.content_height != 80.0f ||
             state.target_height != 80.0f || state.has_scrollbar != 0);

  /* Test 3: Content > max_height */
  rc = ui_textarea_autosize_calculate(&config, 10, &state);
  failed |= (rc != UI_ERROR_NONE || state.content_height != 220.0f ||
             state.target_height != 100.0f || state.has_scrollbar == 0);

  /* Test 4: Unbounded max_height */
  config.max_height = 0.0f;
  rc = ui_textarea_autosize_calculate(&config, 10, &state);
  failed |= (rc != UI_ERROR_NONE || state.content_height != 220.0f ||
             state.target_height != 220.0f || state.has_scrollbar != 0);

  /* Test 5: Unbounded min_height */
  config.min_height = 0.0f;
  rc = ui_textarea_autosize_calculate(&config, 1, &state);
  failed |= (rc != UI_ERROR_NONE || state.content_height != 40.0f ||
             state.target_height != 40.0f || state.has_scrollbar != 0);

  if (!failed) {
    printf("All autosize textarea tests passed.\n");
  }

  return failed;
}
