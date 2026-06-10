/* clang-format off */
#include "cmp_css_box_model.h"
#include "greatest.h"
/* clang-format on */

TEST test_prop_size(void) {
  cmp_prop_size_t size;

  ASSERT_EQ(-1, cmp_prop_size_init_auto(NULL));
  ASSERT_EQ(0, cmp_prop_size_init_auto(&size));
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, size.type);

  ASSERT_EQ(-1, cmp_prop_size_init_phys(NULL, 10.0f, CMP_LENGTH_PX));
  ASSERT_EQ(0, cmp_prop_size_init_phys(&size, 10.0f, CMP_LENGTH_PX));
  ASSERT_EQ(CMP_PROP_SIZE_LENGTH_PHYS, size.type);
  ASSERT_EQ(10.0f, size.value.phys.value);
  ASSERT_EQ(CMP_LENGTH_PX, size.value.phys.unit);

  ASSERT_EQ(-1, cmp_prop_size_init_percent(NULL, 50.0f));
  ASSERT_EQ(0, cmp_prop_size_init_percent(&size, 50.0f));
  ASSERT_EQ(CMP_PROP_SIZE_PERCENT, size.type);
  ASSERT_EQ(50.0f, size.value.percent.value);

  ASSERT_EQ(-1,
            cmp_prop_size_init_intrinsic(NULL, CMP_INTRINSIC_SIZE_MAX_CONTENT));
  ASSERT_EQ(
      0, cmp_prop_size_init_intrinsic(&size, CMP_INTRINSIC_SIZE_MAX_CONTENT));
  ASSERT_EQ(CMP_PROP_SIZE_INTRINSIC, size.type);
  ASSERT_EQ(CMP_INTRINSIC_SIZE_MAX_CONTENT, size.value.intrinsic);

  PASS();
}

TEST test_prop_margin(void) {
  cmp_prop_margin_t margin;

  ASSERT_EQ(-1, cmp_prop_margin_init_auto(NULL));
  ASSERT_EQ(0, cmp_prop_margin_init_auto(&margin));
  ASSERT_EQ(CMP_PROP_MARGIN_AUTO, margin.type);

  ASSERT_EQ(-1, cmp_prop_margin_init_phys(NULL, 15.0f, CMP_LENGTH_PT));
  ASSERT_EQ(0, cmp_prop_margin_init_phys(&margin, 15.0f, CMP_LENGTH_PT));
  ASSERT_EQ(CMP_PROP_MARGIN_LENGTH_PHYS, margin.type);
  ASSERT_EQ(15.0f, margin.value.phys.value);
  ASSERT_EQ(CMP_LENGTH_PT, margin.value.phys.unit);

  PASS();
}

TEST test_prop_padding(void) {
  cmp_prop_padding_t padding;

  ASSERT_EQ(-1, cmp_prop_padding_init_phys(NULL, 20.0f, CMP_LENGTH_CM));
  ASSERT_EQ(0, cmp_prop_padding_init_phys(&padding, 20.0f, CMP_LENGTH_CM));
  ASSERT_EQ(CMP_PROP_PADDING_LENGTH_PHYS, padding.type);
  ASSERT_EQ(20.0f, padding.value.phys.value);
  ASSERT_EQ(CMP_LENGTH_CM, padding.value.phys.unit);

  PASS();
}

TEST test_prop_aspect_ratio(void) {
  cmp_prop_aspect_ratio_t ar;

  ASSERT_EQ(-1, cmp_prop_aspect_ratio_init_auto(NULL));
  ASSERT_EQ(0, cmp_prop_aspect_ratio_init_auto(&ar));
  ASSERT_EQ(CMP_ASPECT_RATIO_AUTO, ar.type);

  ASSERT_EQ(-1, cmp_prop_aspect_ratio_init_ratio(NULL, 16.0f, 9.0f));
  ASSERT_EQ(
      -1, cmp_prop_aspect_ratio_init_ratio(&ar, 16.0f, 0.0f)); /* denom zero */
  ASSERT_EQ(0, cmp_prop_aspect_ratio_init_ratio(&ar, 16.0f, 9.0f));
  ASSERT_EQ(CMP_ASPECT_RATIO_RATIO, ar.type);
  ASSERT_EQ(16.0f, ar.ratio.numerator);
  ASSERT_EQ(9.0f, ar.ratio.denominator);

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_prop_size);
  RUN_TEST(test_prop_margin);
  RUN_TEST(test_prop_padding);
  RUN_TEST(test_prop_aspect_ratio);
  GREATEST_MAIN_END();
}