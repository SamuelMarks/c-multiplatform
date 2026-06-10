/* clang-format off */
#include "cmp_css_anchor_positioning.h"
#include "greatest.h"
/* clang-format on */

TEST test_prop_anchor_name(void) {
  cmp_prop_anchor_name_t anchor;

  ASSERT_EQ(-1, cmp_prop_anchor_name_init(NULL, "--my-anchor"));
  ASSERT_EQ(-1, cmp_prop_anchor_name_init(&anchor, NULL));

  ASSERT_EQ(0, cmp_prop_anchor_name_init(&anchor, "--my-anchor"));
  ASSERT_STR_EQ("--my-anchor", anchor.name);

  ASSERT_EQ(0, cmp_prop_anchor_name_free(&anchor));
  ASSERT_EQ(NULL, anchor.name);

  ASSERT_EQ(-1, cmp_prop_anchor_name_free(NULL));

  PASS();
}

TEST test_prop_position_anchor(void) {
  cmp_prop_position_anchor_t anchor;

  ASSERT_EQ(-1, cmp_prop_position_anchor_init(NULL, "--my-anchor"));
  ASSERT_EQ(-1, cmp_prop_position_anchor_init(&anchor, NULL));

  ASSERT_EQ(0, cmp_prop_position_anchor_init(&anchor, "--my-anchor"));
  ASSERT_STR_EQ("--my-anchor", anchor.name);

  ASSERT_EQ(0, cmp_prop_position_anchor_free(&anchor));
  ASSERT_EQ(NULL, anchor.name);

  PASS();
}

TEST test_anchor_funcs(void) {
  cmp_anchor_funcs_t func;

  ASSERT_EQ(-1, cmp_anchor_func_init(NULL, "--my-anchor", CMP_ANCHOR_SIDE_TOP));
  ASSERT_EQ(0, cmp_anchor_func_init(&func, "--my-anchor", CMP_ANCHOR_SIDE_TOP));
  ASSERT_EQ(0, func.is_size);
  ASSERT_STR_EQ("--my-anchor", func.func.anchor.anchor_name);
  ASSERT_EQ(CMP_ANCHOR_SIDE_TOP, func.func.anchor.side);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, func.func.anchor.fallback.type);

  ASSERT_EQ(0, cmp_anchor_funcs_free(&func));

  ASSERT_EQ(-1, cmp_anchor_size_func_init(NULL, "--my-anchor",
                                          CMP_ANCHOR_SIZE_WIDTH));
  ASSERT_EQ(0, cmp_anchor_size_func_init(&func, "--my-anchor",
                                         CMP_ANCHOR_SIZE_WIDTH));
  ASSERT_EQ(1, func.is_size);
  ASSERT_STR_EQ("--my-anchor", func.func.anchor_size.anchor_name);
  ASSERT_EQ(CMP_ANCHOR_SIZE_WIDTH, func.func.anchor_size.dimension);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, func.func.anchor_size.fallback.type);

  ASSERT_EQ(0, cmp_anchor_funcs_free(&func));

  /* Null name works */
  ASSERT_EQ(0, cmp_anchor_func_init(&func, NULL, CMP_ANCHOR_SIDE_LEFT));
  ASSERT_EQ(NULL, func.func.anchor.anchor_name);
  ASSERT_EQ(0, cmp_anchor_funcs_free(&func));

  PASS();
}

TEST test_prop_position_try(void) {
  cmp_prop_position_try_t prop;

  ASSERT_EQ(-1, cmp_prop_position_try_init(NULL, "flip-block",
                                           CMP_POSITION_TRY_ORDER_NORMAL));
  ASSERT_EQ(-1, cmp_prop_position_try_init(&prop, NULL,
                                           CMP_POSITION_TRY_ORDER_NORMAL));

  ASSERT_EQ(0, cmp_prop_position_try_init(&prop, "flip-block, --custom-try",
                                          CMP_POSITION_TRY_ORDER_MOST_HEIGHT));
  ASSERT_STR_EQ("flip-block, --custom-try", prop.options);
  ASSERT_EQ(CMP_POSITION_TRY_ORDER_MOST_HEIGHT, prop.order);

  ASSERT_EQ(0, cmp_prop_position_try_free(&prop));
  ASSERT_EQ(NULL, prop.options);

  PASS();
}

TEST test_enums(void) {
  cmp_prop_position_visibility_t v = CMP_POSITION_VISIBILITY_ANCHORS_VISIBLE;
  ASSERT_EQ(CMP_POSITION_VISIBILITY_ANCHORS_VISIBLE, v);
  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_prop_anchor_name);
  RUN_TEST(test_prop_position_anchor);
  RUN_TEST(test_anchor_funcs);
  RUN_TEST(test_prop_position_try);
  RUN_TEST(test_enums);
  GREATEST_MAIN_END();
}