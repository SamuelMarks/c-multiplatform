/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_mipmap_generator {
  int max_levels;
};

int cmp_mipmap_generator_create(cmp_mipmap_generator_t **out_gen) {
  struct cmp_mipmap_generator *gen;
  if (!out_gen)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_mipmap_generator), (void **)&gen) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  gen->max_levels = 8;
  *out_gen = (cmp_mipmap_generator_t *)gen;
  return CMP_SUCCESS;
}

int cmp_mipmap_generator_destroy(cmp_mipmap_generator_t *gen) {
  if (!gen)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(gen);
  return CMP_SUCCESS;
}

int cmp_mipmap_generator_generate(cmp_mipmap_generator_t *gen,
                                  const void *image_data, size_t width,
                                  size_t height, void **out_mipmaps,
                                  size_t *out_levels) {
  size_t levels = 0;
  size_t cur_w = width;
  size_t cur_h = height;
  if (!gen || !image_data || !width || !height || !out_mipmaps || !out_levels)
    return CMP_ERROR_INVALID_ARG;

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

  return CMP_SUCCESS;
}
