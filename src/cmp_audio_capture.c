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
  int err_rc;
  const char *err_str;
  cmp_audio_capture_t *cap = NULL;

  if (out_capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_audio_capture_create: Invalid argument (out_capture=NULL): %s\n",
        err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_audio_capture_t), (void **)&cap);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_audio_capture_create: Out of memory: %s\n", err_str);
    return rc;
  }

  cap->is_recording = 0;
  cap->dummy_pcm_size = 0;
  *out_capture = cap;
  cmp_log_debug(
      "cmp_audio_capture_create: Successfully created audio capture context\n");
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
  int err_rc;
  const char *err_str;

  if (capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_audio_capture_destroy: Invalid argument (capture=NULL): %s\n",
        err_str);
    return rc;
  }

  CMP_FREE(capture);
  cmp_log_debug("cmp_audio_capture_destroy: Successfully destroyed audio "
                "capture context\n");
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
  int err_rc;
  const char *err_str;

  if (capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_audio_capture_start: Invalid argument (capture=NULL): %s\n",
        err_str);
    return rc;
  }

  if (capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_audio_capture_start: Already recording: %s\n", err_str);
    return rc;
  }

  capture->is_recording = 1;
  capture->dummy_pcm_size = 44100; /* Mock 1 second of audio */
  cmp_log_debug("cmp_audio_capture_start: Started audio capture\n");
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
  int err_rc;
  const char *err_str;

  if (capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_audio_capture_stop: Invalid argument (capture=NULL): %s\n",
        err_str);
    return rc;
  }

  if (!capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_audio_capture_stop: Not recording: %s\n", err_str);
    return rc;
  }

  capture->is_recording = 0;
  cmp_log_debug("cmp_audio_capture_stop: Stopped audio capture\n");
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
  int err_rc;
  const char *err_str;
  unsigned char *wav = NULL;
  unsigned int total_size;

  if (capture == NULL || out_wav_data == NULL || out_size == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_audio_capture_get_wav: Invalid argument: %s\n", err_str);
    return rc;
  }

  if (capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_audio_capture_get_wav: Must stop recording: %s\n",
                  err_str);
    return rc; /* Must stop before getting */
  }

  /* Mock WAV encoding */
  total_size = capture->dummy_pcm_size + 44; /* 44 bytes for WAV header */
  rc = CMP_MALLOC(total_size, (void **)&wav);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_audio_capture_get_wav: Out of memory: %s\n", err_str);
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
  cmp_log_debug("cmp_audio_capture_get_wav: Retrieved WAV data of size %u\n",
                total_size);
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
  if (wav_data != NULL) {
    CMP_FREE(wav_data);
    cmp_log_debug("cmp_audio_capture_free_wav: Freed WAV data\n");
  } else {
    cmp_log_debug("cmp_audio_capture_free_wav: Null wav_data ignored\n");
  }
  return rc;
}
