/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_overdraw_create_destroy(void) {
  cmp_overdraw_t *overdraw = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_overdraw_create(&overdraw));
  ASSERT_NEQ(NULL, overdraw);

  ASSERT_EQ(CMP_SUCCESS, cmp_overdraw_destroy(overdraw));
  PASS();
}

TEST test_overdraw_set_enabled(void) {
  cmp_overdraw_t *overdraw = NULL;
  cmp_renderer_t *renderer = NULL;
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;

  cmp_window_system_init();
  cmp_window_create(&cfg, &win);
  cmp_renderer_create(win, CMP_RENDER_BACKEND_DEFAULT, &renderer);

  ASSERT_EQ(CMP_SUCCESS, cmp_overdraw_create(&overdraw));

  /* Enable */
  ASSERT_EQ(CMP_SUCCESS, cmp_overdraw_set_enabled(overdraw, renderer, 1));

  /* Disable */
  ASSERT_EQ(CMP_SUCCESS, cmp_overdraw_set_enabled(overdraw, renderer, 0));

  ASSERT_EQ(CMP_SUCCESS, cmp_overdraw_destroy(overdraw));
  cmp_renderer_destroy(renderer);
  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_overdraw_edge_cases(void) {
  cmp_overdraw_t *overdraw = NULL;
  cmp_renderer_t *renderer = NULL;
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 1};
  cmp_window_t *win = NULL;

  cmp_window_system_init();
  cmp_window_create(&cfg, &win);
  cmp_renderer_create(win, CMP_RENDER_BACKEND_DEFAULT, &renderer);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_overdraw_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_overdraw_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_overdraw_create(&overdraw));

  /* Null pointer arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_overdraw_set_enabled(NULL, renderer, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_overdraw_set_enabled(overdraw, NULL, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_overdraw_destroy(overdraw));
  cmp_renderer_destroy(renderer);
  cmp_window_destroy(win);
  cmp_window_system_shutdown();
  PASS();
}
SUITE(cmp_overdraw_suite) {
  RUN_TEST(test_overdraw_create_destroy);
  RUN_TEST(test_overdraw_set_enabled);
  RUN_TEST(test_overdraw_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_overdraw_suite);
  GREATEST_MAIN_END();
}
