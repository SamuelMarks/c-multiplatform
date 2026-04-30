/* clang-format off */
#include "cmp_fluent.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_fluent_reveal {
  float pointer_x;
  float pointer_y;
};

/**
 * @brief cmp_fluent_reveal_create
 *
 * @param out_reveal Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_fluent_reveal_create(cmp_fluent_reveal_t **out_reveal) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_fluent_reveal *ctx = NULL;

  if (out_reveal == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_fluent_reveal_create: Invalid argument (out_reveal=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_fluent_reveal), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_fluent_reveal_create: Out of memory: %s\n", err_str);

    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_fluent_reveal));
  *out_reveal = (cmp_fluent_reveal_t *)ctx;
  cmp_log_debug(
      "cmp_fluent_reveal_create: Successfully created Fluent Reveal Context\n");
  return rc;
}

/**
 * @brief cmp_fluent_reveal_destroy
 *
 * @param reveal Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_fluent_reveal_destroy(cmp_fluent_reveal_t *reveal) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_fluent_reveal *ctx = (struct cmp_fluent_reveal *)reveal;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_fluent_reveal_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_fluent_reveal_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_fluent_reveal_destroy: Successfully destroyed Fluent "
                "Reveal Context\n");
  return rc;
}

/**
 * @brief cmp_fluent_reveal_update_pointer
 *
 * @param reveal Parameter description.
 * @param pointer_x Parameter description.
 * @param pointer_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_fluent_reveal_update_pointer(cmp_fluent_reveal_t *reveal,
                                     float pointer_x, float pointer_y) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_fluent_reveal *ctx = (struct cmp_fluent_reveal *)reveal;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_fluent_reveal_update_pointer: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  ctx->pointer_x = pointer_x;
  ctx->pointer_y = pointer_y;
  cmp_log_debug("cmp_fluent_reveal_update_pointer: Tracked pointer location\n");

  return rc;
}

struct cmp_acrylic_noise {
  int width;
  int height;
  unsigned char *pixels;
};

/**
 * @brief cmp_acrylic_noise_create
 *
 * @param width Parameter description.
 * @param height Parameter description.
 * @param out_noise Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_acrylic_noise_create(int width, int height,
                             cmp_acrylic_noise_t **out_noise) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_acrylic_noise *ctx = NULL;

  if (out_noise == NULL || width <= 0 || height <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_acrylic_noise_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_acrylic_noise), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_acrylic_noise_create: Out of memory for context: %s\n",
                  err_str);

    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_acrylic_noise));
  ctx->width = width;
  ctx->height = height;

  rc = CMP_MALLOC((size_t)(width * height), (void **)&ctx->pixels);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    rc = CMP_FREE(ctx);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_acrylic_noise_create: CMP_FREE ctx failed\n");
    }
    cmp_log_debug("cmp_acrylic_noise_create: Out of memory for pixels: %s\n",
                  err_str);
    return CMP_ERROR_OOM;
  }

  /* Fill with random monochrome noise */
  {
    int i;
    for (i = 0; i < width * height; i++) {
      ctx->pixels[i] = (unsigned char)(rand() % 256);
    }
  }

  *out_noise = (cmp_acrylic_noise_t *)ctx;
  cmp_log_debug(
      "cmp_acrylic_noise_create: Successfully created Acrylic noise\n");
  return rc;
}

/**
 * @brief cmp_acrylic_noise_destroy
 *
 * @param noise Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_acrylic_noise_destroy(cmp_acrylic_noise_t *noise) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_acrylic_noise *ctx = (struct cmp_acrylic_noise *)noise;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_acrylic_noise_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (ctx->pixels != NULL) {
    rc = CMP_FREE(ctx->pixels);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_acrylic_noise_destroy: CMP_FREE array failed\n");
    }
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_acrylic_noise_destroy: CMP_FREE context failed\n");
  }

  cmp_log_debug(
      "cmp_acrylic_noise_destroy: Successfully destroyed Acrylic noise\n");
  return rc;
}
