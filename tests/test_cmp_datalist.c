/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_datalist_lifecycle(void) {
  cmp_datalist_t *datalist = NULL;
  int res = cmp_datalist_create(&datalist);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, datalist);

  res = cmp_datalist_destroy(datalist);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_datalist_null_args(void) {
  int res = cmp_datalist_create(NULL);
  cmp_datalist_t *datalist = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_datalist_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_datalist_filter(NULL, "test");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_datalist_create(&datalist);
  res = cmp_datalist_filter(datalist, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_datalist_destroy(datalist);
  PASS();
}

TEST test_datalist_filter(void) {
  cmp_datalist_t *datalist = NULL;
  int res;
  cmp_datalist_create(&datalist);

  res = cmp_datalist_filter(datalist, "apple");
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_datalist_destroy(datalist);
  PASS();
}

SUITE(datalist_suite) {
  RUN_TEST(test_datalist_lifecycle);
  RUN_TEST(test_datalist_null_args);
  RUN_TEST(test_datalist_filter);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(datalist_suite);
  GREATEST_MAIN_END();
}
