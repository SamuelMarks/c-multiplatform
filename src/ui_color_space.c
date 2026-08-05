/* clang-format off */
#include "ui_color_space.h"
#include <math.h>
/* clang-format on */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Standard sRGB to XYZ matrix */
static ui_error_t srgb_to_xyz(float r, float g, float b, float *x, float *y,
                              float *z) {
  /* Linearize */
  r = (r <= 0.04045f) ? (r / 12.92f) : (float)pow((r + 0.055) / 1.055, 2.4);
  g = (g <= 0.04045f) ? (g / 12.92f) : (float)pow((g + 0.055) / 1.055, 2.4);
  b = (b <= 0.04045f) ? (b / 12.92f) : (float)pow((b + 0.055) / 1.055, 2.4);

  *x = r * 0.4124f + g * 0.3576f + b * 0.1805f;
  *y = r * 0.2126f + g * 0.7152f + b * 0.0722f;
  *z = r * 0.0193f + g * 0.1192f + b * 0.9505f;
  return UI_ERROR_NONE;
}

static ui_error_t xyz_to_srgb(float x, float y, float z, float *r, float *g,
                              float *b) {
  float r_lin = x * 3.2406f + y * -1.5372f + z * -0.4986f;
  float g_lin = x * -0.9689f + y * 1.8758f + z * 0.0415f;
  float b_lin = x * 0.0557f + y * -0.2040f + z * 1.0570f;

  *r = (r_lin <= 0.0031308f) ? (12.92f * r_lin)
                             : (1.055f * (float)pow(r_lin, 1.0 / 2.4) - 0.055f);
  *g = (g_lin <= 0.0031308f) ? (12.92f * g_lin)
                             : (1.055f * (float)pow(g_lin, 1.0 / 2.4) - 0.055f);
  *b = (b_lin <= 0.0031308f) ? (12.92f * b_lin)
                             : (1.055f * (float)pow(b_lin, 1.0 / 2.4) - 0.055f);
  return UI_ERROR_NONE;
}

static float calc_lab_f(float t) {
  if (t > (216.0f / 24389.0f)) {
    return (float)pow(t, 1.0 / 3.0);
  }
  return (841.0f / 108.0f) * t + (4.0f / 29.0f);
}

static float calc_lab_f_inv(float t) {
  float t3 = t * t * t;
  if (t3 > (216.0f / 24389.0f)) {
    return t3;
  }
  return (108.0f / 841.0f) * (t - (4.0f / 29.0f));
}

static ui_error_t xyz_to_lab(float x, float y, float z, float *l, float *a,
                             float *b_out) {
  /* D65 standard illuminant */
  float xn = 0.95047f;
  float yn = 1.00000f;
  float zn = 1.08883f;

  float fx = calc_lab_f(x / xn);
  float fy = calc_lab_f(y / yn);
  float fz = calc_lab_f(z / zn);

  *l = 116.0f * fy - 16.0f;
  *a = 500.0f * (fx - fy);
  *b_out = 200.0f * (fy - fz);
  return UI_ERROR_NONE;
}

static ui_error_t lab_to_xyz(float l, float a, float b_in, float *x, float *y,
                             float *z) {
  float xn = 0.95047f;
  float yn = 1.00000f;
  float zn = 1.08883f;

  float fy = (l + 16.0f) / 116.0f;
  float fx = a / 500.0f + fy;
  float fz = fy - b_in / 200.0f;

  *x = xn * calc_lab_f_inv(fx);
  *y = yn * calc_lab_f_inv(fy);
  *z = zn * calc_lab_f_inv(fz);
  return UI_ERROR_NONE;
}

static ui_error_t lab_to_lch(float l, float a, float b_in, float *l_out,
                             float *c, float *h) {
  *l_out = l;
  *c = (float)sqrt(a * a + b_in * b_in);
  *h = (float)(atan2(b_in, a) * 180.0 / M_PI);
  if (*h < 0.0f) {
    *h += 360.0f;
  }
  return UI_ERROR_NONE;
}

static ui_error_t lch_to_lab(float l, float c, float h, float *l_out, float *a,
                             float *b_out) {
  float h_rad = h * (float)M_PI / 180.0f;
  *l_out = l;
  *a = c * (float)cos(h_rad);
  *b_out = c * (float)sin(h_rad);
  return UI_ERROR_NONE;
}

static float math_clamp(float v, float min, float max) {
  if (v < min)
    return min;
  if (v > max)
    return max;
  return v;
}

ui_error_t ui_color_argb_to_cam16(ui_color_t argb,
                                  struct ui_color_cam16 *out_cam16) {
  float r, g, b, x, y, z, l, a_val, b_val, l_ch, c, h;

  if (!out_cam16) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  r = UI_COLOR_RED(argb) / 255.0f;
  g = UI_COLOR_GREEN(argb) / 255.0f;
  b = UI_COLOR_BLUE(argb) / 255.0f;

  {
    ui_error_t rc;
    (void)srgb_to_xyz(r, g, b, &x, &y, &z);
    (void)xyz_to_lab(x, y, z, &l, &a_val, &b_val);
    (void)lab_to_lch(l, a_val, b_val, &l_ch, &c, &h);
  }

  out_cam16->hue = h;
  out_cam16->chroma = c;
  out_cam16->j = l_ch;
  out_cam16->q = l_ch;     /* Approximated */
  out_cam16->m = c;        /* Approximated */
  out_cam16->s = c / 1.5f; /* Approximated */

  return UI_ERROR_NONE;
}

ui_error_t ui_color_cam16_to_argb(const struct ui_color_cam16 *cam16,
                                  ui_color_t *out_argb) {
  float l_ch, a_val, b_val, x, y, z, r, g, b;
  ui_uint8 r8, g8, b8;

  if (!cam16 || !out_argb) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t rc;
    (void)lch_to_lab(cam16->j, cam16->chroma, cam16->hue, &l_ch, &a_val,
                     &b_val);
    (void)lab_to_xyz(l_ch, a_val, b_val, &x, &y, &z);
    (void)xyz_to_srgb(x, y, z, &r, &g, &b);
  }

  r8 = (ui_uint8)(math_clamp(r, 0.0f, 1.0f) * 255.0f + 0.5f);
  g8 = (ui_uint8)(math_clamp(g, 0.0f, 1.0f) * 255.0f + 0.5f);
  b8 = (ui_uint8)(math_clamp(b, 0.0f, 1.0f) * 255.0f + 0.5f);

  *out_argb = UI_COLOR_ARGB(255, r8, g8, b8);
  return UI_ERROR_NONE;
}

ui_error_t ui_color_argb_to_hct(ui_color_t argb, struct ui_color_hct *out_hct) {
  float r, g, b, x, y, z, l, a_val, b_val, l_ch, c, h;

  if (!out_hct) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  r = UI_COLOR_RED(argb) / 255.0f;
  g = UI_COLOR_GREEN(argb) / 255.0f;
  b = UI_COLOR_BLUE(argb) / 255.0f;

  /* Use LCh as a close surrogate for HCT */
  {
    ui_error_t rc;
    (void)srgb_to_xyz(r, g, b, &x, &y, &z);
    (void)xyz_to_lab(x, y, z, &l, &a_val, &b_val);
    (void)lab_to_lch(l, a_val, b_val, &l_ch, &c, &h);
  }

  out_hct->hue = h;
  out_hct->chroma = c;
  out_hct->tone = l_ch;

  return UI_ERROR_NONE;
}

ui_error_t ui_color_hct_to_argb(const struct ui_color_hct *hct,
                                ui_color_t *out_argb) {
  float l_ch, a_val, b_val, x, y, z, r, g, b;
  ui_uint8 r8, g8, b8;

  if (!hct || !out_argb) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t rc;
    (void)lch_to_lab(hct->tone, hct->chroma, hct->hue, &l_ch, &a_val, &b_val);
    (void)lab_to_xyz(l_ch, a_val, b_val, &x, &y, &z);
    (void)xyz_to_srgb(x, y, z, &r, &g, &b);
  }

  r8 = (ui_uint8)(math_clamp(r, 0.0f, 1.0f) * 255.0f + 0.5f);
  g8 = (ui_uint8)(math_clamp(g, 0.0f, 1.0f) * 255.0f + 0.5f);
  b8 = (ui_uint8)(math_clamp(b, 0.0f, 1.0f) * 255.0f + 0.5f);

  *out_argb = UI_COLOR_ARGB(255, r8, g8, b8);
  return UI_ERROR_NONE;
}
