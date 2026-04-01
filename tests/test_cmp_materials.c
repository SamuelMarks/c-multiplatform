/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_materials_blur(void) {
  cmp_materials_t *ctx = NULL;
  float rad, sat;

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_resolve_blur_effect(
                             ctx, CMP_BLUR_STYLE_ULTRA_THIN, &rad, &sat));
  ASSERT_EQ(5.0f, rad);
  ASSERT_EQ(1.0f, sat);

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_resolve_blur_effect(
                             ctx, CMP_BLUR_STYLE_PROMINENT, &rad, &sat));
  ASSERT_EQ(50.0f, rad);
  ASSERT_EQ(2.2f, sat);

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_destroy(ctx));
  PASS();
}

TEST test_materials_macos_maps(void) {
  cmp_materials_t *ctx = NULL;
  cmp_blur_style_t mapped;

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_materials_resolve_macos_material(
                ctx, CMP_MACOS_MATERIAL_WINDOW_BACKGROUND, &mapped));
  ASSERT_EQ(CMP_BLUR_STYLE_REGULAR, mapped);

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_resolve_macos_material(
                             ctx, CMP_MACOS_MATERIAL_BEHIND_WINDOW, &mapped));
  ASSERT_EQ(CMP_BLUR_STYLE_THICK, mapped);

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_destroy(ctx));
  PASS();
}

TEST test_materials_vibrancy(void) {
  cmp_materials_t *ctx = NULL;
  float op;
  int requires_cd;

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_resolve_vibrancy(
                             ctx, CMP_VIBRANCY_STYLE_LABEL, &op, &requires_cd));
  ASSERT_EQ(1.0f, op);
  ASSERT_EQ(1, requires_cd);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_materials_resolve_vibrancy(
                ctx, CMP_VIBRANCY_STYLE_TERTIARY_LABEL, &op, &requires_cd));
  ASSERT_EQ(0.3f, op);
  ASSERT_EQ(1, requires_cd);

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_resolve_vibrancy(
                             ctx, CMP_VIBRANCY_STYLE_FILL, &op, &requires_cd));
  ASSERT_EQ(0.2f, op);
  ASSERT_EQ(0, requires_cd);

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_destroy(ctx));
  PASS();
}

TEST test_materials_transitions(void) {
  cmp_materials_t *ctx = NULL;
  float rad;

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_create(&ctx));

  /* Ultra Thin (5.0) to Prominent (50.0) at 50% */
  ASSERT_EQ(CMP_SUCCESS, cmp_materials_interpolate_blur_transition(
                             ctx, CMP_BLUR_STYLE_ULTRA_THIN,
                             CMP_BLUR_STYLE_PROMINENT, 0.5f, &rad));
  ASSERT_EQ(27.5f, rad); /* 5.0 + 45.0 * 0.5 */

  /* Bounds check */
  ASSERT_EQ(CMP_SUCCESS, cmp_materials_interpolate_blur_transition(
                             ctx, CMP_BLUR_STYLE_ULTRA_THIN,
                             CMP_BLUR_STYLE_PROMINENT, 2.0f, &rad));
  ASSERT_EQ(50.0f, rad); /* clamped 1.0 */

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_interpolate_blur_transition(
                             ctx, CMP_BLUR_STYLE_ULTRA_THIN,
                             CMP_BLUR_STYLE_PROMINENT, -1.0f, &rad));
  ASSERT_EQ(5.0f, rad); /* clamped 0.0 */

  ASSERT_EQ(CMP_SUCCESS, cmp_materials_destroy(ctx));
  PASS();
}

TEST test_materials_null(void) {
  cmp_materials_t *ctx = NULL;
  float f;
  int i;
  cmp_blur_style_t bs;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_materials_create(NULL));
  cmp_materials_create(&ctx);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_materials_resolve_blur_effect(
                                       NULL, CMP_BLUR_STYLE_REGULAR, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_materials_resolve_blur_effect(
                                       ctx, CMP_BLUR_STYLE_REGULAR, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_materials_resolve_blur_effect(
                                       ctx, CMP_BLUR_STYLE_REGULAR, &f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_materials_resolve_macos_material(
                NULL, CMP_MACOS_MATERIAL_WINDOW_BACKGROUND, &bs));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_materials_resolve_macos_material(
                ctx, CMP_MACOS_MATERIAL_WINDOW_BACKGROUND, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_materials_resolve_vibrancy(
                                       NULL, CMP_VIBRANCY_STYLE_LABEL, &f, &i));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_materials_resolve_vibrancy(ctx, CMP_VIBRANCY_STYLE_LABEL, NULL, &i));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_materials_resolve_vibrancy(ctx, CMP_VIBRANCY_STYLE_LABEL, &f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_materials_interpolate_blur_transition(
                NULL, CMP_BLUR_STYLE_THIN, CMP_BLUR_STYLE_THICK, 0.5f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_materials_interpolate_blur_transition(
                ctx, CMP_BLUR_STYLE_THIN, CMP_BLUR_STYLE_THICK, 0.5f, NULL));

  cmp_materials_destroy(ctx);
  PASS();
}

SUITE(materials_suite) {
  RUN_TEST(test_materials_blur);
  RUN_TEST(test_materials_macos_maps);
  RUN_TEST(test_materials_vibrancy);
  RUN_TEST(test_materials_transitions);
  RUN_TEST(test_materials_null);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(materials_suite);
  GREATEST_MAIN_END();
}
