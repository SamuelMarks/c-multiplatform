/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

typedef struct cmp_app_region_rect {
  cmp_rect_t rect;
  cmp_app_region_type_t type;
} cmp_app_region_rect_t;

struct cmp_app_region {
  cmp_app_region_rect_t *rects;
  size_t count;
  size_t capacity;
};

int cmp_app_region_create(cmp_app_region_t **out_region) {
  int rc = CMP_SUCCESS;
  cmp_app_region_t *region = NULL;

  if (!out_region) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(
        stderr,
        "Error in cmp_app_region_create: Invalid argument (out_region=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_app_region_t), (void **)&region);
  if (rc != CMP_SUCCESS) {
    fprintf(stderr, "Error in cmp_app_region_create: Out of memory\n");
    return rc;
  }

  memset(region, 0, sizeof(cmp_app_region_t));
  *out_region = region;
  return rc;
}

int cmp_app_region_destroy(cmp_app_region_t *region) {
  int rc = CMP_SUCCESS;

  if (!region) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(
        stderr,
        "Error in cmp_app_region_destroy: Invalid argument (region=NULL)\n");
    return rc;
  }

  if (region->rects) {
    CMP_FREE(region->rects);
  }
  CMP_FREE(region);
  return rc;
}

int cmp_app_region_add_rect(cmp_app_region_t *region, float x, float y,
                            float width, float height,
                            cmp_app_region_type_t type) {
  int rc = CMP_SUCCESS;
  size_t new_capacity;
  cmp_app_region_rect_t *new_rects = NULL;

  if (!region) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(
        stderr,
        "Error in cmp_app_region_add_rect: Invalid argument (region=NULL)\n");
    return rc;
  }

  if (region->count >= region->capacity) {
    new_capacity = region->capacity == 0 ? 8 : region->capacity * 2;
    rc = CMP_MALLOC(new_capacity * sizeof(cmp_app_region_rect_t),
                    (void **)&new_rects);
    if (rc != CMP_SUCCESS) {
      fprintf(stderr, "Error in cmp_app_region_add_rect: Out of memory\n");
      return rc;
    }

    if (region->rects) {
      memcpy(new_rects, region->rects,
             region->count * sizeof(cmp_app_region_rect_t));
      CMP_FREE(region->rects);
    }
    region->rects = new_rects;
    region->capacity = new_capacity;
  }

  region->rects[region->count].rect.x = x;
  region->rects[region->count].rect.y = y;
  region->rects[region->count].rect.width = width;
  region->rects[region->count].rect.height = height;
  region->rects[region->count].type = type;
  region->count++;

  return rc;
}

int cmp_app_region_clear(cmp_app_region_t *region) {
  int rc = CMP_SUCCESS;

  if (!region) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_app_region_clear: Invalid argument (region=NULL)\n");
    return rc;
  }
  region->count = 0;
  return rc;
}

int cmp_app_region_hit_test(const cmp_app_region_t *region, float x, float y,
                            cmp_app_region_type_t *out_type) {
  int rc = CMP_SUCCESS;
  size_t i;
  const cmp_app_region_rect_t *r;

  if (!region || !out_type) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_app_region_hit_test: Invalid argument\n");
    return rc;
  }

  *out_type = CMP_APP_REGION_NONE;

  /* Walk backwards to respect painting order (last added = top) */
  for (i = region->count; i > 0; i--) {
    r = &region->rects[i - 1];
    if (x >= r->rect.x && x <= r->rect.x + r->rect.width && y >= r->rect.y &&
        y <= r->rect.y + r->rect.height) {
      *out_type = r->type;
      break;
    }
  }

  return rc;
}
