/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ipados_multitasking {
  cmp_ipados_features_t *features;
};

/**
 * @brief cmp_ipados_multitasking_create
 *
 * @param out_mt Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ipados_multitasking_create(cmp_ipados_multitasking_t **out_mt) {
  int rc = CMP_SUCCESS;
  cmp_ipados_multitasking_t *mt = NULL;

  if (!out_mt) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ipados_multitasking_create: Invalid argument\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ipados_multitasking_t), (void **)&(mt));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  rc = cmp_ipados_features_create(&mt->features);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(mt);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    LOG_DEBUG("Error in cmp_ipados_multitasking_create: "
              "cmp_ipados_features_create failed\n");

    return rc;
  }

  *out_mt = mt;

  return rc;
}

/**
 * @brief cmp_ipados_multitasking_destroy
 *
 * @param mt Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ipados_multitasking_destroy(cmp_ipados_multitasking_t *mt) {
  int rc = CMP_SUCCESS;

  if (!mt) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ipados_multitasking_destroy: Invalid argument\n");

    return rc;
  }

  if (mt->features) {
    rc = cmp_ipados_features_destroy(mt->features);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_ipados_multitasking_destroy: "
                "cmp_ipados_features_destroy failed\n");
    }
  }
  rc = CMP_FREE(mt);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }

  return rc;
}

/**
 * @brief cmp_ipados_multitasking_request_scene
 *
 * @param mt Parameter description.
 * @param activity_identifier Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ipados_multitasking_request_scene(cmp_ipados_multitasking_t *mt,
                                          const char *activity_identifier) {
  int rc = CMP_SUCCESS;

  if (!mt || !activity_identifier) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ipados_multitasking_request_scene: Invalid argument\n");

    return rc;
  }

  rc = cmp_ipados_request_scene_activation(mt->features, activity_identifier);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ipados_multitasking_request_scene: "
              "cmp_ipados_request_scene_activation failed\n");
  }

  return rc;
}

/**
 * @brief cmp_ipados_multitasking_resolve_layout
 *
 * @param mt Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @param out_horizontal Parameter description.
 * @param out_vertical Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ipados_multitasking_resolve_layout(cmp_ipados_multitasking_t *mt,
                                           float width, float height,
                                           cmp_size_class_t *out_horizontal,
                                           cmp_size_class_t *out_vertical) {
  int rc = CMP_SUCCESS;

  if (!mt || !out_horizontal || !out_vertical) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ipados_multitasking_resolve_layout: Invalid argument\n");

    return rc;
  }

  rc = cmp_ipados_resolve_size_classes(width, height, out_horizontal,
                                       out_vertical);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ipados_multitasking_resolve_layout: "
              "cmp_ipados_resolve_size_classes failed\n");
  }

  return rc;
}
