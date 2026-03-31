/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_validation_lifecycle(void) {
  cmp_validation_t *validation = NULL;
  int res = cmp_validation_create(&validation);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, validation);

  res = cmp_validation_destroy(validation);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_validation_null_args(void) {
  int res = cmp_validation_create(NULL);
  int is_valid;
  cmp_validation_t *validation = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_validation_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_validation_check(NULL, "test", &is_valid);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_validation_create(&validation);
  res = cmp_validation_check(validation, NULL, &is_valid);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_validation_check(validation, "test", NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_validation_destroy(validation);
  PASS();
}

TEST test_validation_check(void) {
  cmp_validation_t *validation = NULL;
  int is_valid;
  int res;

  cmp_validation_create(&validation);

  res = cmp_validation_check(validation, "hello", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_valid);

  res = cmp_validation_check(validation, "", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, is_valid);

  cmp_validation_destroy(validation);
  PASS();
}

SUITE(validation_suite) {
  RUN_TEST(test_validation_lifecycle);
  RUN_TEST(test_validation_null_args);
  RUN_TEST(test_validation_check);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(validation_suite);
  GREATEST_MAIN_END();
}
