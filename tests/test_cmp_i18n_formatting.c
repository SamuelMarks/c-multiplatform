/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_i18n_formatting_features(void) {
  cmp_i18n_formatting_t *ctx = NULL;
  int dummy_node = 0;
  char buf[256];
  const char *list[] = {"A", "B", "C"};
  int sort_res;

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_formatting_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_handle_dynamic_expansion(ctx, &dummy_node));
  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_load_stringsdict(ctx, "<plist/>"));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_i18n_format_date(ctx, 1600000000, buf, sizeof(buf)));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_i18n_format_currency(ctx, 99.99, "USD", buf, sizeof(buf)));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_i18n_format_person_name(ctx, "John", "Doe", buf, sizeof(buf)));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_i18n_format_measurement(ctx, 10.5, "km", buf, sizeof(buf)));

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_format_list(ctx, list, 3, buf, sizeof(buf)));
  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_apply_cjk_vertical_text(ctx, &dummy_node));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_i18n_localized_sort(ctx, "apple", "Apple", &sort_res));

  ASSERT_EQ(CMP_SUCCESS, cmp_i18n_formatting_destroy(ctx));
  PASS();
}

TEST test_i18n_formatting_null_args(void) {
  cmp_i18n_formatting_t *feat = NULL;
  int node = 0;
  char buf[256];
  const char *list[] = {"A"};
  int res;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_formatting_create(NULL));
  cmp_i18n_formatting_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_handle_dynamic_expansion(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_handle_dynamic_expansion(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_load_stringsdict(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_load_stringsdict(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_format_date(NULL, 0, buf, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_format_date(feat, 0, NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_format_date(feat, 0, buf, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_format_currency(NULL, 1.0, "USD", buf, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_format_currency(feat, 1.0, NULL, buf, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_format_person_name(NULL, "A", "B", buf, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_format_person_name(feat, NULL, "B", buf, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_format_person_name(feat, "A", NULL, buf, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_format_measurement(NULL, 1.0, "km", buf, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_format_measurement(feat, 1.0, NULL, buf, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_format_list(NULL, list, 1, buf, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_format_list(feat, NULL, 1, buf, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_i18n_format_list(feat, list, 0, buf, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_apply_cjk_vertical_text(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_apply_cjk_vertical_text(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_localized_sort(NULL, "a", "b", &res));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_localized_sort(feat, NULL, "b", &res));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_localized_sort(feat, "a", NULL, &res));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_i18n_localized_sort(feat, "a", "b", NULL));

  cmp_i18n_formatting_destroy(feat);
  PASS();
}

SUITE(i18n_formatting_suite) {
  RUN_TEST(test_i18n_formatting_features);
  RUN_TEST(test_i18n_formatting_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(i18n_formatting_suite);
  GREATEST_MAIN_END();
}
