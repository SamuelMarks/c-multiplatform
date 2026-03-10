/* clang-format off */
#include "m3/m3_color.h"
#include "test_utils.h"
#include <string.h>

static int cmp_color_near_u8(cmp_u8 a, cmp_u8 b, cmp_u8 tol) {
  int diff;

  diff = (int)a - (int)b;
  if (diff < 0) {
    diff = -diff;
  }

  return (diff <= (int)tol) ? 1 : 0;
}

#include "cmpc/cmp_api_ws.h"
/* clang-format on */
int CMP_CALL mock_get_system_color(void *ctx, cmp_u32 color_type,
                                   CMPScalar *out_r, CMPScalar *out_g,
                                   CMPScalar *out_b, CMPScalar *out_a) {
  if (ctx != NULL)
    return CMP_ERR_UNKNOWN;
  if (color_type == CMP_SYSTEM_COLOR_ACCENT) {
    *out_r = 1.5f;  /* Test clamping > 1 */
    *out_g = -0.5f; /* Test clamping < 0 */
    *out_b = 0.5f;
    *out_a = 1.5f; /* Test clamping > 1 */
    return CMP_OK;
  }
  return CMP_ERR_NOT_FOUND;
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

  /* Test M3SchemeVariant values */
  CMP_TEST_EXPECT(m3_scheme_generate_variant(0xFF3366FFu, CMP_FALSE,
                                             M3_SCHEME_VARIANT_TONAL_SPOT,
                                             NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_scheme_generate_variant(
      0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_EXPRESSIVE, &scheme_light));
  CMP_TEST_OK(m3_scheme_generate_variant(
      0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_FIDELITY, &scheme_light));
  CMP_TEST_OK(m3_scheme_generate_variant(
      0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_CONTENT, &scheme_light));
  CMP_TEST_OK(m3_scheme_generate_variant(
      0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_VIBRANT, &scheme_light));
  CMP_TEST_OK(m3_scheme_generate_variant(
      0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_NEUTRAL, &scheme_light));

  {
    cmp_u32 pixels[4] = {0xFF000000, 0xFFFFFFFF, 0xFFFF0000, 0xFF00FF00};
    cmp_u32 seed = 0;

    CMP_TEST_EXPECT(m3_scheme_generate_with_contrast(
                        0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_TONAL_SPOT,
                        M3_CONTRAST_STANDARD, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    {
      M3Scheme test_scheme;
      m3_color_test_set_tonal_palette_init_fail_call(1);
      CMP_TEST_EXPECT(m3_scheme_generate_with_contrast(
                          0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_TONAL_SPOT,
                          M3_CONTRAST_STANDARD, &test_scheme),
                      CMP_ERR_UNKNOWN);
      m3_color_test_reset_failures();
    }
    CMP_TEST_OK(m3_scheme_generate_with_contrast(
        0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_TONAL_SPOT,
        M3_CONTRAST_STANDARD, &scheme_light));
    CMP_TEST_OK(m3_scheme_generate_with_contrast(
        0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_TONAL_SPOT,
        M3_CONTRAST_MEDIUM, &scheme_light));
    CMP_TEST_OK(m3_scheme_generate_with_contrast(
        0xFF3366FFu, CMP_FALSE, M3_SCHEME_VARIANT_TONAL_SPOT, M3_CONTRAST_HIGH,
        &scheme_light));
    CMP_TEST_OK(m3_scheme_generate_with_contrast(
        0xFF3366FFu, CMP_TRUE, M3_SCHEME_VARIANT_TONAL_SPOT,
        M3_CONTRAST_STANDARD, &scheme_dark));
    CMP_TEST_OK(m3_scheme_generate_with_contrast(
        0xFF3366FFu, CMP_TRUE, M3_SCHEME_VARIANT_TONAL_SPOT, M3_CONTRAST_MEDIUM,
        &scheme_dark));
    CMP_TEST_OK(m3_scheme_generate_with_contrast(
        0xFF3366FFu, CMP_TRUE, M3_SCHEME_VARIANT_TONAL_SPOT, M3_CONTRAST_HIGH,
        &scheme_dark));
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

  {
    CMPScalar opacity;
    cmp_u32 blended;

    CMP_TEST_EXPECT(m3_color_surface_tint_opacity(0, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_color_surface_tint_opacity(6, &opacity), CMP_ERR_RANGE);

    CMP_TEST_OK(m3_color_surface_tint_opacity(0, &opacity));
    CMP_TEST_ASSERT(opacity == 0.0f);
    CMP_TEST_OK(m3_color_surface_tint_opacity(1, &opacity));
    CMP_TEST_ASSERT(opacity == 0.05f);
    CMP_TEST_OK(m3_color_surface_tint_opacity(2, &opacity));
    CMP_TEST_ASSERT(opacity == 0.08f);
    CMP_TEST_OK(m3_color_surface_tint_opacity(3, &opacity));
    CMP_TEST_ASSERT(opacity == 0.11f);
    CMP_TEST_OK(m3_color_surface_tint_opacity(4, &opacity));
    CMP_TEST_ASSERT(opacity == 0.12f);
    CMP_TEST_OK(m3_color_surface_tint_opacity(5, &opacity));
    CMP_TEST_ASSERT(opacity == 0.14f);

    CMP_TEST_EXPECT(m3_color_blend_surface_tint(0, 0, 0.0f, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_color_blend_surface_tint(0, 0, -0.1f, &blended),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(m3_color_blend_surface_tint(0, 0, 1.1f, &blended),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(
        m3_color_blend_surface_tint(0xFF000000u, 0xFFFFFFFFu, 0.0f, &blended));
    CMP_TEST_ASSERT(blended == 0xFF000000u);
    CMP_TEST_OK(
        m3_color_blend_surface_tint(0xFF000000u, 0xFFFFFFFFu, 1.0f, &blended));
    CMP_TEST_ASSERT(blended == 0xFFFFFFFFu);
    CMP_TEST_OK(
        m3_color_blend_surface_tint(0xFF000000u, 0xFFFFFFFFu, 0.5f, &blended));
    /* Halfway between 0 and 255 is 127 */
    CMP_TEST_ASSERT(blended == 0xFF7F7F7Fu);
  }

  {
    M3Scheme scheme;
    CMPWSVTable vtable;
    CMPWS ws;

    extern int CMP_CALL mock_get_system_color(
        void *ctx, cmp_u32 color_type, CMPScalar *out_r, CMPScalar *out_g,
        CMPScalar *out_b, CMPScalar *out_a);

    memset(&vtable, 0, sizeof(vtable));
    memset(&ws, 0, sizeof(ws));
    vtable.get_system_color = mock_get_system_color;
    ws.vtable = &vtable;

    CMP_TEST_EXPECT(m3_scheme_generate_system(NULL, CMP_FALSE, &scheme),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_scheme_generate_system(&ws, CMP_FALSE, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    /* Fail */
    ws.ctx = (void *)1;
    CMP_TEST_EXPECT(m3_scheme_generate_system(&ws, CMP_FALSE, &scheme),
                    CMP_ERR_UNKNOWN);

    /* Success */
    ws.ctx = NULL;
    CMP_TEST_OK(m3_scheme_generate_system(&ws, CMP_FALSE, &scheme));
  }
  return 0;
}
