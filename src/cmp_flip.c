/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_flip {
  cmp_rect_t initial_bounds;
  int has_first;
};

/**
 * @brief cmp_flip_create
 *
 * @param out_flip Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_flip_create(cmp_flip_t **out_flip) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_flip *flip = NULL;

  if (out_flip == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_flip_create: Invalid argument (out_flip=NULL): %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_flip), (void **)&flip);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_flip_create: Out of memory: %s\n", err_str);

    return rc;
  }

  memset(flip, 0, sizeof(struct cmp_flip));

  *out_flip = (cmp_flip_t *)flip;
  cmp_log_debug("cmp_flip_create: Successfully created FLIP context\n");
  return rc;
}

/**
 * @brief cmp_flip_destroy
 *
 * @param flip Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_flip_destroy(cmp_flip_t *flip) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_flip *internal_flip = (struct cmp_flip *)flip;

  if (internal_flip == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_flip_destroy: Invalid argument (flip=NULL): %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_FREE(internal_flip);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_flip_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_flip_destroy: Successfully destroyed FLIP context\n");
  return rc;
}

/**
 * @brief cmp_flip_first
 *
 * @param flip Parameter description.
 * @param initial_bounds Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_flip_first(cmp_flip_t *flip, const cmp_rect_t *initial_bounds) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_flip *internal_flip = (struct cmp_flip *)flip;

  if (internal_flip == NULL || initial_bounds == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_flip_first: Invalid argument\n");

    return rc;
  }

  internal_flip->initial_bounds = *initial_bounds;
  internal_flip->has_first = 1;

  cmp_log_debug("cmp_flip_first: Cached initial rect\n");

  return rc;
}

/**
 * @brief cmp_flip_last_and_invert
 *
 * @param flip Parameter description.
 * @param final_bounds Parameter description.
 * @param out_translate_x Parameter description.
 * @param out_translate_y Parameter description.
 * @param out_scale_x Parameter description.
 * @param out_scale_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_flip_last_and_invert(cmp_flip_t *flip, const cmp_rect_t *final_bounds,
                             float *out_translate_x, float *out_translate_y,
                             float *out_scale_x, float *out_scale_y) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_flip *internal_flip = (struct cmp_flip *)flip;

  if (internal_flip == NULL || final_bounds == NULL ||
      out_translate_x == NULL || out_translate_y == NULL ||
      out_scale_x == NULL || out_scale_y == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_flip_last_and_invert: Invalid argument\n");

    return rc;
  }

  if (internal_flip->has_first == 0) {
    rc = CMP_ERROR_INVALID_STATE;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_flip_last_and_invert: First bounds not set: %s\n",
                  err_str);

    return rc;
  }

  /* Invert */
  *out_translate_x = internal_flip->initial_bounds.x - final_bounds->x;
  *out_translate_y = internal_flip->initial_bounds.y - final_bounds->y;

  if (final_bounds->width != 0.0f) {
    *out_scale_x = internal_flip->initial_bounds.width / final_bounds->width;
  } else {
    *out_scale_x = 1.0f;
  }

  if (final_bounds->height != 0.0f) {
    *out_scale_y = internal_flip->initial_bounds.height / final_bounds->height;
  } else {
    *out_scale_y = 1.0f;
  }

  cmp_log_debug("cmp_flip_last_and_invert: Computed delta inversions\n");

  return rc;
}
