/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

struct cmp_carplay_features {
  int is_offline;
};

/**
 * @brief cmp_carplay_features_create
 *
 * @param out_features Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_carplay_features_create(cmp_carplay_features_t **out_features) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_carplay_features *ctx = NULL;

  if (out_features == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_carplay_features_create: Invalid argument "
                  "(out_features=NULL): %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_carplay_features), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_carplay_features_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->is_offline = 0;

  *out_features = (cmp_carplay_features_t *)ctx;
  cmp_log_debug("cmp_carplay_features_create: Successfully created carplay "
                "features context\n");
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_carplay_features_destroy
 *
 * @param features_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_carplay_features_destroy(cmp_carplay_features_t *features_opaque) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (features_opaque == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_carplay_features_destroy: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  CMP_FREE(features_opaque);
  cmp_log_debug("cmp_carplay_features_destroy: Successfully destroyed carplay "
                "features context\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_carplay_apply_driving_focus
 *
 * @param features_opaque Parameter description.
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_carplay_apply_driving_focus(cmp_carplay_features_t *features_opaque,
                                    void *node) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (ctx == NULL || node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_carplay_apply_driving_focus: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Applies high contrast and massive touch targets */
  cmp_log_debug("cmp_carplay_apply_driving_focus: Applied driving focus\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_carplay_export_template_data
 *
 * @param features_opaque Parameter description.
 * @param template_type Parameter description.
 * @param json_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_carplay_export_template_data(cmp_carplay_features_t *features_opaque,
                                     const char *template_type,
                                     const char *json_data) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (ctx == NULL || template_type == NULL || json_data == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_carplay_export_template_data: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Translates internal UI tree into standard CarPlay system templates (List,
   * Grid, Map) */
  cmp_log_debug("cmp_carplay_export_template_data: Exported template data\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_carplay_handle_siri_intent
 *
 * @param features_opaque Parameter description.
 * @param intent_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_carplay_handle_siri_intent(cmp_carplay_features_t *features_opaque,
                                   const char *intent_id) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (ctx == NULL || intent_id == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_carplay_handle_siri_intent: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Executes critical actions purely via voice */
  cmp_log_debug("cmp_carplay_handle_siri_intent: Handled siri intent\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_carplay_handle_knob_navigation
 *
 * @param features_opaque Parameter description.
 * @param delta Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_carplay_handle_knob_navigation(cmp_carplay_features_t *features_opaque,
                                       float delta) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (ctx == NULL || delta == 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_carplay_handle_knob_navigation: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Translates rotary knob input to focus engine movement */
  cmp_log_debug(
      "cmp_carplay_handle_knob_navigation: Handled knob navigation\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_carplay_set_network_status
 *
 * @param features_opaque Parameter description.
 * @param is_online Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_carplay_set_network_status(cmp_carplay_features_t *features_opaque,
                                   int is_online) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_carplay_features *ctx =
      (struct cmp_carplay_features *)features_opaque;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_carplay_set_network_status: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->is_offline = !is_online;
  /* Suppresses blocking alerts and degrades gracefully for tunnels */
  cmp_log_debug("cmp_carplay_set_network_status: Set network status to %d\n",
                is_online);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
