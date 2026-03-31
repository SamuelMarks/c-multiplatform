/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_cmp_dpi_create_destroy(void) {
  cmp_dpi_t *dpi = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_create(&dpi));
  ASSERT_NEQ(NULL, dpi);
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_destroy(dpi));
  PASS();
}

TEST test_cmp_dpi_set_get_scale(void) {
  cmp_dpi_t *dpi = NULL;
  float scale = 0.0f;
  int err;

  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_create(&dpi));

  /* Add a monitor */
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_set_monitor_scale(dpi, 1, 1.5f));

  /* Add another */
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_set_monitor_scale(dpi, 2, 2.0f));

  /* Update existing */
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_set_monitor_scale(dpi, 1, 1.75f));

  /* Retrieve */
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_get_monitor_scale(dpi, 1, &scale));
  ASSERT_EQ(1.75f, scale);

  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_get_monitor_scale(dpi, 2, &scale));
  ASSERT_EQ(2.0f, scale);

  /* Retrieve non-existent */
  err = cmp_dpi_get_monitor_scale(dpi, 3, &scale);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, err);
  ASSERT_EQ(1.0f, scale);

  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_destroy(dpi));
  PASS();
}

TEST test_cmp_dpi_update_window_scale(void) {
  cmp_dpi_t *dpi = NULL;
  cmp_window_config_t cfg = {"Test", 800, 600, 0, 0, 1, 0, 0};
  cmp_window_t *win = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_window_system_init());

  ASSERT_EQ(CMP_SUCCESS, cmp_window_create(&cfg, &win));
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_create(&dpi));

  /* It should handle non-existent monitor smoothly */
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_update_window_scale(dpi, win, 1));

  /* It should handle existent monitor smoothly */
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_set_monitor_scale(dpi, 1, 2.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_update_window_scale(dpi, win, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_dpi_destroy(dpi));
  ASSERT_EQ(CMP_SUCCESS, cmp_window_destroy(win));
  ASSERT_EQ(CMP_SUCCESS, cmp_window_system_shutdown());

  PASS();
}

SUITE(cmp_dpi_suite) {
  RUN_TEST(test_cmp_dpi_create_destroy);
  RUN_TEST(test_cmp_dpi_set_get_scale);
  RUN_TEST(test_cmp_dpi_update_window_scale);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_dpi_suite);
  GREATEST_MAIN_END();
}
