/* clang-format off */
#include "cmp_css_cascade.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_specificity_compute(void) {
  cmp_specificity_t spec;

  ASSERT_EQ(-1, cmp_specificity_compute(NULL, &spec));
  ASSERT_EQ(-1, cmp_specificity_compute("div", NULL));

  ASSERT_EQ(0, cmp_specificity_compute("div", &spec));
  ASSERT_EQ(0, spec.a);
  ASSERT_EQ(0, spec.b);
  ASSERT_EQ(0, spec.c);

  PASS();
}

TEST test_specificity_compare(void) {
  cmp_specificity_t s1 = {0, 0, 0};
  cmp_specificity_t s2 = {0, 0, 0};
  int cmp;

  ASSERT_EQ(-1, cmp_specificity_compare(NULL, &s2, &cmp));

  ASSERT_EQ(0, cmp_specificity_compare(&s1, &s2, &cmp));
  ASSERT_EQ(0, cmp);

  s1.a = 1;
  ASSERT_EQ(0, cmp_specificity_compare(&s1, &s2, &cmp));
  ASSERT(cmp > 0);

  s1.a = 0;
  s2.a = 1;
  ASSERT_EQ(0, cmp_specificity_compare(&s1, &s2, &cmp));
  ASSERT(cmp < 0);

  s1.a = 0;
  s2.a = 0;
  s1.b = 1;
  s2.b = 0;
  ASSERT_EQ(0, cmp_specificity_compare(&s1, &s2, &cmp));
  ASSERT(cmp > 0);

  s1.b = 0;
  s2.b = 0;
  s1.c = 1;
  s2.c = 0;
  ASSERT_EQ(0, cmp_specificity_compare(&s1, &s2, &cmp));
  ASSERT(cmp > 0);

  PASS();
}

TEST test_layer(void) {
  cmp_at_rule_layer_t layer;

  ASSERT_EQ(-1, cmp_at_rule_layer_init(NULL, "test"));

  ASSERT_EQ(0, cmp_at_rule_layer_init(&layer, "test"));
  ASSERT_STR_EQ("test", layer.name);

  ASSERT_EQ(0, cmp_at_rule_layer_free(&layer));
  ASSERT_EQ(NULL, layer.name);

  ASSERT_EQ(0, cmp_at_rule_layer_init(&layer, NULL));
  ASSERT_EQ(NULL, layer.name);

  ASSERT_EQ(0, cmp_at_rule_layer_free(&layer));
  ASSERT_EQ(-1, cmp_at_rule_layer_free(NULL));

  PASS();
}

TEST test_css_var(void) {
  cmp_css_var_t var;

  ASSERT_EQ(-1, cmp_css_var_init(NULL, "--test", "red"));
  ASSERT_EQ(-1, cmp_css_var_init(&var, NULL, "red"));
  ASSERT_EQ(-1, cmp_css_var_init(&var, "--test", NULL));

  ASSERT_EQ(0, cmp_css_var_init(&var, "--test", "red"));
  ASSERT_STR_EQ("--test", var.name);
  ASSERT_STR_EQ("red", var.value);

  ASSERT_EQ(0, cmp_css_var_free(&var));
  ASSERT_EQ(-1, cmp_css_var_free(NULL));

  PASS();
}

TEST test_var_func(void) {
  cmp_var_func_t func;

  ASSERT_EQ(-1, cmp_var_func_init(NULL, "--test", "blue"));

  ASSERT_EQ(0, cmp_var_func_init(&func, "--test", "blue"));
  ASSERT_STR_EQ("--test", func.custom_property_name);
  ASSERT_STR_EQ("blue", func.fallback);

  ASSERT_EQ(0, cmp_var_func_free(&func));

  ASSERT_EQ(0, cmp_var_func_init(&func, "--test", NULL));
  ASSERT_EQ(NULL, func.fallback);

  ASSERT_EQ(0, cmp_var_func_free(&func));
  ASSERT_EQ(-1, cmp_var_func_free(NULL));

  PASS();
}

TEST test_property_rule(void) {
  cmp_at_rule_property_t prop;

  ASSERT_EQ(-1, cmp_at_rule_property_init(NULL, "--test", "<color>", 1, "red"));

  ASSERT_EQ(0, cmp_at_rule_property_init(&prop, "--test", "<color>", 1, "red"));
  ASSERT_STR_EQ("--test", prop.name);
  ASSERT_STR_EQ("<color>", prop.syntax);
  ASSERT_EQ(1, prop.inherits);
  ASSERT_STR_EQ("red", prop.initial_value);

  ASSERT_EQ(0, cmp_at_rule_property_free(&prop));
  ASSERT_EQ(-1, cmp_at_rule_property_free(NULL));

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_specificity_compute);
  RUN_TEST(test_specificity_compare);
  RUN_TEST(test_layer);
  RUN_TEST(test_css_var);
  RUN_TEST(test_var_func);
  RUN_TEST(test_property_rule);
  GREATEST_MAIN_END();
}