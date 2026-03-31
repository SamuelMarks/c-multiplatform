/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_spellcheck_lifecycle(void) {
  cmp_spellcheck_t *spellcheck = NULL;
  int res = cmp_spellcheck_create(&spellcheck);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, spellcheck);

  res = cmp_spellcheck_destroy(spellcheck);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_spellcheck_null_args(void) {
  int res = cmp_spellcheck_create(NULL);
  int v;
  cmp_spellcheck_t *spellcheck = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_spellcheck_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_spellcheck_verify_word(NULL, "test", &v);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_spellcheck_create(&spellcheck);
  res = cmp_spellcheck_verify_word(spellcheck, NULL, &v);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_spellcheck_verify_word(spellcheck, "test", NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_spellcheck_destroy(spellcheck);
  PASS();
}

TEST test_spellcheck_verify(void) {
  cmp_spellcheck_t *spellcheck = NULL;
  int v;
  int res;

  cmp_spellcheck_create(&spellcheck);

  res = cmp_spellcheck_verify_word(spellcheck, "hello", &v);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, v);

  res = cmp_spellcheck_verify_word(spellcheck, "xylophone", &v);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, v);

  cmp_spellcheck_destroy(spellcheck);
  PASS();
}

SUITE(spellcheck_suite) {
  RUN_TEST(test_spellcheck_lifecycle);
  RUN_TEST(test_spellcheck_null_args);
  RUN_TEST(test_spellcheck_verify);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(spellcheck_suite);
  GREATEST_MAIN_END();
}
