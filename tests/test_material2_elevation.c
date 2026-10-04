/**
 * @file test_material2_elevation.c
 * @brief Unit tests for Material Design 2 elevation definitions.
 */

/* clang-format off */
#include "material2/md2_elevation.h"
#include "ui_error.h"
#include <greatest.h>
/* clang-format on */

TEST test_md2_elevation_get_dp_success(void) {
  float dp = -1.0f;
  ui_error_t rc;

  rc = md2_elevation_get_dp(MD2_ELEVATION_0DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(0.0f, dp, "%f");

  rc = md2_elevation_get_dp(MD2_ELEVATION_1DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(1.0f, dp, "%f");

  rc = md2_elevation_get_dp(MD2_ELEVATION_2DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(2.0f, dp, "%f");

  rc = md2_elevation_get_dp(MD2_ELEVATION_3DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(3.0f, dp, "%f");

  rc = md2_elevation_get_dp(MD2_ELEVATION_4DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(4.0f, dp, "%f");

  rc = md2_elevation_get_dp(MD2_ELEVATION_6DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(6.0f, dp, "%f");

  rc = md2_elevation_get_dp(MD2_ELEVATION_8DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(8.0f, dp, "%f");

  rc = md2_elevation_get_dp(MD2_ELEVATION_12DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(12.0f, dp, "%f");

  rc = md2_elevation_get_dp(MD2_ELEVATION_16DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(16.0f, dp, "%f");

  rc = md2_elevation_get_dp(MD2_ELEVATION_24DP, &dp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(24.0f, dp, "%f");

  PASS();
}

TEST test_md2_elevation_get_dp_invalid_args(void) {
  float dp;
  ui_error_t rc;

  rc = md2_elevation_get_dp(MD2_ELEVATION_0DP, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_elevation_get_dp((enum md2_elevation_level)999, &dp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  PASS();
}

SUITE(material2_elevation_suite) {
  RUN_TEST(test_md2_elevation_get_dp_success);
  RUN_TEST(test_md2_elevation_get_dp_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_elevation_suite);
  GREATEST_MAIN_END();
}
