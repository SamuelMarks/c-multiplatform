/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_linear_blend {
  float display_gamma;
  float inv_gamma;
};

int cmp_linear_blend_create(float gamma, cmp_linear_blend_t **out_blend) {
  struct cmp_linear_blend *blend;
  if (!out_blend)
    return CMP_ERROR_INVALID_ARG;
  if (gamma <= 0.0f)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_linear_blend), (void **)&blend) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  blend->display_gamma = gamma;
  blend->inv_gamma = 1.0f / gamma;

  *out_blend = (cmp_linear_blend_t *)blend;
  return CMP_SUCCESS;
}

int cmp_linear_blend_destroy(cmp_linear_blend_t *blend) {
  if (!blend)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(blend);
  return CMP_SUCCESS;
}

int cmp_linear_blend_srgb_to_linear(cmp_linear_blend_t *blend,
                                    const cmp_color_t *srgb,
                                    cmp_color_t *out_linear) {
  struct cmp_linear_blend *ctx = (struct cmp_linear_blend *)blend;
  if (!ctx || !srgb || !out_linear)
    return CMP_ERROR_INVALID_ARG;

  out_linear->r = (float)pow((double)srgb->r, (double)ctx->display_gamma);
  out_linear->g = (float)pow((double)srgb->g, (double)ctx->display_gamma);
  out_linear->b = (float)pow((double)srgb->b, (double)ctx->display_gamma);
  out_linear->a = srgb->a;
  out_linear->space =
      CMP_COLOR_SPACE_SRGB; /* still in standard gamut, just linear value */

  return CMP_SUCCESS;
}

int cmp_linear_blend_linear_to_srgb(cmp_linear_blend_t *blend,
                                    const cmp_color_t *linear,
                                    cmp_color_t *out_srgb) {
  struct cmp_linear_blend *ctx = (struct cmp_linear_blend *)blend;
  if (!ctx || !linear || !out_srgb)
    return CMP_ERROR_INVALID_ARG;

  out_srgb->r = (float)pow((double)linear->r, (double)ctx->inv_gamma);
  out_srgb->g = (float)pow((double)linear->g, (double)ctx->inv_gamma);
  out_srgb->b = (float)pow((double)linear->b, (double)ctx->inv_gamma);
  out_srgb->a = linear->a;
  out_srgb->space = CMP_COLOR_SPACE_SRGB;

  return CMP_SUCCESS;
}

int cmp_linear_blend_mix(cmp_linear_blend_t *blend, const cmp_color_t *bg,
                         const cmp_color_t *fg, float alpha,
                         cmp_color_t *out_blended) {
  cmp_color_t bg_lin, fg_lin, mix_lin;
  float blend_alpha;
  int err;

  if (!blend || !bg || !fg || !out_blended)
    return CMP_ERROR_INVALID_ARG;

  if (alpha < 0.0f)
    alpha = 0.0f;
  if (alpha > 1.0f)
    alpha = 1.0f;

  err = cmp_linear_blend_srgb_to_linear(blend, bg, &bg_lin);
  if (err != CMP_SUCCESS)
    return err;

  err = cmp_linear_blend_srgb_to_linear(blend, fg, &fg_lin);
  if (err != CMP_SUCCESS)
    return err;

  blend_alpha = fg->a * alpha;

  /* Standard alpha blending over opaque/transparent background in linear space
   */
  /* C_out = C_fg * A_fg + C_bg * A_bg * (1 - A_fg) */
  /* A_out = A_fg + A_bg * (1 - A_fg) */

  mix_lin.a = blend_alpha + bg_lin.a * (1.0f - blend_alpha);

  if (mix_lin.a > 0.0f) {
    mix_lin.r =
        (fg_lin.r * blend_alpha + bg_lin.r * bg_lin.a * (1.0f - blend_alpha)) /
        mix_lin.a;
    mix_lin.g =
        (fg_lin.g * blend_alpha + bg_lin.g * bg_lin.a * (1.0f - blend_alpha)) /
        mix_lin.a;
    mix_lin.b =
        (fg_lin.b * blend_alpha + bg_lin.b * bg_lin.a * (1.0f - blend_alpha)) /
        mix_lin.a;
  } else {
    mix_lin.r = 0.0f;
    mix_lin.g = 0.0f;
    mix_lin.b = 0.0f;
  }

  return cmp_linear_blend_linear_to_srgb(blend, &mix_lin, out_blended);
}
