/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_stylus_context {
  cmp_stylus_event_cb event_cb;
  void *userdata;
};

static int g_scribble_enabled = 1;

int cmp_stylus_context_create(cmp_stylus_context_t **out_context) {
  struct cmp_stylus_context *ctx;
  if (!out_context)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_stylus_context), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->event_cb = NULL;
  ctx->userdata = NULL;

  *out_context = (cmp_stylus_context_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_stylus_context_destroy(cmp_stylus_context_t *context_opaque) {
  if (context_opaque)
    CMP_FREE(context_opaque);
  return CMP_SUCCESS;
}

int cmp_stylus_context_set_event_callback(cmp_stylus_context_t *context_opaque,
                                          cmp_stylus_event_cb callback,
                                          void *userdata) {
  struct cmp_stylus_context *ctx = (struct cmp_stylus_context *)context_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->event_cb = callback;
  ctx->userdata = userdata;
  return CMP_SUCCESS;
}

int cmp_stylus_resolve_ink_metrics(const cmp_event_t *event, float base_width,
                                   float *out_brush_opacity,
                                   float *out_brush_width) {
  if (!event || !out_brush_opacity || !out_brush_width)
    return CMP_ERROR_INVALID_ARG;

  /* Assume pointer is a stylus */
  /* Stubbed: Assuming type fields map for stylus */
  if (event->action != CMP_ACTION_CANCEL) {
    /* Scale opacity directly by pressure */
    *out_brush_opacity = event->pressure;
    if (*out_brush_opacity < 0.1f)
      *out_brush_opacity = 0.1f; /* Base minimum */
    if (*out_brush_opacity > 1.0f)
      *out_brush_opacity = 1.0f;

    /* Altitude: 0.0 is flat to screen, 1.0 is perpendicular.
       Flatter pencils draw wider "charcoal" strokes */
    float tilt_multiplier = 1.0f + ((1.0f - 0.5f /* Stubbed altitude */) *
                                    3.0f); /* Max 4x width if flat */
    *out_brush_width = base_width * tilt_multiplier;
  } else {
    *out_brush_opacity = 0.0f;
    *out_brush_width = base_width;
  }

  return CMP_SUCCESS;
}

int cmp_stylus_evaluate_hover(const cmp_event_t *event, int *out_is_hovering,
                              float *out_distance) {
  if (!event || !out_is_hovering || !out_distance)
    return CMP_ERROR_INVALID_ARG;

  /* Represents the iPad Pro M2+ hover event before touch */
  /* Stubbed hover interaction logic */
  if (event->action == CMP_ACTION_MOVE && event->pressure == 0.0f) {
    *out_is_hovering = 1;
    *out_distance =
        0.5f /* Stubbed distance */; /* Assumes OS normalizes to 0-1 */
  } else {
    *out_is_hovering = 0;
    *out_distance = 0.0f;
  }
  return CMP_SUCCESS;
}

int cmp_stylus_set_scribble_enabled(int is_enabled) {
  g_scribble_enabled = is_enabled;
  return CMP_SUCCESS;
}
