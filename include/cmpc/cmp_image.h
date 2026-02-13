#ifndef CMP_IMAGE_H
#define CMP_IMAGE_H

/**
 * @file cmp_image.h
 * @brief Image decoding helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"

/**
 * @brief Image decoder configuration.
 */
typedef struct CMPImageConfig {
  CMPEnv *env; /**< Optional environment backend used for platform decoding. */
  const CMPAllocator *allocator; /**< Allocator used for decoded pixels (NULL
                                   uses default). */
} CMPImageConfig;

/**
 * @brief Image decoder state.
 */
typedef struct CMPImageDecoder {
  CMPImage image;         /**< Image decoder interface handle. */
  CMPAllocator allocator; /**< Allocator used for decoded pixels. */
  CMPBool ready;          /**< CMP_TRUE when initialized. */
  CMPBool has_backend;    /**< CMP_TRUE when a backend decoder is available. */
} CMPImageDecoder;

/**
 * @brief Initialize an image configuration with defaults.
 * @param config Configuration to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_image_config_init(CMPImageConfig *config);

/**
 * @brief Initialize an image decode request with defaults.
 * @param request Request structure to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_image_request_init(CMPImageDecodeRequest *request);

/**
 * @brief Initialize an image decoder using the supplied configuration.
 * @param decoder Decoder instance to initialize.
 * @param config Image decoder configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_image_init(CMPImageDecoder *decoder,
                                    const CMPImageConfig *config);

/**
 * @brief Shut down an image decoder and release its state.
 * @param decoder Decoder instance to shut down.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_image_shutdown(CMPImageDecoder *decoder);

/**
 * @brief Decode an image from memory using backend or fallback decoders.
 * @param decoder Initialized image decoder.
 * @param request Decode request parameters.
 * @param out_image Receives decoded image data.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_image_decode(CMPImageDecoder *decoder,
                                      const CMPImageDecodeRequest *request,
                                      CMPImageData *out_image);

/**
 * @brief Release decoded image data.
 * @param decoder Initialized image decoder.
 * @param image Image data to free.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_image_free(CMPImageDecoder *decoder,
                                    CMPImageData *image);

#ifdef CMP_TESTING
CMP_API int CMP_CALL cmp_image_test_mul_overflow(cmp_usize a, cmp_usize b,
                                                 cmp_usize *out_value);
CMP_API int CMP_CALL cmp_image_test_ppm_skip_ws(const cmp_u8 *data,
                                                cmp_usize size,
                                                cmp_usize *offset);
CMP_API int CMP_CALL cmp_image_test_ppm_read_uint(const cmp_u8 *data,
                                                  cmp_usize size,
                                                  cmp_usize *offset,
                                                  cmp_u32 *out_value);
CMP_API int CMP_CALL cmp_image_test_decode_ppm(
    const CMPImageDecodeRequest *request, const CMPAllocator *allocator,
    CMPImageData *out_image);
CMP_API int CMP_CALL cmp_image_test_decode_raw_rgba8(
    const CMPImageDecodeRequest *request, const CMPAllocator *allocator,
    CMPImageData *out_image);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_IMAGE_H */
