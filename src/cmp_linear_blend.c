/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_linear_blend {
  float display_gamma;
  float inv_gamma;
};

/**
 * @brief cmp_linear_blend_create
 *
 * @param gamma Parameter description.
 * @param out_blend Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_linear_blend_create(float gamma, cmp_linear_blend_t **out_blend) {
  int rc = CMP_SUCCESS;
  struct cmp_linear_blend *blend = NULL;

  if (!out_blend) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_linear_blend_create: Invalid argument "
              "(out_blend=NULL)\n");

    return rc;
  }
  if (gamma <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_linear_blend_create: Invalid gamma\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_linear_blend), (void **)&blend);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_linear_blend_create: Out of memory\n");

    return rc;
  }

  blend->display_gamma = gamma;
  blend->inv_gamma = 1.0f / gamma;

  *out_blend = (cmp_linear_blend_t *)blend;

  return rc;
}

/**
 * @brief cmp_linear_blend_destroy
 *
 * @param blend Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_linear_blend_destroy(cmp_linear_blend_t *blend) {
  int rc = CMP_SUCCESS;

  if (!blend) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_linear_blend_destroy: Invalid argument\n");

    return rc;
  }

  rc = CMP_FREE(blend);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_linear_blend_destroy: CMP_FREE failed\n");
  }

  return rc;
}

/**
 * @brief cmp_linear_blend_srgb_to_linear
 *
 * @param blend Parameter description.
 * @param srgb Parameter description.
 * @param out_linear Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_linear_blend_srgb_to_linear(cmp_linear_blend_t *blend,
                                    const cmp_color_t *srgb,
                                    cmp_color_t *out_linear) {
  int rc = CMP_SUCCESS;
  struct cmp_linear_blend *ctx = (struct cmp_linear_blend *)blend;

  if (!ctx || !srgb || !out_linear) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_linear_blend_srgb_to_linear: Invalid argument\n");

    return rc;
  }

  out_linear->r = (float)pow((double)srgb->r, (double)ctx->display_gamma);
  out_linear->g = (float)pow((double)srgb->g, (double)ctx->display_gamma);
  out_linear->b = (float)pow((double)srgb->b, (double)ctx->display_gamma);
  out_linear->a = srgb->a;
  out_linear->space =
      CMP_COLOR_SPACE_SRGB; /* still in standard gamut, just linear value */

  return rc;
}

/**
 * @brief cmp_linear_blend_linear_to_srgb
 *
 * @param blend Parameter description.
 * @param linear Parameter description.
 * @param out_srgb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_linear_blend_linear_to_srgb(cmp_linear_blend_t *blend,
                                    const cmp_color_t *linear,
                                    cmp_color_t *out_srgb) {
  int rc = CMP_SUCCESS;
  struct cmp_linear_blend *ctx = (struct cmp_linear_blend *)blend;

  if (!ctx || !linear || !out_srgb) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_linear_blend_linear_to_srgb: Invalid argument\n");

    return rc;
  }

  out_srgb->r = (float)pow((double)linear->r, (double)ctx->inv_gamma);
  out_srgb->g = (float)pow((double)linear->g, (double)ctx->inv_gamma);
  out_srgb->b = (float)pow((double)linear->b, (double)ctx->inv_gamma);
  out_srgb->a = linear->a;
  out_srgb->space = CMP_COLOR_SPACE_SRGB;

  return rc;
}

/**
 * @brief cmp_linear_blend_mix
 *
 * @param blend Parameter description.
 * @param bg Parameter description.
 * @param fg Parameter description.
 * @param alpha Parameter description.
 * @param out_blended Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_linear_blend_mix(cmp_linear_blend_t *blend, const cmp_color_t *bg,
                         const cmp_color_t *fg, float alpha,
                         cmp_color_t *out_blended) {
  int rc = CMP_SUCCESS;
  cmp_color_t bg_lin, fg_lin, mix_lin;
  float blend_alpha;

  if (!blend || !bg || !fg || !out_blended) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_linear_blend_mix: Invalid argument\n");

    return rc;
  }

  if (alpha < 0.0f)
    alpha = 0.0f;
  if (alpha > 1.0f)
    alpha = 1.0f;

  rc = cmp_linear_blend_srgb_to_linear(blend, bg, &bg_lin);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_linear_blend_mix: cmp_linear_blend_srgb_to_linear "
              "failed for bg\n");

    return rc;
  }

  rc = cmp_linear_blend_srgb_to_linear(blend, fg, &fg_lin);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_linear_blend_mix: cmp_linear_blend_srgb_to_linear "
              "failed for fg\n");

    return rc;
  }

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

  rc = cmp_linear_blend_linear_to_srgb(blend, &mix_lin, out_blended);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_linear_blend_mix: cmp_linear_blend_linear_to_srgb "
              "failed\n");
  }

  return rc;
}
