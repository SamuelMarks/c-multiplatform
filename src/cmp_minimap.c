/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_minimap {
  char *text;
  float viewport_y;
  float viewport_height;
  float total_height;
  float scroll_ratio; /* Calculated ratio [0.0 - 1.0] */
};

/**
 * @brief cmp_minimap_create
 *
 * @param out_minimap Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_minimap_create(cmp_minimap_t **out_minimap) {
  int rc = CMP_SUCCESS;
  cmp_minimap_t *minimap = NULL;

  if (!out_minimap) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_minimap_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_minimap_t), (void **)&(minimap));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  minimap->text = NULL;
  minimap->viewport_y = 0.0f;
  minimap->viewport_height = 0.0f;
  minimap->total_height = 0.0f;
  minimap->scroll_ratio = 0.0f;

  *out_minimap = minimap;
  return rc;
}

/**
 * @brief cmp_minimap_destroy
 *
 * @param minimap Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_minimap_destroy(cmp_minimap_t *minimap) {
  int rc = CMP_SUCCESS;

  if (!minimap) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_minimap_destroy: Invalid argument\n");
    return rc;
  }

  if (minimap->text) {
    rc = CMP_FREE(minimap->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  rc = CMP_FREE(minimap);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return rc;
}

/**
 * @brief cmp_minimap_set_text
 *
 * @param minimap Parameter description.
 * @param text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_minimap_set_text(cmp_minimap_t *minimap, const char *text) {
  int rc = CMP_SUCCESS;
  size_t len;

  if (!minimap || !text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_minimap_set_text: Invalid argument\n");
    return rc;
  }

  if (minimap->text) {
    rc = CMP_FREE(minimap->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    minimap->text = NULL;
  }

  len = strlen(text);
  rc = CMP_MALLOC(len + 1, (void **)&(minimap->text));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strncpy_s(minimap->text, len + 1, text, _TRUNCATE);
#else
  strncpy(minimap->text, text, len);
#endif
  minimap->text[len] = '\0';

  return rc;
}

/**
 * @brief cmp_minimap_update_viewport
 *
 * @param minimap Parameter description.
 * @param viewport_y Parameter description.
 * @param viewport_height Parameter description.
 * @param total_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_minimap_update_viewport(cmp_minimap_t *minimap, float viewport_y,
                                float viewport_height, float total_height) {
  int rc = CMP_SUCCESS;

  if (!minimap || total_height <= 0.0f || viewport_height <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_minimap_update_viewport: Invalid argument\n");
    return rc;
  }

  minimap->viewport_y = viewport_y;
  minimap->viewport_height = viewport_height;
  minimap->total_height = total_height;

  /* Calculate scroll ratio */
  if (total_height > viewport_height) {
    minimap->scroll_ratio = viewport_y / (total_height - viewport_height);
    if (minimap->scroll_ratio < 0.0f)
      minimap->scroll_ratio = 0.0f;
    if (minimap->scroll_ratio > 1.0f)
      minimap->scroll_ratio = 1.0f;
  } else {
    minimap->scroll_ratio = 0.0f;
  }

  return rc;
}

/**
 * @brief cmp_minimap_compute_layout
 *
 * @param minimap Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_minimap_compute_layout(cmp_minimap_t *minimap) {
  int rc = CMP_SUCCESS;

  if (!minimap) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_minimap_compute_layout: Invalid argument\n");
    return rc;
  }
  /* In a full implementation, this parses minimap->text, mapping line lengths
     to 2-pixel wide dense blocks, and calculates the highlighted region via
     scroll_ratio. */
  return rc;
}
