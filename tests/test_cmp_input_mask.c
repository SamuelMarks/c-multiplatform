/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_input_mask_lifecycle(void) {
  cmp_input_mask_t *mask = NULL;
  int res = cmp_input_mask_create("(XXX) XXX-XXXX", &mask);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, mask);

  res = cmp_input_mask_destroy(mask);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_input_mask_null_args(void) {
  int res = cmp_input_mask_create(NULL, NULL);
  cmp_input_mask_t *mask = NULL;
  char buf[32];

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_input_mask_create(NULL, &mask);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_input_mask_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_input_mask_create("XXX", &mask);
  res = cmp_input_mask_apply(NULL, "123", buf, 32);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_input_mask_apply(mask, NULL, buf, 32);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_input_mask_apply(mask, "123", NULL, 32);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_input_mask_destroy(mask);
  PASS();
}

TEST test_input_mask_apply(void) {
  cmp_input_mask_t *mask = NULL;
  char buf[32];
  int res;
  cmp_input_mask_create("(XXX) XXX-XXXX", &mask);

  /* Partial input */
  res = cmp_input_mask_apply(mask, "123", buf, 32);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("(123) ", buf);

  /* Full input */
  res = cmp_input_mask_apply(mask, "1234567890", buf, 32);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("(123) 456-7890", buf);

  cmp_input_mask_destroy(mask);
  PASS();
}

SUITE(input_mask_suite) {
  RUN_TEST(test_input_mask_lifecycle);
  RUN_TEST(test_input_mask_null_args);
  RUN_TEST(test_input_mask_apply);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(input_mask_suite);
  GREATEST_MAIN_END();
}
