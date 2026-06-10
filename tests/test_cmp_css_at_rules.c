/* clang-format off */
#include "greatest.h"
#include "cmp_css_at_rules.h"
/* clang-format on */

TEST test_media_rule(void) {
  cmp_at_rule_media_t rule;

  ASSERT_EQ(0, cmp_at_rule_media_init(&rule, "(max-width: 600px)"));
  ASSERT_STR_EQ("(max-width: 600px)", rule.condition_text);

  ASSERT_EQ(0, cmp_at_rule_media_free(&rule));
  ASSERT_EQ(NULL, rule.condition_text);

  ASSERT_EQ(-1, cmp_at_rule_media_init(NULL, ""));
  ASSERT_EQ(-1, cmp_at_rule_media_free(NULL));

  PASS();
}

TEST test_container_rule(void) {
  cmp_at_rule_container_t rule;

  ASSERT_EQ(
      0, cmp_at_rule_container_init(&rule, "my-layout", "(min-width: 300px)"));
  ASSERT_STR_EQ("my-layout", rule.name);
  ASSERT_STR_EQ("(min-width: 300px)", rule.condition_text);

  ASSERT_EQ(0, cmp_at_rule_container_free(&rule));
  ASSERT_EQ(NULL, rule.name);
  ASSERT_EQ(NULL, rule.condition_text);

  ASSERT_EQ(-1, cmp_at_rule_container_init(NULL, "", ""));
  ASSERT_EQ(-1, cmp_at_rule_container_free(NULL));

  PASS();
}

TEST test_supports_rule(void) {
  cmp_at_rule_supports_t rule;

  ASSERT_EQ(0, cmp_at_rule_supports_init(&rule, "(display: grid)"));
  ASSERT_STR_EQ("(display: grid)", rule.condition_text);

  ASSERT_EQ(0, cmp_at_rule_supports_free(&rule));
  ASSERT_EQ(NULL, rule.condition_text);

  ASSERT_EQ(-1, cmp_at_rule_supports_init(NULL, ""));
  ASSERT_EQ(-1, cmp_at_rule_supports_free(NULL));

  PASS();
}

TEST test_import_rule(void) {
  cmp_at_rule_import_t rule;

  ASSERT_EQ(0, cmp_at_rule_import_init(&rule, "styles.css", "utilities",
                                       "not (display: grid)", "screen"));
  ASSERT_STR_EQ("styles.css", rule.url);
  ASSERT_STR_EQ("utilities", rule.layer_name);
  ASSERT_STR_EQ("not (display: grid)", rule.supports_cond);
  ASSERT_STR_EQ("screen", rule.media_query);

  ASSERT_EQ(0, cmp_at_rule_import_free(&rule));
  ASSERT_EQ(NULL, rule.url);
  ASSERT_EQ(NULL, rule.layer_name);
  ASSERT_EQ(NULL, rule.supports_cond);
  ASSERT_EQ(NULL, rule.media_query);

  ASSERT_EQ(-1, cmp_at_rule_import_init(NULL, "", "", "", ""));
  ASSERT_EQ(-1, cmp_at_rule_import_free(NULL));

  PASS();
}

TEST test_viewport_rule(void) {
  cmp_at_rule_viewport_t rule;

  ASSERT_EQ(0, cmp_at_rule_viewport_init(&rule, "width: device-width;"));
  ASSERT_STR_EQ("width: device-width;", rule.declarations);

  ASSERT_EQ(0, cmp_at_rule_viewport_free(&rule));
  ASSERT_EQ(NULL, rule.declarations);

  ASSERT_EQ(-1, cmp_at_rule_viewport_init(NULL, ""));
  ASSERT_EQ(-1, cmp_at_rule_viewport_free(NULL));

  PASS();
}

TEST test_charset_rule(void) {
  cmp_at_rule_charset_t rule;

  ASSERT_EQ(0, cmp_at_rule_charset_init(&rule, "utf-8"));
  ASSERT_STR_EQ("utf-8", rule.charset);

  ASSERT_EQ(0, cmp_at_rule_charset_free(&rule));
  ASSERT_EQ(NULL, rule.charset);

  ASSERT_EQ(-1, cmp_at_rule_charset_init(NULL, ""));
  ASSERT_EQ(-1, cmp_at_rule_charset_free(NULL));

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_media_rule);
  RUN_TEST(test_container_rule);
  RUN_TEST(test_supports_rule);
  RUN_TEST(test_import_rule);
  RUN_TEST(test_viewport_rule);
  RUN_TEST(test_charset_rule);
  GREATEST_MAIN_END();
}
