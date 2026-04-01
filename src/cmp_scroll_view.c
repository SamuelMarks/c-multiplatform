/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <math.h>
/* clang-format on */

struct cmp_scroll_view {
  cmp_scroll_deceleration_rate_t deceleration_rate;
  cmp_scroll_keyboard_dismiss_t keyboard_dismiss_mode;
  int is_paging_enabled;
};

int cmp_scroll_view_create(cmp_scroll_view_t **out_scroll_view) {
  struct cmp_scroll_view *ctx;
  if (!out_scroll_view)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_scroll_view), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->deceleration_rate = CMP_SCROLL_DECELERATION_NORMAL;
  ctx->keyboard_dismiss_mode = CMP_SCROLL_KEYBOARD_DISMISS_NONE;
  ctx->is_paging_enabled = 0;

  *out_scroll_view = (cmp_scroll_view_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_scroll_view_destroy(cmp_scroll_view_t *scroll_view_opaque) {
  struct cmp_scroll_view *ctx = (struct cmp_scroll_view *)scroll_view_opaque;
  if (!ctx)
    return CMP_SUCCESS;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_scroll_view_set_deceleration_rate(cmp_scroll_view_t *scroll_view_opaque,
                                          cmp_scroll_deceleration_rate_t rate) {
  struct cmp_scroll_view *ctx = (struct cmp_scroll_view *)scroll_view_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->deceleration_rate = rate;
  return CMP_SUCCESS;
}

int cmp_scroll_view_set_paging_enabled(cmp_scroll_view_t *scroll_view_opaque,
                                       int is_paging_enabled) {
  struct cmp_scroll_view *ctx = (struct cmp_scroll_view *)scroll_view_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->is_paging_enabled = is_paging_enabled;
  return CMP_SUCCESS;
}

int cmp_scroll_view_evaluate_scrollbar(cmp_scroll_view_t *scroll_view_opaque,
                                       int is_scrolling,
                                       float time_since_last_scroll,
                                       float *out_opacity) {
  struct cmp_scroll_view *ctx = (struct cmp_scroll_view *)scroll_view_opaque;
  if (!ctx || !out_opacity)
    return CMP_ERROR_INVALID_ARG;

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

int cmp_scroll_view_hit_test_scrollbar(cmp_scroll_view_t *scroll_view_opaque,
                                       float pointer_x, float pointer_y,
                                       float view_width,
                                       int *out_is_interactive) {
  struct cmp_scroll_view *ctx = (struct cmp_scroll_view *)scroll_view_opaque;
  (void)pointer_y; /* Assuming full height bar hit zone for simplicity */
  if (!ctx || !out_is_interactive)
    return CMP_ERROR_INVALID_ARG;

  /* HIG scroll indicator is on the trailing edge, ~8pt wide hit zone */
  if (pointer_x >= view_width - 16.0f && pointer_x <= view_width) {
    *out_is_interactive = 1;
  } else {
    *out_is_interactive = 0;
  }

  return CMP_SUCCESS;
}

int cmp_scroll_view_calculate_rubber_band(cmp_scroll_view_t *scroll_view_opaque,
                                          float overscroll_delta,
                                          float view_dimension,
                                          float *out_visual_translation) {
  struct cmp_scroll_view *ctx = (struct cmp_scroll_view *)scroll_view_opaque;
  float c = 0.55f; /* Apple's constant */
  float val;
  if (!ctx || !out_visual_translation)
    return CMP_ERROR_INVALID_ARG;

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

int cmp_scroll_view_set_keyboard_dismiss_mode(
    cmp_scroll_view_t *scroll_view_opaque, cmp_scroll_keyboard_dismiss_t mode) {
  struct cmp_scroll_view *ctx = (struct cmp_scroll_view *)scroll_view_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->keyboard_dismiss_mode = mode;
  return CMP_SUCCESS;
}
