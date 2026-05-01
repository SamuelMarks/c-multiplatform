/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_breadcrumbs_lifecycle(void) {
  cmp_breadcrumbs_t *crumbs = NULL;
  int res;

  res = cmp_breadcrumbs_create(&crumbs);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, crumbs);

  res = cmp_breadcrumbs_destroy(crumbs);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_breadcrumbs_null_args(void) {
  cmp_breadcrumbs_t *crumbs = NULL;
  int res;

  res = cmp_breadcrumbs_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_breadcrumbs_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_breadcrumbs_create(&crumbs);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_breadcrumbs_set_path(NULL, "/usr");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_breadcrumbs_set_path(crumbs, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_breadcrumbs_get_count(NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_breadcrumbs_get_segment(NULL, 0, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_breadcrumbs_destroy(crumbs);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_breadcrumbs_set_and_get(void) {
  cmp_breadcrumbs_t *crumbs = NULL;
  cmp_breadcrumb_t *segment = NULL;
  size_t count = 0;
  int res;

  res = cmp_breadcrumbs_create(&crumbs);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_breadcrumbs_set_path(crumbs, "/usr/local/bin");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_breadcrumbs_get_count(crumbs, &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(3, (int)count);

  res = cmp_breadcrumbs_get_segment(crumbs, 0, &segment);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("usr", segment->label);
  ASSERT_EQ(0, segment->is_active);

  res = cmp_breadcrumbs_get_segment(crumbs, 2, &segment);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("bin", segment->label);
  ASSERT_EQ(1, segment->is_active);

  res = cmp_breadcrumbs_get_segment(crumbs, 99, &segment);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_breadcrumbs_destroy(crumbs);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(breadcrumbs_suite) {
  RUN_TEST(test_breadcrumbs_lifecycle);
  RUN_TEST(test_breadcrumbs_null_args);
  RUN_TEST(test_breadcrumbs_set_and_get);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(breadcrumbs_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
