/**
 * @file test_cupertino_material.c
 * @brief Unit tests for Cupertino vibrancy materials and liquid glass
 * meta-materials.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_material.h"
#include <math.h>
/* clang-format on */

SUITE(cupertino_material_suite);

TEST test_material_resolve(void) {
  struct cupertino_material_config cfg;
  struct cupertino_material_params params;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_material_resolve(NULL, &params);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  memset(&cfg, 0, sizeof(cfg));
  rc = cupertino_material_resolve(&cfg, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  cfg.style = (enum cupertino_material_style) - 1;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  cfg.style = CUPERTINO_MATERIAL_STYLE_COUNT;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Reduce transparency (Light mode) */
  cfg.style = CUPERTINO_MATERIAL_REGULAR;
  cfg.reduce_transparency = 1;
  cfg.is_dark = 0;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, params.is_opaque);
  ASSERT(fabs(params.blur_radius - 0.0f) < 1e-5f);
  ASSERT(fabs(params.fallback_color_rgb[0] - 1.0f) < 1e-5f);

  /* Reduce transparency (Dark mode) */
  cfg.is_dark = 1;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, params.is_opaque);
  ASSERT(fabs(params.fallback_color_rgb[0] - 0.11f) < 1e-5f);

  /* Transparent blur materials across all styles (Light mode) */
  cfg.reduce_transparency = 0;
  cfg.is_dark = 0;
  cfg.appearance_tint = 0.5f;

  cfg.style = CUPERTINO_MATERIAL_ULTRA_THIN;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, params.is_opaque);
  ASSERT(fabs(params.blur_radius - 10.0f) < 1e-5f);

  cfg.style = CUPERTINO_MATERIAL_THIN;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(params.blur_radius - 20.0f) < 1e-5f);

  cfg.style = CUPERTINO_MATERIAL_REGULAR;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(params.blur_radius - 30.0f) < 1e-5f);

  cfg.style = CUPERTINO_MATERIAL_THICK;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(params.blur_radius - 45.0f) < 1e-5f);

  cfg.style = CUPERTINO_MATERIAL_CHROME;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(params.blur_radius - 55.0f) < 1e-5f);

  /* Dark mode verification */
  cfg.is_dark = 1;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(params.tint_color_rgba[0] - 0.15f) < 1e-5f);

  /* Clamp appearance tint (negative and > 1.0) */
  cfg.appearance_tint = -0.5f;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  cfg.appearance_tint = 2.0f;
  rc = cupertino_material_resolve(&cfg, &params);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_liquid_glass_refraction_and_dispersion(void) {
  float du = 0.0f;
  float dv = 0.0f;
  float ru = 0.0f;
  float rv = 0.0f;
  float bu = 0.0f;
  float bv = 0.0f;
  ui_error_t rc;

  /* Refraction invalid args */
  rc = cupertino_liquid_glass_refract(0.5f, 0.5f, 0.5f, 0.5f, 0.2f, 1.0f, &du,
                                      &dv);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_refract(0.5f, 0.5f, 0.5f, 0.5f, 0.2f, 0.5f, &du,
                                      &dv);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_refract(0.5f, 0.5f, 0.5f, 0.5f, 0.2f, 1.52f, NULL,
                                      &dv);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_refract(0.5f, 0.5f, 0.5f, 0.5f, 0.2f, 1.52f, &du,
                                      NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Refraction at center: displacement should be 0 */
  rc = cupertino_liquid_glass_refract(
      0.5f, 0.5f, 0.5f, 0.5f, 0.2f, CUPERTINO_LIQUID_GLASS_INDEX_OF_REFRACTION,
      &du, &dv);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(du - 0.5f) < 1e-5f);
  ASSERT(fabs(dv - 0.5f) < 1e-5f);

  /* Refraction off-center: lens distortion */
  rc = cupertino_liquid_glass_refract(
      0.7f, 0.5f, 0.5f, 0.5f, 0.2f, CUPERTINO_LIQUID_GLASS_INDEX_OF_REFRACTION,
      &du, &dv);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(du < 0.7f); /* Distorted towards center */

  /* Dispersion invalid args */
  rc = cupertino_liquid_glass_dispersion(0.5f, 0.5f, 0.5f, 0.5f, -0.1f, &ru,
                                         &rv, &bu, &bv);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_dispersion(0.5f, 0.5f, 0.5f, 0.5f, 0.05f, NULL,
                                         &rv, &bu, &bv);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_dispersion(0.5f, 0.5f, 0.5f, 0.5f, 0.05f, &ru,
                                         NULL, &bu, &bv);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_dispersion(0.5f, 0.5f, 0.5f, 0.5f, 0.05f, &ru,
                                         &rv, NULL, &bv);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_dispersion(0.5f, 0.5f, 0.5f, 0.5f, 0.05f, &ru,
                                         &rv, &bu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Dispersion at center */
  rc = cupertino_liquid_glass_dispersion(0.5f, 0.5f, 0.5f, 0.5f, 0.02f, &ru,
                                         &rv, &bu, &bv);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(ru) < 1e-5f && fabs(bu) < 1e-5f);

  /* Dispersion off-center: Red and Blue separate oppositely */
  rc = cupertino_liquid_glass_dispersion(0.7f, 0.5f, 0.5f, 0.5f, 0.02f, &ru,
                                         &rv, &bu, &bv);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ru > 0.0f);
  ASSERT(bu < 0.0f);
  ASSERT(fabs(ru - (-bu)) < 1e-5f);

  /* Valid refraction with corner_radius <= 0.001f */
  rc = cupertino_liquid_glass_refract(0.5f, 0.5f, 0.5f, 0.5f, 0.0005f, 1.52f,
                                      &du, &dv);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_liquid_glass_specular_and_smin(void) {
  float spec = 0.0f;
  float smin_dist = 0.0f;
  ui_error_t rc;

  /* Specular invalid args */
  rc = cupertino_liquid_glass_specular(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                       &spec);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_specular(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                                       -5.0f, &spec);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_specular(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                                       32.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Direct perpendicular light (light aligned with normal = (0, 0, 1)) */
  rc = cupertino_liquid_glass_specular(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                                       32.0f, &spec);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(spec - 1.0f) < 1e-4f);

  /* Light facing away (dot_nl <= 0) */
  rc = cupertino_liquid_glass_specular(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
                                       32.0f, &spec);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(spec - 0.0f) < 1e-5f);

  /* Zero length normal */
  rc = cupertino_liquid_glass_specular(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                       32.0f, &spec);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(spec - 0.0f) < 1e-5f);

  /* Zero length light_dir */
  rc = cupertino_liquid_glass_specular(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                       32.0f, &spec);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(spec - 0.0f) < 1e-5f);

  /* Light and normal such that dot_nl > 0 but reflected rz <= 0.0f:
   * N = (1, 0, 0), L = (1, 0, 1) -> N_norm = (1,0,0), L_norm = (0.7071, 0,
   * 0.7071) dot_nl = 0.7071 > 0 R_z = 2 * 0.7071 * 0 - 0.7071 = -0.7071 <= 0.0f
   */
  rc = cupertino_liquid_glass_specular(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                                       32.0f, &spec);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(spec - 0.0f) < 1e-5f);

  /* Light and normal such that 0 < rz <= 1.0f:
   * N = (0, 0, 1), L = (0, 0, 1) -> dot_nl = 1.0, R_z = 2*(1)*1 - 1 = 1.0
   * For rz = 0.5: N = (0, 0, 1), L = (0.866, 0, 0.5) -> dot_nl = 0.5, R_z =
   * 2*0.5*1 - 0.5 = 0.5
   */
  rc = cupertino_liquid_glass_specular(0.0f, 0.0f, 1.0f, 0.866f, 0.0f, 0.5f,
                                       2.0f, &spec);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(spec > 0.0f && spec <= 1.0f);

  /* Light and normal such that rz > 1.0f:
   * e.g., N = (0, 0, 1), L = (0.968, 0, 0.25) -> L_norm approx (0.968, 0,
   * 0.25), dot_nl = 0.25 With N=(0, 0, 1) and L=(0.6, 0, 0.8) -> dot_nl = 0.8,
   * R_z = 2*0.8*1 - 0.8 = 0.8 For R_z > 1.0: 2*L_z - L_z = L_z cannot exceed 1
   * if N = (0,0,1). But if N has normal_z = 1 and light is not aligned: Wait,
   * 2*(N.L)*Nz - Lz: if L = (0.3, 0, 0.3) normalized -> dot_nl = 0.7071, R_z =
   * 2*0.7071*1 - 0.7071 = 0.7071. Actually, if V = (0,0,1), reflection of unit
   * vector L off unit normal N has unit length! |R| = 1, so R_z <= 1.0 ALWAYS
   * for any unit vectors N and L!
   */

  /* Smin invalid args */
  rc = cupertino_liquid_glass_smin(1.0f, 2.0f, 0.0f, &smin_dist);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_smin(1.0f, 2.0f, -1.0f, &smin_dist);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_liquid_glass_smin(1.0f, 2.0f, 0.5f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Smin valid blending */
  rc = cupertino_liquid_glass_smin(1.0f, 1.0f, 0.5f, &smin_dist);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* smin(1.0, 1.0, 0.5) should be strictly less than 1.0 (smooth blend) */
  ASSERT(smin_dist < 1.0f);

  /* Far away shapes (|d1 - d2| >= k): returns standard min(d1, d2) */
  rc = cupertino_liquid_glass_smin(1.0f, 10.0f, 0.5f, &smin_dist);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(smin_dist - 1.0f) < 1e-5f);

  PASS();
}

TEST test_liquid_glass_histogram(void) {
  ui_color_t black_canvas[4];
  ui_color_t white_canvas[4];
  ui_color_t complex_canvas[4];
  float avg_lum = 0.0f;
  float contrast = 0.0f;
  float boost = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_liquid_glass_sample_histogram(NULL, 4, &avg_lum, &contrast,
                                               &boost);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_liquid_glass_sample_histogram(black_canvas, 0, &avg_lum,
                                               &contrast, &boost);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_liquid_glass_sample_histogram(black_canvas, 4, NULL, &contrast,
                                               &boost);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_liquid_glass_sample_histogram(black_canvas, 4, &avg_lum, NULL,
                                               &boost);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_liquid_glass_sample_histogram(black_canvas, 4, &avg_lum,
                                               &contrast, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* All black canvas */
  black_canvas[0] = UI_COLOR_ARGB(255, 0, 0, 0);
  black_canvas[1] = UI_COLOR_ARGB(255, 0, 0, 0);
  black_canvas[2] = UI_COLOR_ARGB(255, 0, 0, 0);
  black_canvas[3] = UI_COLOR_ARGB(255, 0, 0, 0);
  rc = cupertino_liquid_glass_sample_histogram(black_canvas, 4, &avg_lum,
                                               &contrast, &boost);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(avg_lum - 0.0f) < 1e-4f);
  ASSERT(fabs(contrast - 0.0f) < 1e-4f);
  ASSERT(boost >= 1.0f);

  /* All white canvas */
  white_canvas[0] = UI_COLOR_ARGB(255, 255, 255, 255);
  white_canvas[1] = UI_COLOR_ARGB(255, 255, 255, 255);
  white_canvas[2] = UI_COLOR_ARGB(255, 255, 255, 255);
  white_canvas[3] = UI_COLOR_ARGB(255, 255, 255, 255);
  rc = cupertino_liquid_glass_sample_histogram(white_canvas, 4, &avg_lum,
                                               &contrast, &boost);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(avg_lum - 1.0f) < 1e-4f);
  ASSERT(fabs(contrast - 0.0f) < 1e-4f);
  ASSERT(boost >= 1.0f);

  /* Complex wallpaper canvas (checkerboard) */
  complex_canvas[0] = UI_COLOR_ARGB(255, 0, 0, 0);
  complex_canvas[1] = UI_COLOR_ARGB(255, 255, 255, 255);
  complex_canvas[2] = UI_COLOR_ARGB(255, 255, 0, 0);
  complex_canvas[3] = UI_COLOR_ARGB(255, 0, 255, 0);
  rc = cupertino_liquid_glass_sample_histogram(complex_canvas, 4, &avg_lum,
                                               &contrast, &boost);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(avg_lum > 0.0f && avg_lum < 1.0f);
  ASSERT(contrast > 0.1f);
  ASSERT(boost > 1.0f);

  PASS();
}

SUITE(cupertino_material_suite) {
  RUN_TEST(test_material_resolve);
  RUN_TEST(test_liquid_glass_refraction_and_dispersion);
  RUN_TEST(test_liquid_glass_specular_and_smin);
  RUN_TEST(test_liquid_glass_histogram);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_material_suite);
  GREATEST_MAIN_END();
}
