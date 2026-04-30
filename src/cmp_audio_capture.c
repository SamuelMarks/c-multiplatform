/* clang-format off */
#include "cmp_audio_capture.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_audio_capture {
  int is_recording;
  unsigned int dummy_pcm_size;
};

/**
 * @brief cmp_audio_capture_create
 *
 * @param out_capture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_capture_create(cmp_audio_capture_t **out_capture) {
  int rc = CMP_SUCCESS;
  cmp_audio_capture_t *cap = NULL;

  if (out_capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "cmp_audio_capture_create: Invalid argument (out_capture=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_audio_capture_t), (void **)&cap);
  if (rc != CMP_SUCCESS || cap == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_audio_capture_create: Out of memory\n");
    return rc;
  }

  cap->is_recording = 0;
  cap->dummy_pcm_size = 0;
  *out_capture = cap;
  return rc;
}

/**
 * @brief cmp_audio_capture_destroy
 *
 * @param capture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_capture_destroy(cmp_audio_capture_t *capture) {
  int rc = CMP_SUCCESS;

  if (capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_audio_capture_destroy: Invalid argument (capture=NULL)\n");
    return rc;
  }

  rc = CMP_FREE(capture);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_audio_capture_destroy: CMP_FREE failed\n");
  }
  return rc;
}

/**
 * @brief cmp_audio_capture_start
 *
 * @param capture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_capture_start(cmp_audio_capture_t *capture) {
  int rc = CMP_SUCCESS;

  if (capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_audio_capture_start: Invalid argument (capture=NULL)\n");
    return rc;
  }

  if (capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("cmp_audio_capture_start: Already recording\n");
    return rc;
  }

  capture->is_recording = 1;
  capture->dummy_pcm_size = 44100; /* Mock 1 second of audio */
  return rc;
}

/**
 * @brief cmp_audio_capture_stop
 *
 * @param capture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_capture_stop(cmp_audio_capture_t *capture) {
  int rc = CMP_SUCCESS;

  if (capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_audio_capture_stop: Invalid argument (capture=NULL)\n");
    return rc;
  }

  if (!capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("cmp_audio_capture_stop: Not recording\n");
    return rc;
  }

  capture->is_recording = 0;
  return rc;
}

/**
 * @brief cmp_audio_capture_get_wav
 *
 * @param capture Parameter description.
 * @param out_wav_data Parameter description.
 * @param out_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_capture_get_wav(cmp_audio_capture_t *capture,
                              unsigned char **out_wav_data,
                              unsigned int *out_size) {
  int rc = CMP_SUCCESS;
  unsigned char *wav = NULL;
  unsigned int total_size;

  if (capture == NULL || out_wav_data == NULL || out_size == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_audio_capture_get_wav: Invalid argument\n");
    return rc;
  }

  if (capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("cmp_audio_capture_get_wav: Must stop recording\n");
    return rc; /* Must stop before getting */
  }

  /* Mock WAV encoding */
  total_size = capture->dummy_pcm_size + 44; /* 44 bytes for WAV header */
  rc = CMP_MALLOC(total_size, (void **)&wav);
  if (rc != CMP_SUCCESS || wav == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_audio_capture_get_wav: Out of memory\n");
    return rc;
  }

  /* Generate a fake WAV header */
  memset(wav, 0, 44);
  memcpy(wav, "RIFF", 4);
  /* ... skipping actual accurate header creation for mock ... */

  *out_wav_data = wav;
  *out_size = total_size;

  return rc;
}

/**
 * @brief cmp_audio_capture_free_wav
 *
 * @param wav_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_capture_free_wav(unsigned char *wav_data) {
  int rc = CMP_SUCCESS;

  if (wav_data == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_audio_capture_free_wav: Invalid argument (wav_data=NULL)\n");
    return rc;
  }

  rc = CMP_FREE(wav_data);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_audio_capture_free_wav: CMP_FREE failed\n");
  }
  return rc;
}
