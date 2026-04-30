/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_stylus_context {
  cmp_stylus_event_cb event_cb;
  void *userdata;
};

static int g_scribble_enabled = 1;

/**
 * @brief cmp_stylus_context_create
 *
 * @param out_context Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stylus_context_create(cmp_stylus_context_t **out_context) {
  int rc = CMP_SUCCESS;
  struct cmp_stylus_context *ctx;
  if (!out_context)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_stylus_context), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->event_cb = NULL;
  ctx->userdata = NULL;

  *out_context = (cmp_stylus_context_t *)ctx;

  return rc;
}

/**
 * @brief cmp_stylus_context_destroy
 *
 * @param context_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stylus_context_destroy(cmp_stylus_context_t *context_opaque) {
  int rc = CMP_SUCCESS;
  if (context_opaque)
    CMP_FREE(context_opaque);

  return rc;
}

/**
 * @brief cmp_stylus_context_set_event_callback
 *
 * @param context_opaque Parameter description.
 * @param callback Parameter description.
 * @param userdata Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stylus_context_set_event_callback(cmp_stylus_context_t *context_opaque,
                                          cmp_stylus_event_cb callback,
                                          void *userdata) {
  int rc = CMP_SUCCESS;
  struct cmp_stylus_context *ctx = (struct cmp_stylus_context *)context_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->event_cb = callback;
  ctx->userdata = userdata;

  return rc;
}

/**
 * @brief cmp_stylus_resolve_ink_metrics
 *
 * @param event Parameter description.
 * @param base_width Parameter description.
 * @param out_brush_opacity Parameter description.
 * @param out_brush_width Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stylus_resolve_ink_metrics(const cmp_event_t *event, float base_width,
                                   float *out_brush_opacity,
                                   float *out_brush_width) {
  int rc = CMP_SUCCESS;
  if (!event || !out_brush_opacity || !out_brush_width)
    return CMP_ERROR_INVALID_ARG;

  /* Assume pointer is a stylus */
  if (event->action != CMP_ACTION_CANCEL) {
    float tilt_multiplier;
    /* Scale opacity directly by pressure */
    *out_brush_opacity = event->pressure;
    if (*out_brush_opacity < 0.1f)
      *out_brush_opacity = 0.1f; /* Base minimum */
    if (*out_brush_opacity > 1.0f)
      *out_brush_opacity = 1.0f;

    /* Altitude: 0.0 is flat to screen, 1.0 is perpendicular.
       Flatter pencils draw wider "charcoal" strokes */
    tilt_multiplier =
        1.0f + ((1.0f - event->altitude) * 3.0f); /* Max 4x width if flat */
    *out_brush_width = base_width * tilt_multiplier;
  } else {
    *out_brush_opacity = 0.0f;
    *out_brush_width = base_width;
  }

  return rc;
}

/**
 * @brief cmp_stylus_evaluate_hover
 *
 * @param event Parameter description.
 * @param out_is_hovering Parameter description.
 * @param out_distance Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stylus_evaluate_hover(const cmp_event_t *event, int *out_is_hovering,
                              float *out_distance) {
  int rc = CMP_SUCCESS;
  if (!event || !out_is_hovering || !out_distance)
    return CMP_ERROR_INVALID_ARG;

  /* Represents the iPad Pro M2+ hover event before touch */
  if (event->action == CMP_ACTION_MOVE && event->pressure == 0.0f) {
    *out_is_hovering = 1;
    *out_distance = event->distance; /* Assumes OS normalizes to 0-1 */
  } else {
    *out_is_hovering = 0;
    *out_distance = 0.0f;
  }

  return rc;
}

/**
 * @brief cmp_stylus_set_scribble_enabled
 *
 * @param is_enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stylus_set_scribble_enabled(int is_enabled) {
  int rc = CMP_SUCCESS;
  g_scribble_enabled = is_enabled;

  return rc;
}
