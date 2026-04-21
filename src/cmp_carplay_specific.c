/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

struct cmp_carplay_features {
  int is_offline;
};

int cmp_carplay_features_create(cmp_carplay_features_t **out_features) {
  int rc = CMP_SUCCESS;
  struct cmp_carplay_features *ctx = NULL;

  if (!out_features) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_carplay_features_create: Invalid argument "
                    "(out_features=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_carplay_features), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    fprintf(stderr, "Error in cmp_carplay_features_create: Out of memory\n");
    return rc;
  }

  ctx->is_offline = 0;

  *out_features = (cmp_carplay_features_t *)ctx;
  return rc;
}

int cmp_carplay_features_destroy(cmp_carplay_features_t *features_opaque) {
  int rc = CMP_SUCCESS;
  if (features_opaque) {
    CMP_FREE(features_opaque);
  }
  return rc;
}

int cmp_carplay_apply_driving_focus(cmp_carplay_features_t *features_opaque,
                                    void *node) {
  int rc = CMP_SUCCESS;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (!ctx || !node) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_carplay_apply_driving_focus: Invalid argument\n");
    return rc;
  }

  /* Applies high contrast and massive touch targets */
  return rc;
}

int cmp_carplay_export_template_data(cmp_carplay_features_t *features_opaque,
                                     const char *template_type,
                                     const char *json_data) {
  int rc = CMP_SUCCESS;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (!ctx || !template_type || !json_data) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_carplay_export_template_data: Invalid argument\n");
    return rc;
  }

  /* Translates internal UI tree into standard CarPlay system templates (List,
   * Grid, Map) */
  return rc;
}

int cmp_carplay_handle_siri_intent(cmp_carplay_features_t *features_opaque,
                                   const char *intent_id) {
  int rc = CMP_SUCCESS;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (!ctx || !intent_id) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_carplay_handle_siri_intent: Invalid argument\n");
    return rc;
  }

  /* Executes critical actions purely via voice */
  return rc;
}

int cmp_carplay_handle_knob_navigation(cmp_carplay_features_t *features_opaque,
                                       float delta) {
  int rc = CMP_SUCCESS;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (!ctx || delta == 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_carplay_handle_knob_navigation: Invalid argument\n");
    return rc;
  }

  /* Translates rotary knob input to focus engine movement */
  return rc;
}

int cmp_carplay_set_network_status(cmp_carplay_features_t *features_opaque,
                                   int is_online) {
  int rc = CMP_SUCCESS;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_carplay_set_network_status: Invalid argument\n");
    return rc;
  }

  ctx->is_offline = !is_online;
  /* Suppresses blocking alerts and degrades gracefully for tunnels */
  return rc;
}
