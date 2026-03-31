/* clang-format off */
#include <cmp.h>
#include <greatest.h>
/* clang-format on */

SUITE(cmp_prefers_color_scheme_suite);

TEST test_cmp_prefers_color_scheme_create_destroy(void) {
  cmp_prefers_color_scheme_t *scheme = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_prefers_color_scheme_create(&scheme));
  ASSERT_NEQ(NULL, scheme);

  ASSERT_EQ(CMP_SUCCESS, cmp_prefers_color_scheme_destroy(scheme));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_prefers_color_scheme_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_prefers_color_scheme_destroy(NULL));

  PASS();
}

TEST test_cmp_prefers_color_scheme_get_set(void) {
  cmp_prefers_color_scheme_t *scheme = NULL;
  cmp_color_scheme_t current_scheme;

  ASSERT_EQ(CMP_SUCCESS, cmp_prefers_color_scheme_create(&scheme));

  /* Check default */
  ASSERT_EQ(CMP_SUCCESS, cmp_prefers_color_scheme_get(scheme, &current_scheme));
  ASSERT_EQ(CMP_COLOR_SCHEME_LIGHT, current_scheme);

  /* Set dark */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_prefers_color_scheme_set(scheme, CMP_COLOR_SCHEME_DARK));
  ASSERT_EQ(CMP_SUCCESS, cmp_prefers_color_scheme_get(scheme, &current_scheme));
  ASSERT_EQ(CMP_COLOR_SCHEME_DARK, current_scheme);

  /* Set light */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_prefers_color_scheme_set(scheme, CMP_COLOR_SCHEME_LIGHT));
  ASSERT_EQ(CMP_SUCCESS, cmp_prefers_color_scheme_get(scheme, &current_scheme));
  ASSERT_EQ(CMP_COLOR_SCHEME_LIGHT, current_scheme);

  /* Null checks & invalid args */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_prefers_color_scheme_set(NULL, CMP_COLOR_SCHEME_DARK));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_prefers_color_scheme_set(scheme, (cmp_color_scheme_t)999));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_prefers_color_scheme_get(NULL, &current_scheme));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_prefers_color_scheme_get(scheme, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_prefers_color_scheme_destroy(scheme));
  PASS();
}

SUITE(cmp_prefers_color_scheme_suite) {
  RUN_TEST(test_cmp_prefers_color_scheme_create_destroy);
  RUN_TEST(test_cmp_prefers_color_scheme_get_set);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_prefers_color_scheme_suite);
  GREATEST_MAIN_END();
}
