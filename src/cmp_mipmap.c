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
 * @brief Create a mipmap generator.
 *
 * @param out_gen Pointer to store the created generator.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mipmap_generator_create(cmp_mipmap_generator_t **out_gen) {
  int rc;
  struct cmp_mipmap_generator *gen;

  rc = CMP_SUCCESS;
  gen = NULL;

  if (out_gen == NULL) {
    LOG_DEBUG("Error in cmp_mipmap_generator_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_mipmap_generator), (void **)&gen);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_mipmap_generator_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  gen->max_levels = 8;
  *out_gen = (cmp_mipmap_generator_t *)gen;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a mipmap generator.
 *
 * @param gen The generator to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mipmap_generator_destroy(cmp_mipmap_generator_t *gen) {
  int rc;

  rc = CMP_SUCCESS;

  if (gen == NULL) {
    LOG_DEBUG("Error in cmp_mipmap_generator_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(gen);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_mipmap_generator_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Generate mipmaps.
 *
 * @param gen The generator context.
 * @param image_data Raw image data.
 * @param width The image width.
 * @param height The image height.
 * @param out_mipmaps Output array of mipmap pointers.
 * @param out_levels Output number of levels generated.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mipmap_generator_generate(cmp_mipmap_generator_t *gen,
                                  const void *image_data, size_t width,
                                  size_t height, void **out_mipmaps,
                                  size_t *out_levels) {
  int rc;
  rc = 0;
  size_t levels;
  size_t cur_w;
  size_t cur_h;

  levels = 0;
  cur_w = width;
  cur_h = height;

  if (gen == NULL || image_data == NULL || width == 0 || height == 0 ||
      out_mipmaps == NULL || out_levels == NULL) {
    LOG_DEBUG("Error in cmp_mipmap_generator_generate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  while (cur_w > 1 || cur_h > 1) {
    if (cur_w > 1) {
      cur_w /= 2;
    }
    if (cur_h > 1) {
      cur_h /= 2;
    }
    levels++;
  }

  *out_levels = levels;
  /* Mock implementation */
  if (levels > 0 && out_mipmaps != NULL) {
    out_mipmaps[0] = NULL; /* Simulate mipmap pointers */
  }
  return CMP_SUCCESS;
}
