/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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

/**
 * @brief Creates an app region manager.
 *
 * @param out_region Pointer to the newly created region manager.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_app_region_create(cmp_app_region_t **out_region) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_app_region_t *region = NULL;

  if (out_region == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_app_region_create: %s\n", err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_app_region_t), (void **)&region);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_app_region_create CMP_MALLOC: %s\n", err_str);
    return rc;
  }

  memset(region, 0, sizeof(cmp_app_region_t));
  *out_region = region;
  cmp_log_debug(
      "cmp_app_region_create: Successfully created app region manager\n");
  return rc;
}

/**
 * @brief Destroys an app region manager.
 *
 * @param region Pointer to the region manager.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_app_region_destroy(cmp_app_region_t *region) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (region == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_app_region_destroy: %s\n", err_str);
    return rc;
  }

  if (region->rects != NULL) {
    CMP_FREE(region->rects);
  }
  CMP_FREE(region);
  cmp_log_debug(
      "cmp_app_region_destroy: Successfully destroyed app region manager\n");
  return rc;
}

/**
 * @brief Adds a hit-testable rectangle to the app region.
 *
 * @param region Pointer to the region manager.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param width The width.
 * @param height The height.
 * @param type The region type.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_app_region_add_rect(cmp_app_region_t *region, float x, float y,
                            float width, float height,
                            cmp_app_region_type_t type) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t new_capacity;
  cmp_app_region_rect_t *new_rects = NULL;

  if (region == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_app_region_add_rect: %s\n", err_str);
    return rc;
  }

  if (region->count >= region->capacity) {
    new_capacity = region->capacity == 0 ? 8 : region->capacity * 2;
    cmp_log_debug("cmp_app_region_add_rect: Growing capacity to %u\n",
                  (unsigned int)new_capacity);
    rc = CMP_MALLOC(new_capacity * sizeof(cmp_app_region_rect_t),
                    (void **)&new_rects);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_app_region_add_rect CMP_MALLOC: %s\n", err_str);
      return rc;
    }

    if (region->rects != NULL) {
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

  cmp_log_debug("cmp_app_region_add_rect: Added rect (%.2f, %.2f, %.2f, %.2f) "
                "of type %d\n",
                x, y, width, height, (int)type);
  return rc;
}

/**
 * @brief Clears all rectangles from the app region manager.
 *
 * @param region Pointer to the region manager.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_app_region_clear(cmp_app_region_t *region) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (region == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_app_region_clear: %s\n", err_str);
    return rc;
  }
  region->count = 0;
  cmp_log_debug("cmp_app_region_clear: Cleared all rects\n");
  return rc;
}

/**
 * @brief Performs a hit test to determine the region type at specific
 * coordinates.
 *
 * @param region Pointer to the region manager.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param out_type Pointer to store the resulting region type.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_app_region_hit_test(const cmp_app_region_t *region, float x, float y,
                            cmp_app_region_type_t *out_type) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i;
  const cmp_app_region_rect_t *r;

  if (region == NULL || out_type == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_app_region_hit_test: %s\n", err_str);
    return rc;
  }

  *out_type = CMP_APP_REGION_NONE;

  /* Walk backwards to respect painting order (last added = top) */
  for (i = region->count; i > 0; i--) {
    r = &region->rects[i - 1];
    if (x >= r->rect.x && x <= r->rect.x + r->rect.width && y >= r->rect.y &&
        y <= r->rect.y + r->rect.height) {
      *out_type = r->type;
      cmp_log_debug(
          "cmp_app_region_hit_test: Hit found of type %d at (%.2f, %.2f)\n",
          (int)r->type, x, y);
      break;
    }
  }

  if (*out_type == CMP_APP_REGION_NONE) {
    cmp_log_debug("cmp_app_region_hit_test: No hit found at (%.2f, %.2f)\n", x,
                  y);
  }

  return rc;
}
