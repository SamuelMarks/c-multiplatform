/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_hover_intent {
  float last_x;
  float last_y;
  float time_spent_in_bounds;
  int is_active;
  float required_time_ms;
  float tolerance_radius;
};

int cmp_hover_intent_create(cmp_hover_intent_t **out_intent) {
  struct cmp_hover_intent *ctx;

  if (!out_intent)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_hover_intent), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_hover_intent));
  ctx->required_time_ms = 150.0f; /* Default 150ms hover delay */
  ctx->tolerance_radius = 5.0f;   /* Default 5px wobble tolerance */
  ctx->last_x = -1.0f;
  ctx->last_y = -1.0f;

  *out_intent = (cmp_hover_intent_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_hover_intent_destroy(cmp_hover_intent_t *intent) {
  struct cmp_hover_intent *ctx = (struct cmp_hover_intent *)intent;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_hover_intent_process(cmp_hover_intent_t *intent,
                             const cmp_event_t *event, float dt_ms) {
  struct cmp_hover_intent *ctx = (struct cmp_hover_intent *)intent;
  float dx, dy, dist;

  if (!ctx || !event)
    return CMP_ERROR_INVALID_ARG;

  /* If it's a new interaction or pointer left bounding area entirely, reset */
  if (event->action == CMP_ACTION_CANCEL) {
    ctx->is_active = 0;
    ctx->time_spent_in_bounds = 0.0f;
    ctx->last_x = -1.0f;
    ctx->last_y = -1.0f;
    return 0;
  }

  if (ctx->last_x < 0.0f && ctx->last_y < 0.0f) {
    /* Initialize tracking position */
    ctx->last_x = (float)event->x;
    ctx->last_y = (float)event->y;
    ctx->time_spent_in_bounds = 0.0f;
    ctx->is_active = 1;
    return 0;
  }

  /* Calculate movement distance */
  dx = (float)event->x - ctx->last_x;
  dy = (float)event->y - ctx->last_y;
  dist = (float)sqrt((double)(dx * dx + dy * dy));

  /* If movement exceeded the hysteresis tolerance, reset timer */
  if (dist > ctx->tolerance_radius) {
    ctx->time_spent_in_bounds = 0.0f;
    ctx->last_x = (float)event->x;
    ctx->last_y = (float)event->y;
    return 0; /* Still deliberating */
  }

  /* Target is steady inside tolerance radius, accumulate time */
  ctx->time_spent_in_bounds += dt_ms;

  /* Confirmed hover intent! */
  if (ctx->time_spent_in_bounds >= ctx->required_time_ms) {
    return 1;
  }

  return 0;
}
