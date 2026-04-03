/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_color.h"
#include <math.h>
/* clang-format on */

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* Utility mathematical functions for color conversion */
static float linearize(float channel) {
  if (channel <= 0.04045f) {
    return channel / 12.92f;
  } else {
    return (float)pow((channel + 0.055f) / 1.055f, 2.4f);
  }
}

static float delinearize(float channel) {
  if (channel <= 0.0031308f) {
    return channel * 12.92f;
  } else {
    return 1.055f * (float)pow(channel, 1.0f / 2.4f) - 0.055f;
  }
}

static void srgb_to_xyz(float r, float g, float b, float *x, float *y,
                        float *z) {
  float rl = linearize(r);
  float gl = linearize(g);
  float bl = linearize(b);

  *x = rl * 0.4124f + gl * 0.3576f + bl * 0.1805f;
  *y = rl * 0.2126f + gl * 0.7152f + bl * 0.0722f;
  *z = rl * 0.0193f + gl * 0.1192f + bl * 0.9505f;
}

static void xyz_to_srgb(float x, float y, float z, float *r, float *g,
                        float *b) {
  float rl = x * 3.2406f + y * -1.5372f + z * -0.4986f;
  float gl = x * -0.9689f + y * 1.8758f + z * 0.0415f;
  float bl = x * 0.0557f + y * -0.2040f + z * 1.0570f;

  *r = delinearize(rl);
  *g = delinearize(gl);
  *b = delinearize(bl);
}

/* L*a*b* D65 white point */
#define WHITE_X 0.95047f
#define WHITE_Y 1.00000f
#define WHITE_Z 1.08883f

static float lab_f(float t) {
  if (t > (216.0f / 24389.0f)) {
    return (float)pow(t, 1.0f / 3.0f);
  } else {
    return (841.0f / 108.0f) * t + (4.0f / 29.0f);
  }
}

static float lab_inv_f(float t) {
  if (t > (6.0f / 29.0f)) {
    return t * t * t;
  } else {
    return (108.0f / 841.0f) * (t - (4.0f / 29.0f));
  }
}

static void xyz_to_lab(float x, float y, float z, float *l, float *a,
                       float *b) {
  float fx = lab_f(x / WHITE_X);
  float fy = lab_f(y / WHITE_Y);
  float fz = lab_f(z / WHITE_Z);

  *l = (116.0f * fy) - 16.0f;
  *a = 500.0f * (fx - fy);
  *b = 200.0f * (fy - fz);
}

static void lab_to_xyz(float l, float a, float b, float *x, float *y,
                       float *z) {
  float fy = (l + 16.0f) / 116.0f;
  float fx = (a / 500.0f) + fy;
  float fz = fy - (b / 200.0f);

  *x = WHITE_X * lab_inv_f(fx);
  *y = WHITE_Y * lab_inv_f(fy);
  *z = WHITE_Z * lab_inv_f(fz);
}

/* Actual HCT Approximation via CAM16 equivalent mapping to LCH */
int cmp_m3_srgb_to_hct(const cmp_color_t *in_color, float *out_hue,
                       float *out_chroma, float *out_tone) {
  float x, y, z, l, a, b, chroma, hue;
  if (!in_color || !out_hue || !out_chroma || !out_tone)
    return CMP_ERROR_INVALID_ARG;

  srgb_to_xyz(in_color->r, in_color->g, in_color->b, &x, &y, &z);
  xyz_to_lab(x, y, z, &l, &a, &b);

  chroma = (float)sqrt(a * a + b * b);
  hue = (float)(atan2(b, a) * 180.0f / M_PI);
  if (hue < 0.0f)
    hue += 360.0f;

  *out_hue = hue;
  *out_chroma = chroma;
  *out_tone = l; /* Tone is rigorously defined as L* in HCT */

  return CMP_SUCCESS;
}

int cmp_m3_hct_to_srgb(float hue, float chroma, float tone,
                       cmp_color_t *out_color) {
  float l, a, b, x, y, z, r, g, bl;
  if (!out_color)
    return CMP_ERROR_INVALID_ARG;

  l = tone;
  a = chroma * (float)cos(hue * M_PI / 180.0f);
  b = chroma * (float)sin(hue * M_PI / 180.0f);

  lab_to_xyz(l, a, b, &x, &y, &z);
  xyz_to_srgb(x, y, z, &r, &g, &bl);

  /* Clamp gamut */
  if (r < 0.0f)
    r = 0.0f;
  if (r > 1.0f)
    r = 1.0f;
  if (g < 0.0f)
    g = 0.0f;
  if (g > 1.0f)
    g = 1.0f;
  if (bl < 0.0f)
    bl = 0.0f;
  if (bl > 1.0f)
    bl = 1.0f;

  out_color->r = r;
  out_color->g = g;
  out_color->b = bl;
  out_color->a = 1.0f;
  out_color->space = CMP_COLOR_SPACE_SRGB;

  return CMP_SUCCESS;
}

int cmp_m3_p3_to_hct(const cmp_color_t *in_color, float *out_hue,
                     float *out_chroma, float *out_tone) {
  /* P3 -> XYZ -> LAB -> HCT (approx. sRGB for now) */
  return cmp_m3_srgb_to_hct(in_color, out_hue, out_chroma, out_tone);
}

int cmp_m3_linear_to_hct(const cmp_color_t *in_color, float *out_hue,
                         float *out_chroma, float *out_tone) {
  float x, y, z, l, a, b, chroma, hue;
  if (!in_color || !out_hue || !out_chroma || !out_tone)
    return CMP_ERROR_INVALID_ARG;

  x = in_color->r * 0.4124f + in_color->g * 0.3576f + in_color->b * 0.1805f;
  y = in_color->r * 0.2126f + in_color->g * 0.7152f + in_color->b * 0.0722f;
  z = in_color->r * 0.0193f + in_color->g * 0.1192f + in_color->b * 0.9505f;

  xyz_to_lab(x, y, z, &l, &a, &b);

  chroma = (float)sqrt(a * a + b * b);
  hue = (float)(atan2(b, a) * 180.0f / M_PI);
  if (hue < 0.0f)
    hue += 360.0f;

  *out_hue = hue;
  *out_chroma = chroma;
  *out_tone = l;

  return CMP_SUCCESS;
}

int cmp_m3_generate_tonal_palette(float hue, float chroma, float tone,
                                  cmp_palette_t *out_palette) {
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone; /* We ignore original tone to generate a full spectrum */

  /* Material 3 specifically defines these specific tones */
  cmp_m3_hct_to_srgb(hue, chroma, 40.0f, &out_palette->primary);
  cmp_m3_hct_to_srgb(hue, chroma, 100.0f, &out_palette->on_primary);

  return CMP_SUCCESS;
}

int cmp_m3_scheme_tonal_spot(float hue, float chroma, float tone, int is_dark,
                             cmp_palette_t *out_palette) {
  float p_chroma = 36.0f;
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone;
  (void)chroma;

  /* Tonal spot: Chroma 36 */
  cmp_m3_hct_to_srgb(hue, p_chroma, is_dark ? 80.0f : 40.0f,
                     &out_palette->primary);
  cmp_m3_hct_to_srgb(hue, p_chroma, is_dark ? 20.0f : 100.0f,
                     &out_palette->on_primary);

  return CMP_SUCCESS;
}

int cmp_m3_scheme_spritz(float hue, float chroma, float tone, int is_dark,
                         cmp_palette_t *out_palette) {
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone;
  (void)chroma;

  /* Spritz: Chroma 12 */
  cmp_m3_hct_to_srgb(hue, 12.0f, is_dark ? 80.0f : 40.0f,
                     &out_palette->primary);
  return CMP_SUCCESS;
}

int cmp_m3_scheme_vibrant(float hue, float chroma, float tone, int is_dark,
                          cmp_palette_t *out_palette) {
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone;
  (void)chroma;

  /* Vibrant: Chroma 130 (max) */
  cmp_m3_hct_to_srgb(hue, 130.0f, is_dark ? 80.0f : 40.0f,
                     &out_palette->primary);
  return CMP_SUCCESS;
}

int cmp_m3_scheme_expressive(float hue, float chroma, float tone, int is_dark,
                             cmp_palette_t *out_palette) {
  float shifted_hue;
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone;
  (void)chroma;

  /* Expressive: Hue + 120 */
  shifted_hue = fmodf(hue + 120.0f, 360.0f);
  cmp_m3_hct_to_srgb(shifted_hue, 40.0f, is_dark ? 80.0f : 40.0f,
                     &out_palette->primary);
  return CMP_SUCCESS;
}

int cmp_m3_scheme_rainbow(float hue, float chroma, float tone, int is_dark,
                          cmp_palette_t *out_palette) {
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone;
  (void)chroma;

  cmp_m3_hct_to_srgb(hue, 48.0f, is_dark ? 80.0f : 40.0f,
                     &out_palette->primary);
  return CMP_SUCCESS;
}

int cmp_m3_scheme_fruit_salad(float hue, float chroma, float tone, int is_dark,
                              cmp_palette_t *out_palette) {
  float shifted_hue;
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone;
  (void)chroma;

  shifted_hue = fmodf(hue - 50.0f + 360.0f, 360.0f);
  cmp_m3_hct_to_srgb(shifted_hue, 48.0f, is_dark ? 80.0f : 40.0f,
                     &out_palette->primary);
  return CMP_SUCCESS;
}

int cmp_m3_scheme_monochrome(float hue, float chroma, float tone, int is_dark,
                             cmp_palette_t *out_palette) {
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone;
  (void)chroma;

  /* Monochrome: 0 Chroma */
  cmp_m3_hct_to_srgb(hue, 0.0f, is_dark ? 80.0f : 40.0f, &out_palette->primary);
  return CMP_SUCCESS;
}

int cmp_m3_scheme_fidelity(float hue, float chroma, float tone, int is_dark,
                           cmp_palette_t *out_palette) {
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone;

  /* Fidelity: Exact match */
  cmp_m3_hct_to_srgb(hue, chroma, is_dark ? 80.0f : 40.0f,
                     &out_palette->primary);
  return CMP_SUCCESS;
}

int cmp_m3_scheme_content(float hue, float chroma, float tone, int is_dark,
                          cmp_palette_t *out_palette) {
  if (!out_palette)
    return CMP_ERROR_INVALID_ARG;
  (void)tone;

  /* Content: Uses extracted */
  cmp_m3_hct_to_srgb(hue, chroma, is_dark ? 80.0f : 40.0f,
                     &out_palette->primary);
  return CMP_SUCCESS;
}