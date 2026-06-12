/* clang-format off */
#include "greatest.h"
#include "cmp_css_svg.h"
/* clang-format on */

TEST test_svg_fill_init(void) {
  cmp_prop_svg_fill_t fill;
  int rc;

  rc = cmp_prop_svg_fill_init(&fill);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(CMP_PROP_COLOR_TRANSPARENT, fill.fill.type);
  ASSERT_EQ(CMP_SVG_FILL_RULE_NONZERO, fill.rule);
  ASSERT_EQ(1.0f, fill.opacity.value);

  rc = cmp_prop_svg_fill_init(NULL);
  ASSERT_EQ(-1, rc);

  PASS();
}

TEST test_svg_stroke_init(void) {
  cmp_prop_svg_stroke_t stroke;
  int rc;

  rc = cmp_prop_svg_stroke_init(&stroke);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(CMP_PROP_COLOR_TRANSPARENT, stroke.stroke.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, stroke.width.type);
  ASSERT_EQ(CMP_SVG_STROKE_LINECAP_BUTT, stroke.linecap);
  ASSERT_EQ(CMP_SVG_STROKE_LINEJOIN_MITER, stroke.linejoin);
  ASSERT_EQ(4.0f, stroke.miterlimit);
  ASSERT_EQ(0, stroke.dasharray_count);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, stroke.dashoffset.type);
  ASSERT_EQ(1.0f, stroke.opacity.value);

  rc = cmp_prop_svg_stroke_init(NULL);
  ASSERT_EQ(-1, rc);

  PASS();
}

TEST test_svg_geom_init(void) {
  cmp_prop_svg_geom_t geom;
  int rc;

  rc = cmp_prop_svg_geom_init(&geom);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, geom.cx.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, geom.cy.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, geom.r.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, geom.rx.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, geom.ry.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, geom.x.type);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, geom.y.type);

  rc = cmp_prop_svg_geom_init(NULL);
  ASSERT_EQ(-1, rc);

  PASS();
}

TEST test_svg_vector_effect_enums(void) {
  cmp_prop_svg_vector_effect_t effect =
      CMP_SVG_VECTOR_EFFECT_NON_SCALING_STROKE;
  ASSERT_EQ(CMP_SVG_VECTOR_EFFECT_NON_SCALING_STROKE, effect);
  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_svg_fill_init);
  RUN_TEST(test_svg_stroke_init);
  RUN_TEST(test_svg_geom_init);
  RUN_TEST(test_svg_vector_effect_enums);
  GREATEST_MAIN_END();
}
