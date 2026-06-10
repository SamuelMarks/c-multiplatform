/* clang-format off */
#include "greatest.h"
#include "cmp_cssom.h"
/* clang-format on */

TEST test_cssom_rule(void) {
  cmp_cssom_rule_t rule;

  ASSERT_EQ(0, cmp_cssom_rule_init(&rule, CMP_CSSOM_RULE_STYLE, "color: red;"));
  ASSERT_EQ(CMP_CSSOM_RULE_STYLE, rule.type);
  ASSERT_STR_EQ("color: red;", rule.css_text);
  ASSERT_EQ(NULL, rule.parent_rule);

  ASSERT_EQ(0, cmp_cssom_rule_free(&rule));
  ASSERT_EQ(NULL, rule.css_text);

  ASSERT_EQ(-1, cmp_cssom_rule_init(NULL, CMP_CSSOM_RULE_UNKNOWN, ""));
  ASSERT_EQ(-1, cmp_cssom_rule_free(NULL));

  PASS();
}

TEST test_cssom_decl(void) {
  cmp_cssom_decl_t decl;

  ASSERT_EQ(0, cmp_cssom_decl_init(&decl, "margin: 0; padding: 0;"));
  ASSERT_STR_EQ("margin: 0; padding: 0;", decl.css_text);
  ASSERT_EQ(0, decl.length);

  ASSERT_EQ(0, cmp_cssom_decl_free(&decl));
  ASSERT_EQ(NULL, decl.css_text);

  ASSERT_EQ(-1, cmp_cssom_decl_init(NULL, ""));
  ASSERT_EQ(-1, cmp_cssom_decl_free(NULL));

  PASS();
}

TEST test_cssom_computed_style(void) {
  cmp_cssom_computed_style_t cs;
  int dummy_node = 42;

  ASSERT_EQ(0, cmp_cssom_computed_style_init(&cs, &dummy_node));
  ASSERT_EQ(&dummy_node, cs.node_ref);
  ASSERT_EQ(NULL, cs.decl);

  ASSERT_EQ(0, cmp_cssom_computed_style_free(&cs));

  ASSERT_EQ(-1, cmp_cssom_computed_style_init(NULL, NULL));
  ASSERT_EQ(-1, cmp_cssom_computed_style_free(NULL));

  PASS();
}

TEST test_cssom_mutator(void) {
  cmp_cssom_mutator_t mutator;
  int dummy_stylesheet = 100;

  ASSERT_EQ(0, cmp_cssom_mutator_init(&mutator, &dummy_stylesheet));
  ASSERT_EQ(&dummy_stylesheet, mutator.stylesheet_ref);

  ASSERT_EQ(
      0, cmp_cssom_mutator_insert_rule(&mutator, ".cls { display: none; }", 0));
  ASSERT_EQ(0, cmp_cssom_mutator_delete_rule(&mutator, 0));

  ASSERT_EQ(-1, cmp_cssom_mutator_init(NULL, NULL));
  ASSERT_EQ(-1, cmp_cssom_mutator_insert_rule(NULL, "", 0));
  ASSERT_EQ(-1, cmp_cssom_mutator_delete_rule(NULL, 0));

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_cssom_rule);
  RUN_TEST(test_cssom_decl);
  RUN_TEST(test_cssom_computed_style);
  RUN_TEST(test_cssom_mutator);
  GREATEST_MAIN_END();
}
