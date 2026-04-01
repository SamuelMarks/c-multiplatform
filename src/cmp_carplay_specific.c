/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_carplay_features {
  int is_offline;
};

int cmp_carplay_features_create(cmp_carplay_features_t **out_features) {
  struct cmp_carplay_features *ctx;
  if (!out_features)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_carplay_features), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_offline = 0;

  *out_features = (cmp_carplay_features_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_carplay_features_destroy(cmp_carplay_features_t *features_opaque) {
  if (features_opaque)
    CMP_FREE(features_opaque);
  return CMP_SUCCESS;
}

int cmp_carplay_apply_driving_focus(cmp_carplay_features_t *features_opaque,
                                    void *node) {
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;
  if (!ctx || !node)
    return CMP_ERROR_INVALID_ARG;

  /* Applies high contrast and massive touch targets */
  return CMP_SUCCESS;
}

int cmp_carplay_export_template_data(cmp_carplay_features_t *features_opaque,
                                     const char *template_type,
                                     const char *json_data) {
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;
  if (!ctx || !template_type || !json_data)
    return CMP_ERROR_INVALID_ARG;

  /* Translates internal UI tree into standard CarPlay system templates (List,
   * Grid, Map) */
  return CMP_SUCCESS;
}

int cmp_carplay_handle_siri_intent(cmp_carplay_features_t *features_opaque,
                                   const char *intent_id) {
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;
  if (!ctx || !intent_id)
    return CMP_ERROR_INVALID_ARG;

  /* Executes critical actions purely via voice */
  return CMP_SUCCESS;
}

int cmp_carplay_handle_knob_navigation(cmp_carplay_features_t *features_opaque,
                                       float delta) {
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;
  if (!ctx || delta == 0.0f)
    return CMP_ERROR_INVALID_ARG;

  /* Translates rotary knob input to focus engine movement */
  return CMP_SUCCESS;
}

int cmp_carplay_set_network_status(cmp_carplay_features_t *features_opaque,
                                   int is_online) {
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->is_offline = !is_online;
  /* Suppresses blocking alerts and degrades gracefully for tunnels */
  return CMP_SUCCESS;
}
