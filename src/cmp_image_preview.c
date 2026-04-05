/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_image_preview {
  int flags;
};

int cmp_image_preview_create(cmp_image_preview_t **out_preview) {
  cmp_image_preview_t *preview;
  if (!out_preview) {
    return CMP_ERROR_INVALID_ARG;
  }

  preview = (cmp_image_preview_t *)malloc(sizeof(cmp_image_preview_t));
  if (!preview) {
    return CMP_ERROR_OOM;
  }

  preview->flags = 0;
  *out_preview = preview;
  return CMP_SUCCESS;
}

int cmp_image_preview_destroy(cmp_image_preview_t *preview) {
  if (!preview) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(preview);
  return CMP_SUCCESS;
}

int cmp_image_preview_load_base64(cmp_image_preview_t *preview,
                                  const char *base64_data,
                                  unsigned char **out_raw_pixels,
                                  int *out_width, int *out_height) {
  unsigned char *pixels;

  if (!preview || !base64_data || !out_raw_pixels || !out_width ||
      !out_height) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Stub: Decode base64 to RGBA logic. For now, generate a tiny red 4x4 square
   */
  *out_width = 4;
  *out_height = 4;
  pixels = (unsigned char *)malloc(16 * 4); /* 16 pixels * 4 bytes/pixel */
  if (!pixels) {
    return CMP_ERROR_OOM;
  }

  {
    int i;
    for (i = 0; i < 16; i++) {
      pixels[i * 4 + 0] = 255; /* R */
      pixels[i * 4 + 1] = 0;   /* G */
      pixels[i * 4 + 2] = 0;   /* B */
      pixels[i * 4 + 3] = 255; /* A */
    }
  }

  *out_raw_pixels = pixels;
  return CMP_SUCCESS;
}

int cmp_image_preview_free_pixels(unsigned char *pixels) {
  if (pixels) {
    free(pixels);
  }
  return CMP_SUCCESS;
}
