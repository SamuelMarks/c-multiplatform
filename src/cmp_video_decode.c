/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_hw_video_decoder {
    int active;
};

int cmp_hw_video_decoder_create(cmp_hw_video_decoder_t **out_decoder) {
    struct cmp_hw_video_decoder *decoder;
    if (!out_decoder) return CMP_ERROR_INVALID_ARG;
    if (CMP_MALLOC(sizeof(struct cmp_hw_video_decoder), (void **)&decoder) != CMP_SUCCESS)
        return CMP_ERROR_OOM;
    decoder->active = 1;
    *out_decoder = (cmp_hw_video_decoder_t *)decoder;
    return CMP_SUCCESS;
}

int cmp_hw_video_decoder_destroy(cmp_hw_video_decoder_t *decoder) {
    if (!decoder) return CMP_ERROR_INVALID_ARG;
    CMP_FREE(decoder);
    return CMP_SUCCESS;
}

int cmp_hw_video_decoder_decode_frame(cmp_hw_video_decoder_t *decoder, const void *data, size_t size, void *out_nv12_buffer) {
    if (!decoder || !data || !size || !out_nv12_buffer) return CMP_ERROR_INVALID_ARG;
    memset(out_nv12_buffer, 0, size);
    return CMP_SUCCESS;
}
