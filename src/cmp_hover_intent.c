/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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
  int rc = CMP_SUCCESS;
  struct cmp_hover_intent *ctx = NULL;

  if (!out_intent) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_hover_intent_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_hover_intent), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_hover_intent_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_hover_intent));
  ctx->required_time_ms = 150.0f; /* Default 150ms hover delay */
  ctx->tolerance_radius = 5.0f;   /* Default 5px wobble tolerance */
  ctx->last_x = -1.0f;
  ctx->last_y = -1.0f;

  *out_intent = (cmp_hover_intent_t *)ctx;
  return rc;
}

int cmp_hover_intent_destroy(cmp_hover_intent_t *intent) {
  int rc = CMP_SUCCESS;
  struct cmp_hover_intent *ctx = (struct cmp_hover_intent *)intent;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_hover_intent_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(ctx);
  return rc;
}

int cmp_hover_intent_process(cmp_hover_intent_t *intent,
                             const cmp_event_t *event, float dt_ms) {
  int rc = 0; /* Returns boolean 0 or 1, or negative error */
  struct cmp_hover_intent *ctx = (struct cmp_hover_intent *)intent;
  float dx, dy, dist;

  if (!ctx || !event) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_hover_intent_process: Invalid argument\n");
    return rc;
  }

  /* If it's a new interaction or pointer left bounding area entirely, reset */
  if (event->action == CMP_ACTION_CANCEL) {
    ctx->is_active = 0;
    ctx->time_spent_in_bounds = 0.0f;
    ctx->last_x = -1.0f;
    ctx->last_y = -1.0f;
    return rc; /* 0 */
  }

  if (ctx->last_x < 0.0f && ctx->last_y < 0.0f) {
    /* Initialize tracking position */
    ctx->last_x = (float)event->x;
    ctx->last_y = (float)event->y;
    ctx->time_spent_in_bounds = 0.0f;
    ctx->is_active = 1;
    return rc; /* 0 */
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
    return rc; /* 0: Still deliberating */
  }

  /* Target is steady inside tolerance radius, accumulate time */
  ctx->time_spent_in_bounds += dt_ms;

  /* Confirmed hover intent! */
  if (ctx->time_spent_in_bounds >= ctx->required_time_ms) {
    rc = 1;
  }

  return rc;
}
