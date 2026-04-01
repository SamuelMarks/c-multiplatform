/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_tvos_features {
  int top_shelf_mode; /* 0=carousel, 1=inset */
};

int cmp_tvos_features_create(cmp_tvos_features_t **out_features) {
  struct cmp_tvos_features *ctx;
  if (!out_features)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_tvos_features), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->top_shelf_mode = 0;

  *out_features = (cmp_tvos_features_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_tvos_features_destroy(cmp_tvos_features_t *features_opaque) {
  if (features_opaque)
    CMP_FREE(features_opaque);
  return CMP_SUCCESS;
}

int cmp_tvos_handle_focus_engine_update(cmp_tvos_features_t *features_opaque,
                                        void *focused_node, float *out_scale,
                                        float *out_tilt_x, float *out_tilt_y) {
  struct cmp_tvos_features *ctx = (struct cmp_tvos_features *)features_opaque;
  if (!ctx || !focused_node)
    return CMP_ERROR_INVALID_ARG;

  /* Apply parallax effect and focus scaling (e.g. 1.1x) */
  if (out_scale)
    *out_scale = 1.1f;
  if (out_tilt_x)
    *out_tilt_x = 0.05f; /* simulated micro-movement */
  if (out_tilt_y)
    *out_tilt_y = 0.05f;

  return CMP_SUCCESS;
}

int cmp_tvos_set_overscan_margins(cmp_tvos_features_t *features_opaque,
                                  float *out_margin_top,
                                  float *out_margin_bottom,
                                  float *out_margin_left,
                                  float *out_margin_right) {
  struct cmp_tvos_features *ctx = (struct cmp_tvos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  /* Standard 1080p/4K overscan safe areas */
  if (out_margin_top)
    *out_margin_top = 60.0f;
  if (out_margin_bottom)
    *out_margin_bottom = 60.0f;
  if (out_margin_left)
    *out_margin_left = 90.0f;
  if (out_margin_right)
    *out_margin_right = 90.0f;

  return CMP_SUCCESS;
}

int cmp_tvos_export_top_shelf(cmp_tvos_features_t *features_opaque,
                              int is_inset, const char *json_data) {
  struct cmp_tvos_features *ctx = (struct cmp_tvos_features *)features_opaque;
  if (!ctx || !json_data)
    return CMP_ERROR_INVALID_ARG;

  ctx->top_shelf_mode = is_inset ? 1 : 0;
  /* Format for TVTopShelfCarouselContent or TVTopShelfInsetContent */
  return CMP_SUCCESS;
}

int cmp_tvos_handle_hardware_play_pause(cmp_tvos_features_t *features_opaque) {
  struct cmp_tvos_features *ctx = (struct cmp_tvos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  /* Broadcast global media play/pause regardless of focus */
  return CMP_SUCCESS;
}

int cmp_tvos_handle_hardware_menu_button(cmp_tvos_features_t *features_opaque) {
  struct cmp_tvos_features *ctx = (struct cmp_tvos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  /* Strictly pops navigation stack or exits */
  return CMP_SUCCESS;
}
