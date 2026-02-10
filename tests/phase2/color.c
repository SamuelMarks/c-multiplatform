#include "m3/m3_color.h"
#include "test_utils.h"

static int m3_color_near_u8(m3_u8 a, m3_u8 b, m3_u8 tol) {
  int diff;

  diff = (int)a - (int)b;
  if (diff < 0) {
    diff = -diff;
  }

  return (diff <= (int)tol) ? 1 : 0;
}

int main(void) {
  M3ColorHct hct;
  M3ColorHct hct2;
  M3TonalPalette palette;
  M3Scheme scheme_light;
  M3Scheme scheme_dark;
  m3_u32 argb;
  m3_u32 argb2;
  m3_u8 r;
  m3_u8 g;
  m3_u8 b;
  m3_u8 a;
  M3Scalar x;
  M3Scalar y;
  M3Scalar z;
  M3Scalar l;
  M3Scalar a_lab;
  M3Scalar b_lab;
  M3Bool dark;
  M3Bool in_gamut;
  int i;

  M3_TEST_OK(m3_color_test_reset_failures());

  M3_TEST_EXPECT(m3_color_test_argb_to_xyz(0, NULL, &y, &z),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_test_argb_to_xyz(0, &x, NULL, &z),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_test_argb_to_xyz(0, &x, &y, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_color_test_xyz_to_argb(0.0f, 0.0f, 0.0f, NULL, &in_gamut),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_test_xyz_to_argb(0.0f, 0.0f, 0.0f, &argb2, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(
      m3_color_test_xyz_to_lab(0.0f, 0.0f, 0.0f, NULL, &a_lab, &b_lab),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_test_xyz_to_lab(0.0f, 0.0f, 0.0f, &l, NULL, &b_lab),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_test_xyz_to_lab(0.0f, 0.0f, 0.0f, &l, &a_lab, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_color_test_lab_to_xyz(0.0f, 0.0f, 0.0f, NULL, &y, &z),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_test_lab_to_xyz(0.0f, 0.0f, 0.0f, &x, NULL, &z),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_test_lab_to_xyz(0.0f, 0.0f, 0.0f, &x, &y, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_color_test_lch_to_argb(0.0f, 0.0f, 0.0f, NULL, &in_gamut),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_test_lch_to_argb(0.0f, 0.0f, 0.0f, &argb2, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_color_test_argb_to_xyz(0xFF000000u, &x, &y, &z));
  M3_TEST_OK(m3_color_test_argb_to_xyz(0xFFFFFFFFu, &x, &y, &z));

  M3_TEST_OK(m3_color_test_xyz_to_argb(0.0f, 0.0f, 0.0f, &argb2, &in_gamut));
  M3_TEST_ASSERT(in_gamut == M3_TRUE);
  M3_TEST_ASSERT(argb2 == 0xFF000000u);
  M3_TEST_OK(m3_color_test_xyz_to_argb(2.0f, 0.0f, 0.0f, &argb2, &in_gamut));
  M3_TEST_ASSERT(in_gamut == M3_FALSE);

  M3_TEST_OK(m3_color_test_xyz_to_lab(0.0f, 0.0f, 0.0f, &l, &a_lab, &b_lab));
  M3_TEST_OK(m3_color_test_xyz_to_lab(1.0f, 1.0f, 1.0f, &l, &a_lab, &b_lab));

  M3_TEST_OK(m3_color_test_lab_to_xyz(0.0f, 0.0f, 0.0f, &x, &y, &z));
  M3_TEST_OK(m3_color_test_lab_to_xyz(100.0f, 0.0f, 0.0f, &x, &y, &z));

  M3_TEST_EXPECT(m3_color_test_set_lch_to_argb_fail_call(-1), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_color_test_set_tonal_palette_init_fail_call(-1),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_color_test_set_tonal_palette_tone_fail_call(-1),
                 M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_color_argb_from_rgba(0, 0, 0, 0, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_rgba_from_argb(0, NULL, &g, &b, &a),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_rgba_from_argb(0, &r, NULL, &b, &a),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_rgba_from_argb(0, &r, &g, NULL, &a),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_color_rgba_from_argb(0, &r, &g, &b, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_color_argb_from_rgba(255, 0, 0, 255, &argb));
  M3_TEST_ASSERT(argb == 0xFFFF0000u);
  M3_TEST_OK(m3_color_rgba_from_argb(argb, &r, &g, &b, &a));
  M3_TEST_ASSERT(r == 255 && g == 0 && b == 0 && a == 255);

  M3_TEST_EXPECT(m3_hct_from_argb(0, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_hct_to_argb(NULL, &argb2), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_hct_to_argb(&hct, NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_color_test_set_xyz_to_argb_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_color_test_xyz_to_argb(0.0f, 0.0f, 0.0f, &argb2, &in_gamut),
                 M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_color_test_set_xyz_to_argb_fail(M3_FALSE));

  M3_TEST_OK(m3_color_test_set_argb_to_xyz_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_hct_from_argb(0xFF000000u, &hct), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_color_test_set_argb_to_xyz_fail(M3_FALSE));

  M3_TEST_OK(m3_color_test_set_xyz_to_lab_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_hct_from_argb(0xFF000000u, &hct), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_color_test_set_xyz_to_lab_fail(M3_FALSE));

  M3_TEST_OK(m3_hct_from_argb(0xFF00FF00u, &hct));
  M3_TEST_ASSERT(hct.tone >= 0.0f && hct.tone <= 100.0f);
  M3_TEST_ASSERT(hct.chroma >= 0.0f);

  M3_TEST_OK(m3_hct_from_argb(0xFFFF0000u, &hct));
  M3_TEST_OK(m3_hct_to_argb(&hct, &argb2));
  M3_TEST_OK(m3_color_rgba_from_argb(argb2, &r, &g, &b, &a));
  M3_TEST_ASSERT(m3_color_near_u8(r, 255, 2));
  M3_TEST_ASSERT(m3_color_near_u8(g, 0, 2));
  M3_TEST_ASSERT(m3_color_near_u8(b, 0, 2));

  hct.hue = 370.0f;
  hct.chroma = 10.0f;
  hct.tone = 50.0f;
  M3_TEST_OK(m3_hct_to_argb(&hct, &argb2));

  hct.chroma = -1.0f;
  M3_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), M3_ERR_RANGE);
  hct.chroma = 10.0f;
  hct.tone = -1.0f;
  M3_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), M3_ERR_RANGE);
  hct.tone = 101.0f;
  M3_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), M3_ERR_RANGE);

  hct.tone = 0.0f;
  M3_TEST_OK(m3_hct_to_argb(&hct, &argb2));
  M3_TEST_ASSERT(argb2 == 0xFF000000u);
  hct.tone = 100.0f;
  M3_TEST_OK(m3_hct_to_argb(&hct, &argb2));
  M3_TEST_ASSERT(argb2 == 0xFFFFFFFFu);

  M3_TEST_OK(m3_color_test_set_lab_to_xyz_fail(M3_TRUE));
  M3_TEST_EXPECT(
      m3_color_test_lch_to_argb(0.0f, 10.0f, 50.0f, &argb2, &in_gamut),
      M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_color_test_set_lab_to_xyz_fail(M3_FALSE));

  hct.hue = 30.0f;
  hct.chroma = 200.0f;
  hct.tone = 50.0f;
  M3_TEST_OK(m3_hct_to_argb(&hct, &argb2));
  M3_TEST_OK(m3_hct_from_argb(argb2, &hct2));
  M3_TEST_ASSERT(hct2.chroma <= hct.chroma + 0.01f);
  M3_TEST_ASSERT(
      m3_color_near_u8((m3_u8)(hct2.tone + 0.5f), (m3_u8)(hct.tone + 0.5f), 2));

  M3_TEST_OK(m3_color_test_set_lch_to_argb_fail_call(1));
  M3_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_color_test_set_lch_to_argb_fail_call(2));
  M3_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_color_test_set_lch_to_argb_fail_call(3));
  M3_TEST_EXPECT(m3_hct_to_argb(&hct, &argb2), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_color_test_set_lch_to_argb_fail_call(0));

  M3_TEST_EXPECT(m3_tonal_palette_init(NULL, 0.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tonal_palette_init(&palette, 0.0f, -1.0f), M3_ERR_RANGE);
  M3_TEST_OK(m3_tonal_palette_init(&palette, -10.0f, 20.0f));
  M3_TEST_EXPECT(m3_tonal_palette_tone_argb(NULL, 50.0f, &argb2),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tonal_palette_tone_argb(&palette, 50.0f, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tonal_palette_tone_argb(&palette, -1.0f, &argb2),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_tonal_palette_tone_argb(&palette, 101.0f, &argb2),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tonal_palette_tone_argb(&palette, 50.0f, &argb2));

  M3_TEST_EXPECT(m3_scheme_generate(0xFF000000u, M3_FALSE, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_color_test_set_argb_to_xyz_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_scheme_generate(0xFF3366FFu, M3_FALSE, &scheme_light),
                 M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_color_test_set_argb_to_xyz_fail(M3_FALSE));

  for (i = 1; i <= 6; ++i) {
    M3_TEST_OK(m3_color_test_set_tonal_palette_init_fail_call(i));
    M3_TEST_EXPECT(m3_scheme_generate(0xFF3366FFu, M3_FALSE, &scheme_light),
                   M3_ERR_UNKNOWN);
  }
  M3_TEST_OK(m3_color_test_set_tonal_palette_init_fail_call(0));

  for (i = 1; i <= 23; ++i) {
    M3_TEST_OK(m3_color_test_set_tonal_palette_tone_fail_call(i));
    M3_TEST_EXPECT(m3_scheme_generate(0xFF3366FFu, M3_TRUE, &scheme_dark),
                   M3_ERR_UNKNOWN);
  }
  for (i = 1; i <= 23; ++i) {
    M3_TEST_OK(m3_color_test_set_tonal_palette_tone_fail_call(i));
    M3_TEST_EXPECT(m3_scheme_generate(0xFF3366FFu, M3_FALSE, &scheme_light),
                   M3_ERR_UNKNOWN);
  }
  M3_TEST_OK(m3_color_test_set_tonal_palette_tone_fail_call(0));

  dark = M3_FALSE;
  M3_TEST_OK(m3_scheme_generate(0xFF3366FFu, dark, &scheme_light));
  M3_TEST_OK(m3_scheme_generate(0xFF808080u, dark, &scheme_light));
  dark = M3_TRUE;
  M3_TEST_OK(m3_scheme_generate(0xFF3366FFu, dark, &scheme_dark));
  M3_TEST_ASSERT(scheme_light.primary != scheme_dark.primary);
  M3_TEST_ASSERT(scheme_light.background != scheme_dark.background);
  M3_TEST_ASSERT(scheme_light.error != scheme_dark.error);

  return 0;
}
