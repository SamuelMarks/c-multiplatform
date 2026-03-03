#ifndef CMP_AUDIO_H
#define CMP_AUDIO_H

/**
 * @file cmp_audio.h
 * @brief Audio decoding helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"

/**
 * @brief Audio decoder configuration.
 */
typedef struct CMPAudioConfig {
  CMPEnv *env; /**< Optional environment backend used for platform decoding. */
  const CMPAllocator *allocator; /**< Allocator used for decoded PCM (NULL uses
                                   default). */
} CMPAudioConfig;

/**
 * @brief Audio decoder state.
 */
typedef struct CMPAudioDecoder {
  CMPAudio audio;         /**< Audio decoder interface handle. */
  CMPAllocator allocator; /**< Allocator used for decoded PCM. */
  CMPBool ready;          /**< CMP_TRUE when initialized. */
  CMPBool has_backend;    /**< CMP_TRUE when a backend decoder is available. */
} CMPAudioDecoder;

/**
 * @brief Initialize an audio configuration with defaults.
 * @param config Configuration to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_audio_config_init(CMPAudioConfig *config);

/**
 * @brief Initialize an audio decode request with defaults.
 * @param request Request structure to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_audio_request_init(CMPAudioDecodeRequest *request);

/**
 * @brief Initialize an audio decoder using the supplied configuration.
 * @param decoder Decoder instance to initialize.
 * @param config Audio decoder configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_audio_init(CMPAudioDecoder *decoder,
                                    const CMPAudioConfig *config);

/**
 * @brief Shut down an audio decoder and release its state.
 * @param decoder Decoder instance to shut down.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_audio_shutdown(CMPAudioDecoder *decoder);

/**
 * @brief Decode audio from memory using backend or fallback decoders.
 * @param decoder Initialized audio decoder.
 * @param request Decode request parameters.
 * @param out_audio Receives decoded audio data.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_audio_decode(CMPAudioDecoder *decoder,
                                      const CMPAudioDecodeRequest *request,
                                      CMPAudioData *out_audio);

/**
 * @brief Release decoded audio data.
 * @param decoder Initialized audio decoder.
 * @param audio Audio data to free.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_audio_free(CMPAudioDecoder *decoder,
                                    CMPAudioData *audio);

CMP_API int CMP_CALL cmp_audio_test_set_read_u16_fail_after(cmp_u32 call_count);
CMP_API int CMP_CALL cmp_audio_test_set_read_u32_fail_after(cmp_u32 call_count);
CMP_API int CMP_CALL cmp_audio_test_read_u16_le(const cmp_u8 *data,
                                                cmp_usize size,
                                                cmp_usize offset,
                                                cmp_u16 *out_val);
CMP_API int CMP_CALL cmp_audio_test_read_u32_le(const cmp_u8 *data,
                                                cmp_usize size,
                                                cmp_usize offset,
                                                cmp_u32 *out_val);
CMP_API int CMP_CALL cmp_audio_test_decode_wav(
    const CMPAudioDecodeRequest *request, const CMPAllocator *allocator,
    CMPAudioData *out_data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_AUDIO_H */
