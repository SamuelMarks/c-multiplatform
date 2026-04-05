/* clang-format off */
#include "cmp.h"
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

int cmp_minimap_create(cmp_minimap_t **out_minimap) {
  cmp_minimap_t *minimap;
  if (!out_minimap) {
    return CMP_ERROR_INVALID_ARG;
  }

  minimap = (cmp_minimap_t *)malloc(sizeof(cmp_minimap_t));
  if (!minimap) {
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

int cmp_minimap_destroy(cmp_minimap_t *minimap) {
  if (!minimap) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (minimap->text) {
    free(minimap->text);
  }
  free(minimap);
  return CMP_SUCCESS;
}

int cmp_minimap_set_text(cmp_minimap_t *minimap, const char *text) {
  size_t len;
  if (!minimap || !text) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (minimap->text) {
    free(minimap->text);
    minimap->text = NULL;
  }

  len = strlen(text);
  minimap->text = (char *)malloc(len + 1);
  if (!minimap->text) {
    return CMP_ERROR_OOM;
  }

  strncpy(minimap->text, text, len);
  minimap->text[len] = '\0';

  return CMP_SUCCESS;
}

int cmp_minimap_update_viewport(cmp_minimap_t *minimap, float viewport_y,
                                float viewport_height, float total_height) {
  if (!minimap || total_height <= 0.0f || viewport_height <= 0.0f) {
    return CMP_ERROR_INVALID_ARG;
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

  return CMP_SUCCESS;
}

int cmp_minimap_compute_layout(cmp_minimap_t *minimap) {
  if (!minimap) {
    return CMP_ERROR_INVALID_ARG;
  }
  /* In a full implementation, this parses minimap->text, mapping line lengths
     to 2-pixel wide dense blocks, and calculates the highlighted region via
     scroll_ratio. */
  return CMP_SUCCESS;
}
