/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_material3_typography.h"
/* clang-format on */

TEST test_typography_resolve(void) {
  cmp_m3_type_metrics_t metrics;
  int rc;

  rc = cmp_m3_typography_resolve(CMP_M3_TYPESCALE_DISPLAY_LARGE, &metrics);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_EQ(57.0f, metrics.font_size);

  rc = cmp_m3_typography_resolve(CMP_M3_TYPESCALE_LABEL_SMALL, &metrics);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_EQ(11.0f, metrics.font_size);

  rc = cmp_m3_typography_resolve((cmp_m3_typescale_category_t)999, &metrics);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_m3_typography_resolve(CMP_M3_TYPESCALE_DISPLAY_LARGE, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  PASS();
}

TEST test_typography_family(void) {
  const char *family = NULL;
  int rc;

  rc = cmp_m3_typography_get_family(1, &family);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_STR_EQ("Roboto", family);

  rc = cmp_m3_typography_get_family(0, &family);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_STR_EQ("Roboto", family);

  rc = cmp_m3_typography_get_family(1, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  PASS();
}

SUITE(cmp_material3_typography_suite) {
  RUN_TEST(test_typography_resolve);
  RUN_TEST(test_typography_family);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_typography_suite);
  GREATEST_MAIN_END();
}