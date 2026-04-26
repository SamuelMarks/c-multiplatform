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
  struct cmp_fluent_reveal *ctx = NULL;

  if (!out_reveal) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_fluent_reveal_create: Invalid argument "
              "(out_reveal=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_fluent_reveal), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_fluent_reveal_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_fluent_reveal));
  *out_reveal = (cmp_fluent_reveal_t *)ctx;
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
  struct cmp_fluent_reveal *ctx = (struct cmp_fluent_reveal *)reveal;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_fluent_reveal_destroy: Invalid argument (reveal=NULL)\n");
    return rc;
  }

  CMP_FREE(ctx);
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
  struct cmp_fluent_reveal *ctx = (struct cmp_fluent_reveal *)reveal;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_fluent_reveal_update_pointer: Invalid argument\n");
    return rc;
  }

  ctx->pointer_x = pointer_x;
  ctx->pointer_y = pointer_y;
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
  struct cmp_acrylic_noise *ctx = NULL;

  if (!out_noise || width <= 0 || height <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_acrylic_noise_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_acrylic_noise), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_acrylic_noise_create: Out of memory for context\n");
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_acrylic_noise));
  ctx->width = width;
  ctx->height = height;

  rc = CMP_MALLOC((size_t)(width * height), (void **)&ctx->pixels);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(ctx);
    LOG_DEBUG("Error in cmp_acrylic_noise_create: Out of memory for pixels\n");
    return rc;
  }

  /* Fill with random monochrome noise */
  {
    int i;
    for (i = 0; i < width * height; i++) {
      ctx->pixels[i] = (unsigned char)(rand() % 256);
    }
  }

  *out_noise = (cmp_acrylic_noise_t *)ctx;
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
  struct cmp_acrylic_noise *ctx = (struct cmp_acrylic_noise *)noise;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_acrylic_noise_destroy: Invalid argument (noise=NULL)\n");
    return rc;
  }

  if (ctx->pixels) {
    CMP_FREE(ctx->pixels);
  }
  CMP_FREE(ctx);

  return rc;
}