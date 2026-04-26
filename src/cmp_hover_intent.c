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
  float required_time_ms;
  float tolerance_radius;
  int is_tracking;
};

/**
 * @brief cmp_hover_intent_create
 *
 * @param out_intent Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_hover_intent_create(cmp_hover_intent_t **out_intent) {
  int rc = 0; /* CMP_SUCCESS */
  struct cmp_hover_intent *ctx = NULL;

  if (!out_intent) {
    LOG_DEBUG("cmp_hover_intent_create: invalid argument (out_intent is NULL)\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_hover_intent), (void **)&ctx);
  if (rc != 0) {
    LOG_DEBUG("cmp_hover_intent_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_hover_intent));
  ctx->required_time_ms = 500.0f; /* default half a second */
  ctx->tolerance_radius = 5.0f;   /* default 5px tolerance */
  ctx->is_tracking = 0;

  *out_intent = (cmp_hover_intent_t *)ctx;
  return 0;
}

/**
 * @brief cmp_hover_intent_destroy
 *
 * @param intent Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_hover_intent_destroy(cmp_hover_intent_t *intent) {
  int rc = 0; /* CMP_SUCCESS */
  struct cmp_hover_intent *ctx = (struct cmp_hover_intent *)intent;

  if (!ctx) {
    LOG_DEBUG("cmp_hover_intent_destroy: invalid argument (intent is NULL)\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(ctx);
  if (rc != 0) {
    LOG_DEBUG("cmp_hover_intent_destroy: CMP_FREE failed\n");
  }
  return rc;
}

/**
 * @brief Processes input to determine if a hover intent has been confirmed.
 *
 * @param intent Pointer to the hover intent context.
 * @param event Pointer to the event to process.
 * @param dt_ms Delta time in milliseconds since the last process call.
 * @param out_confirmed Pointer to an integer which will receive 1 if hover intent is confirmed, 0 otherwise.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_hover_intent_process(cmp_hover_intent_t *intent,
                             const cmp_event_t *event, float dt_ms, int *out_confirmed) {
  int rc = 0; /* CMP_SUCCESS */
  struct cmp_hover_intent *ctx = (struct cmp_hover_intent *)intent;
  float dx;
  float dy;
  float dist_sq;

  if (!ctx || !event || !out_confirmed) {
    LOG_DEBUG("cmp_hover_intent_process: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  
  *out_confirmed = 0;

  if (event->type == CMP_EVENT_POINTER_DOWN ||
      event->type == CMP_EVENT_POINTER_UP) {
    /* Click or touch clears intent */
    ctx->is_tracking = 0;
    ctx->time_spent_in_bounds = 0.0f;
    return 0;
  }

  if (event->type == CMP_EVENT_POINTER_MOVE) {
    if (!ctx->is_tracking) {
      ctx->is_tracking = 1;
      ctx->last_x = event->pointer.x;
      ctx->last_y = event->pointer.y;
      ctx->time_spent_in_bounds = 0.0f;
      return 0;
    }

    dx = event->pointer.x - ctx->last_x;
    dy = event->pointer.y - ctx->last_y;
    dist_sq = (dx * dx) + (dy * dy);

    if (dist_sq > (ctx->tolerance_radius * ctx->tolerance_radius)) {
      /* Moved outside tolerance, reset tracking origin and time */
      ctx->last_x = event->pointer.x;
      ctx->last_y = event->pointer.y;
      ctx->time_spent_in_bounds = 0.0f;
      return 0;
    }
  }

  if (!ctx->is_tracking) {
    return 0;
  }

  /* Target is steady inside tolerance radius, accumulate time */
  ctx->time_spent_in_bounds += dt_ms;

  /* Confirmed hover intent! */
  if (ctx->time_spent_in_bounds >= ctx->required_time_ms) {
    *out_confirmed = 1;
  }

  return 0;
}
