/* clang-format off */
#include "cmp.h"
#include "cmp_regex.h"
#include "greatest.h"
/* clang-format on */

SUITE(cmp_regex_suite);

TEST test_regex_lifecycle(void) {
  cmp_regex_t *regex = NULL;

  /* Null arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_regex_compile(NULL, "pattern"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_regex_compile(&regex, NULL));

  /* Successful compile */
  ASSERT_EQ(CMP_SUCCESS, cmp_regex_compile(&regex, "hello"));
  ASSERT_NEQ(NULL, regex);

  /* Null argument for free */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_regex_free(NULL));

  /* Successful free */
  ASSERT_EQ(CMP_SUCCESS, cmp_regex_free(regex));

  PASS();
}

TEST test_regex_match(void) {
  cmp_regex_t *regex = NULL;
  int matched = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_regex_compile(&regex, "world"));

  /* Null arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_regex_match(NULL, "hello world", &matched));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_regex_match(regex, NULL, &matched));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_regex_match(regex, "hello world", NULL));

  /* Successful match */
  ASSERT_EQ(CMP_SUCCESS, cmp_regex_match(regex, "hello world", &matched));
  ASSERT_EQ(1, matched);

  /* Unsuccessful match */
  ASSERT_EQ(CMP_SUCCESS, cmp_regex_match(regex, "hello there", &matched));
  ASSERT_EQ(0, matched);

  ASSERT_EQ(CMP_SUCCESS, cmp_regex_free(regex));

  PASS();
}

SUITE(cmp_regex_suite) {
  RUN_TEST(test_regex_lifecycle);
  RUN_TEST(test_regex_match);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_regex_suite);
  GREATEST_MAIN_END();
}