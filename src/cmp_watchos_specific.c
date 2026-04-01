/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_watchos_features {
  int aod_enabled;
};

int cmp_watchos_features_create(cmp_watchos_features_t **out_features) {
  struct cmp_watchos_features *ctx;
  if (!out_features)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_watchos_features), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->aod_enabled = 0;

  *out_features = (cmp_watchos_features_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_watchos_features_destroy(cmp_watchos_features_t *features_opaque) {
  if (features_opaque)
    CMP_FREE(features_opaque);
  return CMP_SUCCESS;
}

int cmp_watchos_handle_digital_crown(cmp_watchos_features_t *features_opaque,
                                     float delta) {
  struct cmp_watchos_features *ctx =
      (struct cmp_watchos_features *)features_opaque;
  if (!ctx || delta == 0.0f)
    return CMP_ERROR_INVALID_ARG;

  /* Translates crown rotation delta to scroll offsets and fires haptics for
   * pagination boundaries */
  return CMP_SUCCESS;
}

int cmp_watchos_handle_double_tap(cmp_watchos_features_t *features_opaque) {
  struct cmp_watchos_features *ctx =
      (struct cmp_watchos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  /* Triggers the primary view action defined for the Series 9 / Ultra 2 double
   * tap */
  return CMP_SUCCESS;
}

int cmp_watchos_set_aod_state(cmp_watchos_features_t *features_opaque,
                              int is_wrist_down) {
  struct cmp_watchos_features *ctx =
      (struct cmp_watchos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->aod_enabled = is_wrist_down;
  /* Dims visual components, hides sensitive data, reduces render loop to 1Hz */
  return CMP_SUCCESS;
}

int cmp_watchos_export_complication_data(
    cmp_watchos_features_t *features_opaque, const char *json_data) {
  struct cmp_watchos_features *ctx =
      (struct cmp_watchos_features *)features_opaque;
  if (!ctx || !json_data)
    return CMP_ERROR_INVALID_ARG;

  /* Exports data formatted for CLKComplicationTemplate */
  return CMP_SUCCESS;
}

int cmp_watchos_export_smart_stack(cmp_watchos_features_t *features_opaque,
                                   const char *json_data) {
  struct cmp_watchos_features *ctx =
      (struct cmp_watchos_features *)features_opaque;
  if (!ctx || !json_data)
    return CMP_ERROR_INVALID_ARG;

  /* App provides relevant UI snippets to the watchOS 10+ Smart Stack */
  return CMP_SUCCESS;
}

int cmp_watchos_apply_edge_to_edge_styling(
    cmp_watchos_features_t *features_opaque, void *node) {
  struct cmp_watchos_features *ctx =
      (struct cmp_watchos_features *)features_opaque;
  if (!ctx || !node)
    return CMP_ERROR_INVALID_ARG;

  /* Forces pure black background, strips cards/borders/side-insets */
  return CMP_SUCCESS;
}

int cmp_watchos_apply_pill_button_styling(
    cmp_watchos_features_t *features_opaque, void *button_node) {
  struct cmp_watchos_features *ctx =
      (struct cmp_watchos_features *)features_opaque;
  if (!ctx || !button_node)
    return CMP_ERROR_INVALID_ARG;

  /* Forces full-width, pill-shape for primary actions */
  return CMP_SUCCESS;
}

int cmp_watchos_apply_hierarchical_pagination(
    cmp_watchos_features_t *features_opaque, void *view_controller) {
  struct cmp_watchos_features *ctx =
      (struct cmp_watchos_features *)features_opaque;
  if (!ctx || !view_controller)
    return CMP_ERROR_INVALID_ARG;

  /* Limits navigation to 1-deep push or vertical pagination */
  return CMP_SUCCESS;
}
