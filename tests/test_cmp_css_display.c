/* clang-format off */
#include "cmp_css_display.h"
#include "greatest.h"
/* clang-format on */

TEST test_prop_float_advanced(void) {
  cmp_prop_float_advanced_t adv;

  ASSERT_EQ(-1, cmp_prop_float_advanced_init(NULL, 1, "page"));

  /* With reference */
  ASSERT_EQ(0, cmp_prop_float_advanced_init(&adv, 2, "column"));
  ASSERT_EQ(2, adv.defer);
  ASSERT_STR_EQ("column", adv.reference);

  ASSERT_EQ(0, cmp_prop_float_advanced_free(&adv));
  ASSERT_EQ(NULL, adv.reference);

  /* Without reference */
  ASSERT_EQ(0, cmp_prop_float_advanced_init(&adv, 1, NULL));
  ASSERT_EQ(1, adv.defer);
  ASSERT_EQ(NULL, adv.reference);
  ASSERT_EQ(0, cmp_prop_float_advanced_free(&adv));

  ASSERT_EQ(-1, cmp_prop_float_advanced_free(NULL));

  PASS();
}

TEST test_enums(void) {
  /* Just verify that the enums are declared and usable */
  cmp_prop_display_t d = CMP_DISPLAY_BLOCK;
  cmp_prop_float_t f = CMP_FLOAT_LEFT;
  cmp_prop_clear_t c = CMP_CLEAR_BOTH;
  cmp_prop_visibility_t v = CMP_VISIBILITY_HIDDEN;

  ASSERT_EQ(CMP_DISPLAY_BLOCK, d);
  ASSERT_EQ(CMP_FLOAT_LEFT, f);
  ASSERT_EQ(CMP_CLEAR_BOTH, c);
  ASSERT_EQ(CMP_VISIBILITY_HIDDEN, v);
  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_prop_float_advanced);
  RUN_TEST(test_enums);
  GREATEST_MAIN_END();
}