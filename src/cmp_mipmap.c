/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_mipmap_generator {
  int max_levels;
};

/**
 * @brief cmp_mipmap_generator_create
 *
 * @param out_gen Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mipmap_generator_create(cmp_mipmap_generator_t **out_gen) {
  int rc = CMP_SUCCESS;
  struct cmp_mipmap_generator *gen = NULL;

  if (!out_gen) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mipmap_generator_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_mipmap_generator), (void **)&gen);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_mipmap_generator_create: Out of memory\n");
    return rc;
  }

  gen->max_levels = 8;
  *out_gen = (cmp_mipmap_generator_t *)gen;
  return rc;
}

/**
 * @brief cmp_mipmap_generator_destroy
 *
 * @param gen Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mipmap_generator_destroy(cmp_mipmap_generator_t *gen) {
  int rc = CMP_SUCCESS;

  if (!gen) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mipmap_generator_destroy: Invalid argument\n");
    return rc;
  }
  CMP_FREE(gen);
  return rc;
}

/**
 * @brief cmp_mipmap_generator_generate
 *
 * @param gen Parameter description.
 * @param image_data Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @param out_mipmaps Parameter description.
 * @param out_levels Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mipmap_generator_generate(cmp_mipmap_generator_t *gen,
                                  const void *image_data, size_t width,
                                  size_t height, void **out_mipmaps,
                                  size_t *out_levels) {
  int rc = CMP_SUCCESS;
  size_t levels = 0;
  size_t cur_w = width;
  size_t cur_h = height;

  if (!gen || !image_data || !width || !height || !out_mipmaps || !out_levels) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mipmap_generator_generate: Invalid argument\n");
    return rc;
  }

  while (cur_w > 1 || cur_h > 1) {
    if (cur_w > 1)
      cur_w /= 2;
    if (cur_h > 1)
      cur_h /= 2;
    levels++;
  }

  *out_levels = levels;
  /* Mock implementation */
  if (levels > 0 && out_mipmaps) {
    out_mipmaps[0] = NULL; /* Simulate mipmap pointers */
  }

  return rc;
}
