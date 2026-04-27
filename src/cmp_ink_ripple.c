/* clang-format off */
#include "cmp_ink_ripple.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_ink_ripple {
  float radius;
  float target_radius;
  float origin_x;
  float origin_y;
  float opacity;

  float t_expand; /* 0.0 to 1.0 */
  float t_fade;   /* 0.0 to 1.0 */

  int active; /* 1 if expanding/held */
  int fading; /* 1 if pointer released */
};

/* Helper: Cubic Bezier (0, 0, 0, 1) - Standard Decelerate */
CMP_EXEMPT(static float cmp_math_cubic_bezier_decelerate(float t)) {
  float tt = t * t;
  float ttt = tt * t;
  float p = ttt * 1.0f;
  return p;
}

/* Helper: Cubic Bezier (0.3, 0, 1, 1) - Standard Accelerate */
CMP_EXEMPT(static float cmp_math_cubic_bezier_accelerate(float t)) {
  /* For standard accelerate, p0=0, p1=0, p2=1, p3=1 */
  /* This is just an approximation for ease-in */
  return t * t; /* Simple quadratic ease-in for alpha fade out */
}

/**
 * @brief cmp_ink_ripple_create
 *
 * @param out_ripple Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ink_ripple_create(cmp_ink_ripple_t **out_ripple) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_ink_ripple *ctx = NULL;

  if (!out_ripple) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ink_ripple_create: Invalid argument (out_ripple=NULL)\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_ink_ripple), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ink_ripple_create: Out of memory\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_ink_ripple));
  *out_ripple = (cmp_ink_ripple_t *)ctx;
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ink_ripple_destroy
 *
 * @param ripple Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ink_ripple_destroy(cmp_ink_ripple_t *ripple) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ink_ripple_destroy: Invalid argument (ripple=NULL)\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ink_ripple_destroy: CMP_FREE failed\n");
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ink_ripple_update
 *
 * @param ripple Parameter description.
 * @param dt_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ink_ripple_update(cmp_ink_ripple_t *ripple, float dt_ms) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;

  if (!ctx || dt_ms < 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ink_ripple_update: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->active) {
    ctx->t_expand += dt_ms / 200.0f; /* 200ms expansion */
    if (ctx->t_expand > 1.0f)
      ctx->t_expand = 1.0f;
    ctx->radius =
        ctx->target_radius * cmp_math_cubic_bezier_decelerate(ctx->t_expand);
  }

  if (ctx->fading) {
    ctx->t_fade += dt_ms / 150.0f; /* 150ms fade */
    if (ctx->t_fade > 1.0f) {
      ctx->t_fade = 1.0f;
      ctx->active = 0;
      ctx->fading = 0;
    }
    ctx->opacity = 0.12f * (1.0f - cmp_math_cubic_bezier_accelerate(
                                       ctx->t_fade)); /* Base ripple opacity */
  }

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ink_ripple_trigger
 *
 * @param ripple Parameter description.
 * @param start_x Parameter description.
 * @param start_y Parameter description.
 * @param box_width Parameter description.
 * @param box_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ink_ripple_trigger(cmp_ink_ripple_t *ripple, float start_x,
                           float start_y, float box_width, float box_height) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;
  float dx, dy, dx2, dy2, max_dist;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ink_ripple_trigger: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->origin_x = start_x;
  ctx->origin_y = start_y;

  dx = (start_x > box_width / 2.0f) ? start_x : box_width - start_x;
  dy = (start_y > box_height / 2.0f) ? start_y : box_height - start_y;
  dx2 = dx * dx;
  dy2 = dy * dy;
  max_dist = (float)sqrt(dx2 + dy2);

  ctx->target_radius = max_dist;
  ctx->radius = 0.0f;
  ctx->opacity = 0.12f; /* Ripple peak opacity */
  ctx->t_expand = 0.0f;
  ctx->t_fade = 0.0f;
  ctx->active = 1;
  ctx->fading = 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ink_ripple_release
 *
 * @param ripple Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ink_ripple_release(cmp_ink_ripple_t *ripple) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ink_ripple_release: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->active) {
    ctx->fading = 1;
  }
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ink_ripple_get_state
 *
 * @param ripple Parameter description.
 * @param out_radius Parameter description.
 * @param out_opacity Parameter description.
 * @param out_x Parameter description.
 * @param out_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ink_ripple_get_state(cmp_ink_ripple_t *ripple, float *out_radius,
                             float *out_opacity, float *out_x, float *out_y) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_ink_ripple *ctx = (struct cmp_ink_ripple *)ripple;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ink_ripple_get_state: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (out_radius)
    *out_radius = ctx->radius;
  if (out_opacity)
    *out_opacity = ctx->opacity;
  if (out_x)
    *out_x = ctx->origin_x;
  if (out_y)
    *out_y = ctx->origin_y;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}