/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_hw_video_decoder {
  int active;
};

/**
 * @brief cmp_hw_video_decoder_create
 *
 * @param out_decoder Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_hw_video_decoder_create(cmp_hw_video_decoder_t **out_decoder) {
  int rc;
  rc = 0;
  struct cmp_hw_video_decoder *decoder;
  if (!out_decoder)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_hw_video_decoder), (void **)&decoder) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  decoder->active = 1;
  *out_decoder = (cmp_hw_video_decoder_t *)decoder;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_hw_video_decoder_destroy
 *
 * @param decoder Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_hw_video_decoder_destroy(cmp_hw_video_decoder_t *decoder) {
  int rc;
  rc = 0;
  if (!decoder)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(decoder);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_hw_video_decoder_decode_frame
 *
 * @param decoder Parameter description.
 * @param data Parameter description.
 * @param size Parameter description.
 * @param out_nv12_buffer Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_hw_video_decoder_decode_frame(cmp_hw_video_decoder_t *decoder,
                                      const void *data, size_t size,
                                      void *out_nv12_buffer) {
  int rc;
  rc = 0;
  if (!decoder || !data || !size || !out_nv12_buffer)
    return CMP_ERROR_INVALID_ARG;
  memset(out_nv12_buffer, 0, size);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
