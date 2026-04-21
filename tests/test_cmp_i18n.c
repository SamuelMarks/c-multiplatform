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

TEST test_cmp_i18n_global_translate(void) {
  cmp_string_t s;
  FILE *f;

  cmp_vfs_init();
  f = fopen("path", "wb");
  if (f) {
    fwrite("dummy=text\n", 1, 11, f);
    fclose(f);
  }

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_init());
  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_load_catalog("path", "en"));

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_translate("unknown_key", &s));
  ASSERT_STR_EQ("unknown_key", s.data);
  CMP_FREE(s.data);

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_translate_plural("item", 1, &s));
  ASSERT_STR_EQ("item", s.data);
  CMP_FREE(s.data);

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_translate_plural("item", 2, &s));
  ASSERT_STR_EQ("item_plural", s.data);
  CMP_FREE(s.data);

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_shutdown());
  PASS();
}

TEST test_cmp_i18n_format(void) {
  cmp_string_t out;

  /* Missing positional args are naturally ignored in our simplistic c-format
     string, but we check that we format correctly */
  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_format("Hello %1$s, you have %2$d messages.",
                                         &out, "Alice", 5));
  ASSERT_STR_EQ("Hello Alice, you have 5 messages.", out.data);
  CMP_FREE(out.data);

  /* Cyclical / re-ordered args */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_i18n_format("Messages: %2$d. User: %1$s.", &out, "Bob", 10));
  ASSERT_STR_EQ("Messages: 10. User: Bob.", out.data);
  CMP_FREE(out.data);

  PASS();
}

TEST test_cmp_i18n_bidi(void) {
  /* Test RTL string handling and BIDI direction states */
  cmp_i18n_set_bidi_direction(CMP_TEXT_DIR_LTR);
  ASSERT_EQ(CMP_TEXT_DIR_LTR, cmp_i18n_get_bidi_direction());
  ASSERT_EQ(0, cmp_i18n_is_rtl());

  cmp_i18n_set_bidi_direction(CMP_TEXT_DIR_RTL);
  ASSERT_EQ(CMP_TEXT_DIR_RTL, cmp_i18n_get_bidi_direction());
  ASSERT_EQ(1, cmp_i18n_is_rtl());

  cmp_i18n_set_bidi_direction(CMP_TEXT_DIR_LTR);

  PASS();
}

SUITE(cmp_i18n_suite) {
  RUN_TEST(test_cmp_i18n_create_destroy);
  RUN_TEST(test_cmp_i18n_strings);
  RUN_TEST(test_cmp_i18n_global_translate);
  RUN_TEST(test_cmp_i18n_format);
  RUN_TEST(test_cmp_i18n_bidi);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_i18n_suite);
  GREATEST_MAIN_END();
}
