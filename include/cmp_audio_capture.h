#ifndef CMP_AUDIO_CAPTURE_H
#define CMP_AUDIO_CAPTURE_H

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file cmp_audio_capture.h
 * @brief OS Audio Capture APIs (WASAPI, ALSA, CoreAudio) and format encoding.
 */

typedef struct cmp_audio_capture cmp_audio_capture_t;

/**
 * @brief Creates an audio capture instance.
 * @param out_capture Pointer to receive the created instance.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_audio_capture_create(cmp_audio_capture_t **out_capture);

/**
 * @brief Destroys an audio capture instance.
 * @param capture The instance to destroy.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_audio_capture_destroy(cmp_audio_capture_t *capture);

/**
 * @brief Starts recording audio.
 * @param capture The capture instance.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_audio_capture_start(cmp_audio_capture_t *capture);

/**
 * @brief Stops recording audio.
 * @param capture The capture instance.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_audio_capture_stop(cmp_audio_capture_t *capture);

/**
 * @brief Retrieves recorded PCM data encoded to WAV.
 * @param capture The capture instance.
 * @param out_wav_data Pointer to receive the WAV data buffer. Must be freed.
 * @param out_size Pointer to receive the size of the WAV data.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_audio_capture_get_wav(cmp_audio_capture_t *capture,
                                      unsigned char **out_wav_data,
                                      unsigned int *out_size);

/**
 * @brief Frees the allocated WAV data buffer.
 * @param wav_data The buffer to free.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_audio_capture_free_wav(unsigned char *wav_data);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_AUDIO_CAPTURE_H */
