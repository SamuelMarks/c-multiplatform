#include "m3/m3_color.h"
#include "test_utils.h"

static int cmp_color_near_u8(cmp_u8 a, cmp_u8 b, cmp_u8 tol) {
  int diff;

  diff = (int)a - (int)b;
  if (diff < 0) {
    diff = -diff;
  }

  return (diff <= (int)tol) ? 1 : 0;
}

int main(void) {
  M3ColorHct hct = {0};
  M3ColorHct hct2 = {0};
  M3TonalPalette palette;
  M3Scheme scheme_light;
  M3Scheme scheme_dark;
  cmp_u32 argb;
  cmp_u32 argb2;
  cmp_u8 r;
  cmp_u8 g;
  cmp_u8 b;
  cmp_u8 a;
  CMPScalar x;
  CMPScalar y;
  CMPScalar z;
  CMPScalar l;
  CMPScalar a_lab;
  CMPScalar b_lab;
  CMPBool dark;
  CMPBool in_gamut;
  int i;

  CMP_TEST_OK(m3_color_test_reset_failures());

  CMP_TEST_EXPECT(m3_color_test_argb_to_xyz(0, NULL, &y, &z),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_test_argb_to_xyz(0, &x, NULL, &z),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_test_argb_to_xyz(0, &x, &y, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_color_test_xyz_to_argb(0.0f, 0.0f, 0.0f, NULL, &in_gamut),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_test_xyz_to_argb(0.0f, 0.0f, 0.0f, &argb2, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(
      m3_color_test_xyz_to_lab(0.0f, 0.0f, 0.0f, NULL, &a_lab, &b_lab),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_test_xyz_to_lab(0.0f, 0.0f, 0.0f, &l, NULL, &b_lab),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_test_xyz_to_lab(0.0f, 0.0f, 0.0f, &l, &a_lab, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_color_test_lab_to_xyz(0.0f, 0.0f, 0.0f, NULL, &y, &z),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_test_lab_to_xyz(0.0f, 0.0f, 0.0f, &x, NULL, &z),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_test_lab_to_xyz(0.0f, 0.0f, 0.0f, &x, &y, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_color_test_lch_to_argb(0.0f, 0.0f, 0.0f, NULL, &in_gamut),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_test_lch_to_argb(0.0f, 0.0f, 0.0f, &argb2, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_color_test_argb_to_xyz(0xFF000000u, &x, &y, &z));
  CMP_TEST_OK(m3_color_test_argb_to_xyz(0xFFFFFFFFu, &x, &y, &z));

  CMP_TEST_OK(m3_color_test_xyz_to_argb(0.0f, 0.0f, 0.0f, &argb2, &in_gamut));
  CMP_TEST_ASSERT(in_gamut == CMP_TRUE);
  CMP_TEST_ASSERT(argb2 == 0xFF000000u);
  CMP_TEST_OK(m3_color_test_xyz_to_argb(2.0f, 0.0f, 0.0f, &argb2, &in_gamut));
  /* CMP_TEST_ASSERT(in_gamut == CMP_FALSE); */

  CMP_TEST_OK(m3_color_test_xyz_to_lab(0.0f, 0.0f, 0.0f, &l, &a_lab, &b_lab));
  CMP_TEST_OK(m3_color_test_xyz_to_lab(1.0f, 1.0f, 1.0f, &l, &a_lab, &b_lab));

  CMP_TEST_OK(m3_color_test_lab_to_xyz(0.0f, 0.0f, 0.0f, &x, &y, &z));
  CMP_TEST_OK(m3_color_test_lab_to_xyz(100.0f, 0.0f, 0.0f, &x, &y, &z));

  CMP_TEST_EXPECT(m3_color_test_set_lch_to_argb_fail_call(-1), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_color_test_set_tonal_palette_init_fail_call(-1),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_color_test_set_tonal_palette_tone_fail_call(-1),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_color_argb_from_rgba(0, 0, 0, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_rgba_from_argb(0, NULL, &g, &b, &a),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_rgba_from_argb(0, &r, NULL, &b, &a),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_rgba_from_argb(0, &r, &g, NULL, &a),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_color_rgba_from_argb(0, &r, &g, &b, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_color_argb_from_rgba(255, 0, 0, 255, &argb));
  CMP_TEST_ASSERT(argb == 0xFFFF0000u);
  CMP_TEST_OK(m3_color_rgba_from_argb(argb, &r, &g, &b, &a));
  CMP_TEST_ASSERT(r == 255 && g == 0 && b == 0 && a == 255);

  CMP_TEST_EXPECT(m3_hct_from_argb(0, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_hct_to_argb(NULL, &argb2), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_hct_to_argb(&hct, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_color_test_set_xyz_to_argb_fail(CMP_TRUE));
  CMP_TEST_EXPECT(
      m3_color_test_xyz_to_argb(0.0f, 0.0f, 0.0f, &argb2, &in_gamut),
      CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_color_test_set_xyz_to_argb_fail(CMP_FALSE));

  CMP_TEST_OK(m3_color_test_set_argb_to_xyz_fail(CMP_TRUE));
  CMP_TEST_EXPECT(m3_hct_from_argb(0xFF000000u, &hct), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_color_test_set_argb_to_xyz_fail(CMP_FALSE));

  CMP_TEST_OK(m3_color_test_set_xyz_to_lab_fail(CMP_TRUE));
  CMP_TEST_EXPECT(m3_hct_from_argb(0xFF000000u, &hct), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_color_test_set_xyz_to_lab_fail(CMP_FALSE));

  CMP_TEST_OK(m3_hct_from_argb(0xFF00FF00u, &hct));
  CMP_TEST_ASSERT(hct.tone >= 0.0f && hct.tone <= 100.0f);
  CMP_TEST_ASSERT(hct.chroma >= 0.0f);

  CMP_TEST_OK(m3_hct_from_argb(0xFFFF0000u, &hct));
  CMP_TEST_OK(m3_hct_to_argb(&hct, &argb2));
  CMP_TEST_OK(m3_color_rgba_from_argb(argb2, &r, &g, &b, &a));
  CMP_TEST_ASSERT(cmp_color_near_u8(r, 255, 2));
  CMP_TEST_ASSERT(cmp_color_near_u8(g, 0, 2));
  CMP_TEST_ASSERT(cmp_color_near_u8(b, 0, 2));

  hct.hue = 370.0f;
  hct.chroma = 10.0f;
  hct.tone = 50.0f;
  CMP_TEST_OK(m3_hct_to_argb(&hct, &argb2));

  hct.chroma = -1.0f;
  CMP_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), CMP_ERR_RANGE);
  hct.chroma = 10.0f;
  hct.tone = -1.0f;
  CMP_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), CMP_ERR_RANGE);
  hct.tone = 101.0f;
  CMP_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), CMP_ERR_RANGE);

  hct.tone = 0.0f;
  CMP_TEST_OK(m3_hct_to_argb(&hct, &argb2));
  CMP_TEST_ASSERT(argb2 == 0xFF000000u);
  hct.tone = 100.0f;
  CMP_TEST_OK(m3_hct_to_argb(&hct, &argb2));
  CMP_TEST_ASSERT(argb2 == 0xFFFFFFFFu);

  CMP_TEST_OK(m3_color_test_set_lab_to_xyz_fail(CMP_TRUE));
  CMP_TEST_EXPECT(
      m3_color_test_lch_to_argb(0.0f, 10.0f, 50.0f, &argb2, &in_gamut),
      CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_color_test_set_lab_to_xyz_fail(CMP_FALSE));

  hct.hue = 30.0f;
  hct.chroma = 200.0f;
  hct.tone = 50.0f;
  CMP_TEST_OK(m3_hct_to_argb(&hct, &argb2));
  CMP_TEST_OK(m3_hct_from_argb(argb2, &hct2));
  CMP_TEST_ASSERT(hct2.chroma <= hct.chroma + 0.01f);
  CMP_TEST_ASSERT(cmp_color_near_u8((cmp_u8)(hct2.tone + 0.5f),
                                    (cmp_u8)(hct.tone + 0.5f), 2));

  CMP_TEST_OK(m3_color_test_set_lch_to_argb_fail_call(1));
  CMP_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_color_test_set_lch_to_argb_fail_call(2));
  CMP_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_color_test_set_lch_to_argb_fail_call(3));
  CMP_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_color_test_set_lch_to_argb_fail_call(0));

  CMP_TEST_EXPECT(m3_tonal_palette_init(NULL, 0.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tonal_palette_init(&palette, 0.0f, -1.0f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tonal_palette_init(&palette, -10.0f, 20.0f));
  CMP_TEST_EXPECT(m3_tonal_palette_tone_argb(NULL, 50.0f, &argb2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tonal_palette_tone_argb(&palette, 50.0f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tonal_palette_tone_argb(&palette, -1.0f, &argb2),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tonal_palette_tone_argb(&palette, 101.0f, &argb2),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tonal_palette_tone_argb(&palette, 50.0f, &argb2));

  CMP_TEST_EXPECT(m3_scheme_generate(0xFF000000u, CMP_FALSE, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_color_test_set_argb_to_xyz_fail(CMP_TRUE));
  CMP_TEST_EXPECT(m3_scheme_generate(0xFF3366FFu, CMP_FALSE, &scheme_light),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_color_test_set_argb_to_xyz_fail(CMP_FALSE));

  for (i = 1; i <= 6; ++i) {
    CMP_TEST_OK(m3_color_test_set_tonal_palette_init_fail_call(i));
    CMP_TEST_EXPECT(m3_scheme_generate(0xFF3366FFu, CMP_FALSE, &scheme_light),
                    CMP_ERR_UNKNOWN);
  }
  CMP_TEST_OK(m3_color_test_set_tonal_palette_init_fail_call(0));

  for (i = 1; i <= 23; ++i) {
    CMP_TEST_OK(m3_color_test_set_tonal_palette_tone_fail_call(i));
    CMP_TEST_EXPECT(m3_scheme_generate(0xFF3366FFu, CMP_TRUE, &scheme_dark),
                    CMP_ERR_UNKNOWN);
  }
  for (i = 1; i <= 23; ++i) {
    CMP_TEST_OK(m3_color_test_set_tonal_palette_tone_fail_call(i));
    CMP_TEST_EXPECT(m3_scheme_generate(0xFF3366FFu, CMP_FALSE, &scheme_light),
                    CMP_ERR_UNKNOWN);
  }
  CMP_TEST_OK(m3_color_test_set_tonal_palette_tone_fail_call(0));

  dark = CMP_FALSE;
  CMP_TEST_OK(m3_scheme_generate(0xFF3366FFu, dark, &scheme_light));
  CMP_TEST_OK(m3_scheme_generate(0xFF808080u, dark, &scheme_light));
  dark = CMP_TRUE;
  CMP_TEST_OK(m3_scheme_generate(0xFF3366FFu, dark, &scheme_dark));
  CMP_TEST_ASSERT(scheme_light.primary != scheme_dark.primary);
  CMP_TEST_ASSERT(scheme_light.background != scheme_dark.background);
  CMP_TEST_ASSERT(scheme_light.error != scheme_dark.error);

  {
    cmp_u32 pixels[4] = {0xFF000000, 0xFFFFFFFF, 0xFFFF0000, 0xFF00FF00};
    cmp_u32 seed = 0;

    CMP_TEST_EXPECT(m3_color_extract_seed_from_image(NULL, 4, &seed),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_color_extract_seed_from_image(pixels, 4, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_color_extract_seed_from_image(pixels, 0, &seed),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(m3_color_extract_seed_from_image(pixels, 4, &seed));
    /* It should pick one of the saturated ones, probably Red because it's first
     * high-chroma hit */
    CMP_TEST_EXPECT(seed == 0xFFFF0000 ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);
  }

  return 0;
}
