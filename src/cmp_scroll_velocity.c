/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

#define CMP_SCROLL_HISTORY_MAX 5

struct cmp_scroll_velocity {
  float dx[CMP_SCROLL_HISTORY_MAX];
  float dy[CMP_SCROLL_HISTORY_MAX];
  float dt[CMP_SCROLL_HISTORY_MAX];
  int head;
  int count;
  float last_x;
  float last_y;
};

/**
 * @brief Create a scroll velocity tracker.
 *
 * @param out_tracker Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_velocity_create(cmp_scroll_velocity_t **out_tracker) {
  int rc;
  struct cmp_scroll_velocity *tracker;

  rc = CMP_SUCCESS;

  if (out_tracker == NULL) {
    LOG_DEBUG("Invalid argument: out_tracker is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_scroll_velocity), (void **)&tracker);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(tracker, 0, sizeof(struct cmp_scroll_velocity));
  tracker->last_x = -1.0f;
  tracker->last_y = -1.0f;

  *out_tracker = (cmp_scroll_velocity_t *)tracker;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a scroll velocity tracker.
 *
 * @param tracker Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_velocity_destroy(cmp_scroll_velocity_t *tracker) {
  int rc;
  struct cmp_scroll_velocity *ctx;

  rc = CMP_SUCCESS;
  ctx = (struct cmp_scroll_velocity *)tracker;

  if (ctx == NULL) {
    LOG_DEBUG("Invalid argument: tracker is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Push an event to the scroll velocity tracker.
 *
 * @param tracker Parameter description.
 * @param event Parameter description.
 * @param dt_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_velocity_push(cmp_scroll_velocity_t *tracker,
                             const cmp_event_t *event, float dt_ms) {
  struct cmp_scroll_velocity *ctx;
  float dx, dy;

  ctx = (struct cmp_scroll_velocity *)tracker;

  if (ctx == NULL || event == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (event->action == CMP_ACTION_DOWN) {
    ctx->head = 0;
    ctx->count = 0;
    ctx->last_x = (float)event->x;
    ctx->last_y = (float)event->y;
    return CMP_SUCCESS;
  }

  if (event->action == CMP_ACTION_MOVE && dt_ms > 0.0f) {
    if (ctx->last_x < 0.0f) {
      ctx->last_x = (float)event->x;
      ctx->last_y = (float)event->y;
      return CMP_SUCCESS;
    }

    dx = (float)event->x - ctx->last_x;
    dy = (float)event->y - ctx->last_y;

    ctx->dx[ctx->head] = dx;
    ctx->dy[ctx->head] = dy;
    ctx->dt[ctx->head] = dt_ms;

    ctx->head = (ctx->head + 1) % CMP_SCROLL_HISTORY_MAX;
    if (ctx->count < CMP_SCROLL_HISTORY_MAX) {
      ctx->count++;
    }

    ctx->last_x = (float)event->x;
    ctx->last_y = (float)event->y;
  } else if (event->action == CMP_ACTION_UP ||
             event->action == CMP_ACTION_CANCEL) {
    /* Keep existing history for calculation but reset tracking positions */
    ctx->last_x = -1.0f;
    ctx->last_y = -1.0f;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Get the calculated scroll velocity.
 *
 * @param tracker Parameter description.
 * @param out_vx Parameter description.
 * @param out_vy Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_velocity_get(const cmp_scroll_velocity_t *tracker, float *out_vx,
                            float *out_vy) {
  const struct cmp_scroll_velocity *ctx;
  int i;
  float total_dx;
  float total_dy;
  float total_dt;

  ctx = (const struct cmp_scroll_velocity *)tracker;
  total_dx = 0.0f;
  total_dy = 0.0f;
  total_dt = 0.0f;

  if (ctx == NULL || (out_vx == NULL && out_vy == NULL)) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (out_vx != NULL) {
    *out_vx = 0.0f;
  }
  if (out_vy != NULL) {
    *out_vy = 0.0f;
  }

  if (ctx->count == 0) {
    return CMP_SUCCESS;
  }

  for (i = 0; i < ctx->count; i++) {
    total_dx += ctx->dx[i];
    total_dy += ctx->dy[i];
    total_dt += ctx->dt[i];
  }

  if (total_dt > 0.0f) {
    /* Convert ms to seconds for standard px/sec velocity */
    if (out_vx != NULL) {
      *out_vx = (total_dx / total_dt) * 1000.0f;
    }
    if (out_vy != NULL) {
      *out_vy = (total_dy / total_dt) * 1000.0f;
    }
  }

  return CMP_SUCCESS;
}
