/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_caret_lifecycle(void) {
  cmp_caret_t *caret = NULL;
  int res = cmp_caret_create(&caret);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, caret);

  res = cmp_caret_destroy(caret);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_caret_null_args(void) {
  int res = cmp_caret_create(NULL);
  int v;
  cmp_caret_t *caret = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_caret_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_caret_update_blink(NULL, 16.6, &v);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_caret_create(&caret);
  res = cmp_caret_update_blink(caret, 16.6, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_caret_update_blink(caret, -1.0, &v);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_caret_destroy(caret);
  PASS();
}

TEST test_caret_blink(void) {
  cmp_caret_t *caret = NULL;
  int v;
  int res;

  cmp_caret_create(&caret);

  /* Initial state is visible */
  res = cmp_caret_update_blink(caret, 0.0, &v);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, v);

  /* Half a second later, should turn off */
  res = cmp_caret_update_blink(caret, 500.0, &v);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, v);

  /* Another half second, should turn back on */
  res = cmp_caret_update_blink(caret, 500.0, &v);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, v);

  cmp_caret_destroy(caret);
  PASS();
}

SUITE(caret_suite) {
  RUN_TEST(test_caret_lifecycle);
  RUN_TEST(test_caret_null_args);
  RUN_TEST(test_caret_blink);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(caret_suite);
  GREATEST_MAIN_END();
}
