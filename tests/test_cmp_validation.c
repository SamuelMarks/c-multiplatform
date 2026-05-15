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

  /* Setters null args */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_validation_set_regex(NULL, ".*"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_validation_set_min_length(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_validation_set_max_length(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_validation_set_required(NULL, 1));

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

  /* No constraints applied yet, empty should be valid since not required */
  res = cmp_validation_check(validation, "", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_valid);

  /* Required */
  cmp_validation_set_required(validation, 1);
  res = cmp_validation_check(validation, "", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, is_valid);

  res = cmp_validation_check(validation, "a", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_valid);

  /* Min Length */
  cmp_validation_set_min_length(validation, 3);
  res = cmp_validation_check(validation, "ab", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, is_valid);

  res = cmp_validation_check(validation, "abc", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_valid);

  /* Max Length */
  cmp_validation_set_max_length(validation, 5);
  res = cmp_validation_check(validation, "abcdef", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, is_valid);

  res = cmp_validation_check(validation, "abcde", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_valid);

  /* Regex */
  cmp_validation_set_regex(validation, "^a.*c$");
  res = cmp_validation_check(validation, "abc", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_valid);

  res = cmp_validation_check(validation, "ab1", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, is_valid); /* 1 doesn't match regex logic */

  /* Change Regex to match our simple rob pike subset: ^hello */
  cmp_validation_set_regex(validation, "^hello");
  res = cmp_validation_check(validation, "hello", &is_valid);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_valid);

  res = cmp_validation_check(validation, "world", &is_valid);
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
