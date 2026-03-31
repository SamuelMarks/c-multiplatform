/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_i18n_suite);

TEST test_cmp_i18n_create_destroy(void) {
  cmp_i18n_t *i18n = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_create(&i18n));
  ASSERT_NEQ(NULL, i18n);

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_destroy(i18n));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_destroy(NULL));

  PASS();
}

TEST test_cmp_i18n_strings(void) {
  cmp_i18n_t *i18n = NULL;
  char *value = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_create(&i18n));

  /* Add and retrieve valid strings */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_i18n_add_string(i18n, "en", "hello", "Hello World"));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_i18n_add_string(i18n, "fr", "hello", "Bonjour le monde"));

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_get_string(i18n, "en", "hello", &value));
  ASSERT_NEQ(NULL, value);
  ASSERT_STR_EQ("Hello World", value);
  CMP_FREE(value);

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_get_string(i18n, "fr", "hello", &value));
  ASSERT_NEQ(NULL, value);
  ASSERT_STR_EQ("Bonjour le monde", value);
  CMP_FREE(value);

  /* Missing keys/locales */
  ASSERT_EQ(CMP_ERROR_NOT_FOUND,
            cmp_i18n_get_string(i18n, "es", "hello", &value));
  ASSERT_EQ(CMP_ERROR_NOT_FOUND,
            cmp_i18n_get_string(i18n, "en", "goodbye", &value));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_add_string(NULL, "en", "k", "v"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_add_string(i18n, NULL, "k", "v"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_add_string(i18n, "en", NULL, "v"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_add_string(i18n, "en", "k", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_get_string(NULL, "en", "k", &value));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_get_string(i18n, NULL, "k", &value));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_get_string(i18n, "en", NULL, &value));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_get_string(i18n, "en", "k", NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_destroy(i18n));
  PASS();
}

SUITE(cmp_i18n_suite) {
  RUN_TEST(test_cmp_i18n_create_destroy);
  RUN_TEST(test_cmp_i18n_strings);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_i18n_suite);
  GREATEST_MAIN_END();
}
