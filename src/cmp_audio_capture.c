/* clang-format off */
#include "cmp_audio_capture.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
/* clang-format on */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct cmp_audio_capture {
  int is_recording;
  unsigned int dummy_pcm_size;
};

/**
 * @brief Writes a 32-bit little-endian value to a byte buffer.
 * @param ptr The buffer.
 * @param val The 32-bit value.
 */
static void write_le32(unsigned char *ptr, unsigned int val) {
  ptr[0] = (unsigned char)(val & 0xFF);
  ptr[1] = (unsigned char)((val >> 8) & 0xFF);
  ptr[2] = (unsigned char)((val >> 16) & 0xFF);
  ptr[3] = (unsigned char)((val >> 24) & 0xFF);
}

/**
 * @brief Writes a 16-bit little-endian value to a byte buffer.
 * @param ptr The buffer.
 * @param val The 16-bit value.
 */
static void write_le16(unsigned char *ptr, unsigned short val) {
  ptr[0] = (unsigned char)(val & 0xFF);
  ptr[1] = (unsigned char)((val >> 8) & 0xFF);
}

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
  capture->dummy_pcm_size = 44100 * 2; /* Mock 1 second of 16-bit mono audio */
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
  unsigned int data_size;
  unsigned int sample_rate = 44100;
  unsigned short num_channels = 1;
  unsigned short bits_per_sample = 16;
  unsigned int i;

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

  data_size = capture->dummy_pcm_size;
  total_size = data_size + 44; /* 44 bytes for WAV header */

  rc = CMP_MALLOC(total_size, (void **)&wav);
  if (rc != CMP_SUCCESS || wav == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_audio_capture_get_wav: Out of memory\n");
    return rc;
  }

  memset(wav, 0, total_size);

  /* Generate valid WAV header */
  memcpy(wav, "RIFF", 4);
  write_le32(wav + 4, total_size - 8);
  memcpy(wav + 8, "WAVE", 4);
  memcpy(wav + 12, "fmt ", 4);
  write_le32(wav + 16, 16); /* fmt chunk size */
  write_le16(wav + 20, 1);  /* audio format (PCM) */
  write_le16(wav + 22, num_channels);
  write_le32(wav + 24, sample_rate);
  write_le32(wav + 28, sample_rate * num_channels *
                           (bits_per_sample / 8));            /* byte rate */
  write_le16(wav + 32, num_channels * (bits_per_sample / 8)); /* block align */
  write_le16(wav + 34, bits_per_sample);
  memcpy(wav + 36, "data", 4);
  write_le32(wav + 40, data_size);

  /* Generate 440 Hz sine wave */
  for (i = 0; i < data_size / 2; i++) {
    double time = (double)i / sample_rate;
    double val = sin(2.0 * M_PI * 440.0 * time) * 32767.0;
    short sample = (short)val;
    write_le16(wav + 44 + (i * 2), sample);
  }

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
