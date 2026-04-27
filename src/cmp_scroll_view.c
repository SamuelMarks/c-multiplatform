/* clang-format off */
#include "cmp.h"

#include <math.h>
#include <stdlib.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_scroll_view {
  cmp_scroll_deceleration_rate_t deceleration_rate;
  cmp_scroll_keyboard_dismiss_t keyboard_dismiss_mode;
  int is_paging_enabled;
};

/**
 * @brief Create a scroll view context.
 *
 * @param out_scroll_view Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_view_create(cmp_scroll_view_t **out_scroll_view) {
  int rc;
  struct cmp_scroll_view *ctx;

  rc = CMP_SUCCESS;

  if (out_scroll_view == NULL) {
    LOG_DEBUG("Invalid argument: out_scroll_view is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_scroll_view), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  ctx->deceleration_rate = CMP_SCROLL_DECELERATION_NORMAL;
  ctx->keyboard_dismiss_mode = CMP_SCROLL_KEYBOARD_DISMISS_NONE;
  ctx->is_paging_enabled = 0;

  *out_scroll_view = (cmp_scroll_view_t *)ctx;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a scroll view context.
 *
 * @param scroll_view_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_view_destroy(cmp_scroll_view_t *scroll_view_opaque) {
  int rc;
  struct cmp_scroll_view *ctx;

  rc = CMP_SUCCESS;
  ctx = (struct cmp_scroll_view *)scroll_view_opaque;

  if (ctx == NULL) {
    return CMP_SUCCESS;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Set deceleration rate.
 *
 * @param scroll_view_opaque Parameter description.
 * @param rate Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_view_set_deceleration_rate(cmp_scroll_view_t *scroll_view_opaque,
                                          cmp_scroll_deceleration_rate_t rate) {
  struct cmp_scroll_view *ctx;

  ctx = (struct cmp_scroll_view *)scroll_view_opaque;

  if (ctx == NULL) {
    LOG_DEBUG("Invalid argument: scroll_view_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->deceleration_rate = rate;

  return CMP_SUCCESS;
}

/**
 * @brief Set whether paging is enabled.
 *
 * @param scroll_view_opaque Parameter description.
 * @param is_paging_enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_view_set_paging_enabled(cmp_scroll_view_t *scroll_view_opaque,
                                       int is_paging_enabled) {
  struct cmp_scroll_view *ctx;

  ctx = (struct cmp_scroll_view *)scroll_view_opaque;

  if (ctx == NULL) {
    LOG_DEBUG("Invalid argument: scroll_view_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->is_paging_enabled = is_paging_enabled;

  return CMP_SUCCESS;
}

/**
 * @brief Evaluate scrollbar visibility.
 *
 * @param scroll_view_opaque Parameter description.
 * @param is_scrolling Parameter description.
 * @param time_since_last_scroll Parameter description.
 * @param out_opacity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_view_evaluate_scrollbar(cmp_scroll_view_t *scroll_view_opaque,
                                       int is_scrolling,
                                       float time_since_last_scroll,
                                       float *out_opacity) {
  struct cmp_scroll_view *ctx;

  ctx = (struct cmp_scroll_view *)scroll_view_opaque;

  if (ctx == NULL || out_opacity == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (is_scrolling) {
    *out_opacity = 1.0f; /* Fully visible while moving */
  } else {
    /* Fades out quickly after ~500ms */
    if (time_since_last_scroll < 500.0f) {
      *out_opacity = 1.0f;
    } else if (time_since_last_scroll < 800.0f) {
      *out_opacity =
          1.0f - ((time_since_last_scroll - 500.0f) / 300.0f); /* linear fade */
    } else {
      *out_opacity = 0.0f; /* Hidden */
    }
  }

  return CMP_SUCCESS;
}

/**
 * @brief Hit test the scrollbar.
 *
 * @param scroll_view_opaque Parameter description.
 * @param pointer_x Parameter description.
 * @param pointer_y Parameter description.
 * @param view_width Parameter description.
 * @param out_is_interactive Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_view_hit_test_scrollbar(cmp_scroll_view_t *scroll_view_opaque,
                                       float pointer_x, float pointer_y,
                                       float view_width,
                                       int *out_is_interactive) {
  struct cmp_scroll_view *ctx;

  ctx = (struct cmp_scroll_view *)scroll_view_opaque;
  (void)pointer_y; /* Assuming full height bar hit zone for simplicity */

  if (ctx == NULL || out_is_interactive == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* HIG scroll indicator is on the trailing edge, ~8pt wide hit zone */
  if (pointer_x >= view_width - 16.0f && pointer_x <= view_width) {
    *out_is_interactive = 1;
  } else {
    *out_is_interactive = 0;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Calculate rubber band effect translation.
 *
 * @param scroll_view_opaque Parameter description.
 * @param overscroll_delta Parameter description.
 * @param view_dimension Parameter description.
 * @param out_visual_translation Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_view_calculate_rubber_band(cmp_scroll_view_t *scroll_view_opaque,
                                          float overscroll_delta,
                                          float view_dimension,
                                          float *out_visual_translation) {
  struct cmp_scroll_view *ctx;
  float c;
  float val;

  ctx = (struct cmp_scroll_view *)scroll_view_opaque;
  c = 0.55f; /* Apple's constant */

  if (ctx == NULL || out_visual_translation == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (view_dimension <= 0.0f) {
    *out_visual_translation = overscroll_delta;
    return CMP_SUCCESS;
  }

  /* Formula: (1.0 - (1.0 / ((overscroll * c / dimension) + 1.0))) * dimension
   */
  /* Handles positive bounds */
  if (overscroll_delta >= 0.0f) {
    val = (overscroll_delta * c / view_dimension) + 1.0f;
    *out_visual_translation = (1.0f - (1.0f / val)) * view_dimension;
  } else {
    /* Negative bounds */
    overscroll_delta = -overscroll_delta;
    val = (overscroll_delta * c / view_dimension) + 1.0f;
    *out_visual_translation = -((1.0f - (1.0f / val)) * view_dimension);
  }

  return CMP_SUCCESS;
}

/**
 * @brief Set keyboard dismiss mode.
 *
 * @param scroll_view_opaque Parameter description.
 * @param mode Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_view_set_keyboard_dismiss_mode(
    cmp_scroll_view_t *scroll_view_opaque, cmp_scroll_keyboard_dismiss_t mode) {
  struct cmp_scroll_view *ctx;

  ctx = (struct cmp_scroll_view *)scroll_view_opaque;

  if (ctx == NULL) {
    LOG_DEBUG("Invalid argument: scroll_view_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->keyboard_dismiss_mode = mode;

  return CMP_SUCCESS;
}
