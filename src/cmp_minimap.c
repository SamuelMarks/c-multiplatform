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
 * @brief Create a minimap context.
 *
 * @param out_minimap Pointer to store the created minimap.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_minimap_create(cmp_minimap_t **out_minimap) {
  int rc;
  cmp_minimap_t *minimap;

  rc = CMP_SUCCESS;
  minimap = NULL;

  if (out_minimap == NULL) {
    LOG_DEBUG("Error in cmp_minimap_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_minimap_t), (void **)&minimap);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_minimap_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  minimap->text = NULL;
  minimap->viewport_y = 0.0f;
  minimap->viewport_height = 0.0f;
  minimap->total_height = 0.0f;
  minimap->scroll_ratio = 0.0f;

  *out_minimap = minimap;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a minimap context.
 *
 * @param minimap The minimap to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_minimap_destroy(cmp_minimap_t *minimap) {
  int rc;

  rc = CMP_SUCCESS;

  if (minimap == NULL) {
    LOG_DEBUG("Error in cmp_minimap_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (minimap->text != NULL) {
    rc = CMP_FREE(minimap->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_minimap_destroy: CMP_FREE failed for text\n");
    }
  }

  rc = CMP_FREE(minimap);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_minimap_destroy: CMP_FREE failed for minimap\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Set the text for the minimap.
 *
 * @param minimap The minimap context.
 * @param text The text to display.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_minimap_set_text(cmp_minimap_t *minimap, const char *text) {
  int rc;
  size_t len;

  rc = CMP_SUCCESS;

  if (minimap == NULL || text == NULL) {
    LOG_DEBUG("Error in cmp_minimap_set_text: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (minimap->text != NULL) {
    rc = CMP_FREE(minimap->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_minimap_set_text: CMP_FREE failed\n");
    }
    minimap->text = NULL;
  }

  len = strlen(text);
  rc = CMP_MALLOC(len + 1, (void **)&(minimap->text));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_minimap_set_text: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  rc = strncpy_s(minimap->text, len + 1, text, _TRUNCATE);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_minimap_set_text: strncpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strncpy(minimap->text, text, len);
#endif
  minimap->text[len] = '\0';

  return CMP_SUCCESS;
}

/**
 * @brief Update the minimap viewport mapping.
 *
 * @param minimap The minimap context.
 * @param viewport_y Viewport Y offset.
 * @param viewport_height Viewport height.
 * @param total_height Total document height.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_minimap_update_viewport(cmp_minimap_t *minimap, float viewport_y,
                                float viewport_height, float total_height) {
  int rc;
  rc = 0;
  if (minimap == NULL || total_height <= 0.0f || viewport_height <= 0.0f) {
    LOG_DEBUG("Error in cmp_minimap_update_viewport: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  minimap->viewport_y = viewport_y;
  minimap->viewport_height = viewport_height;
  minimap->total_height = total_height;

  /* Calculate scroll ratio */
  if (total_height > viewport_height) {
    minimap->scroll_ratio = viewport_y / (total_height - viewport_height);
    if (minimap->scroll_ratio < 0.0f) {
      minimap->scroll_ratio = 0.0f;
    }
    if (minimap->scroll_ratio > 1.0f) {
      minimap->scroll_ratio = 1.0f;
    }
  } else {
    minimap->scroll_ratio = 0.0f;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Compute the layout of the minimap.
 *
 * @param minimap The minimap context.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_minimap_compute_layout(cmp_minimap_t *minimap) {
  int rc;
  rc = 0;
  if (minimap == NULL) {
    LOG_DEBUG("Error in cmp_minimap_compute_layout: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  /* In a full implementation, this parses minimap->text, mapping line lengths
     to 2-pixel wide dense blocks, and calculates the highlighted region via
     scroll_ratio. */
  return CMP_SUCCESS;
}
