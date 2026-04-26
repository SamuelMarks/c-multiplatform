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

  if (!out_capture) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_audio_capture_create: Invalid argument "
                    "(out_capture=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_audio_capture_t), (void **)&cap);
  if (rc != CMP_SUCCESS) {
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

  if (!capture) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_audio_capture_destroy: Invalid argument "
                    "(capture=NULL)\n");
    return rc;
  }
  rc = CMP_FREE(capture);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_audio_capture_destroy: CMP_FREE failed\n");
    return rc;
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

  if (!capture) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG(
          "cmp_audio_capture_start: Invalid argument (capture=NULL): %s\n",
          err_str);
    }
    return rc;
  }
  if (capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_audio_capture_start: Already recording: %s\n", err_str);
    }
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

  if (!capture) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_audio_capture_stop: Invalid argument (capture=NULL): %s\n",
                err_str);
    }
    return rc;
  }
  if (!capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_audio_capture_stop: Not recording: %s\n", err_str);
    }
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

  if (!capture || !out_wav_data || !out_size) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_audio_capture_get_wav: Invalid argument: %s\n", err_str);
    }
    return rc;
  }
  if (capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_audio_capture_get_wav: Must stop recording : %s\n",
                err_str);
    }
    return rc; /* Must stop before getting */
  }

  /* Mock WAV encoding */
  total_size = capture->dummy_pcm_size + 44; /* 44 bytes for WAV header */
  rc = CMP_MALLOC(total_size, (void **)&wav);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_audio_capture_get_wav: Out of memory: %s\n", err_str);
    }
    return rc;
  }

  memset(wav, 0, total_size);
  /* Mock RIFF header bytes */
  wav[0] = 'R';
  wav[1] = 'I';
  wav[2] = 'F';
  wav[3] = 'F';
  wav[8] = 'W';
  wav[9] = 'A';
  wav[10] = 'V';
  wav[11] = 'E';

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
  if (wav_data) {
    rc = CMP_FREE(wav_data);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_audio_capture_free_wav: CMP_FREE failed\n");
    }
  }
  return rc;
}
