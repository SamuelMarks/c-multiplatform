/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_flex_align_start(void) {
  float pos, size;
  ASSERT_EQ(CMP_SUCCESS, cmp_flex_align_evaluate(CMP_FLEX_ALIGN_START, 100.0,
                                                 20.0, 0.0, 0.0, &pos, &size));
  ASSERT_EQ(0.0, pos);
  ASSERT_EQ(20.0, size);
  PASS();
}

TEST test_flex_align_end(void) {
  float pos, size;
  ASSERT_EQ(CMP_SUCCESS, cmp_flex_align_evaluate(CMP_FLEX_ALIGN_END, 100.0,
                                                 20.0, 0.0, 0.0, &pos, &size));
  ASSERT_EQ(80.0, pos);
  ASSERT_EQ(20.0, size);
  PASS();
}

TEST test_flex_align_center(void) {
  float pos, size;
  ASSERT_EQ(CMP_SUCCESS, cmp_flex_align_evaluate(CMP_FLEX_ALIGN_CENTER, 100.0,
                                                 20.0, 0.0, 0.0, &pos, &size));
  ASSERT_EQ(40.0, pos);
  ASSERT_EQ(20.0, size);
  PASS();
}

TEST test_flex_align_stretch(void) {
  float pos, size;
  ASSERT_EQ(CMP_SUCCESS, cmp_flex_align_evaluate(CMP_FLEX_ALIGN_STRETCH, 100.0,
                                                 20.0, 0.0, 0.0, &pos, &size));
  ASSERT_EQ(0.0, pos);
  ASSERT_EQ(100.0, size);
  PASS();
}

TEST test_flex_align_baseline(void) {
  float pos, size;
  ASSERT_EQ(CMP_SUCCESS, cmp_flex_align_evaluate(CMP_FLEX_ALIGN_BASELINE, 100.0,
                                                 20.0, 5.0, 15.0, &pos, &size));
  ASSERT_EQ(10.0, pos); /* max_baseline - item_baseline */
  ASSERT_EQ(20.0, size);
  PASS();
}

TEST test_flex_align_invalid(void) {
  float pos, size;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_flex_align_evaluate(CMP_FLEX_ALIGN_START, 100.0, 20.0, 0.0, 0.0,
                                    NULL, &size));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_flex_align_evaluate(CMP_FLEX_ALIGN_START, 100.0, 20.0, 0.0, 0.0,
                                    &pos, NULL));
  PASS();
}

SUITE(cmp_flex_align_suite) {
  RUN_TEST(test_flex_align_start);
  RUN_TEST(test_flex_align_end);
  RUN_TEST(test_flex_align_center);
  RUN_TEST(test_flex_align_stretch);
  RUN_TEST(test_flex_align_baseline);
  RUN_TEST(test_flex_align_invalid);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_flex_align_suite);
  GREATEST_MAIN_END();
}
