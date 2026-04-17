/* clang-format off */
#include "cmp_audio_capture.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_audio_capture {
  int is_recording;
  unsigned int dummy_pcm_size;
};

int cmp_audio_capture_create(cmp_audio_capture_t **out_capture) {
  int rc = CMP_SUCCESS;
  cmp_audio_capture_t *cap = NULL;

  if (!out_capture) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_audio_capture_create: Invalid argument "
                    "(out_capture=NULL)\n");
    return rc;
  }

  cap = (cmp_audio_capture_t *)malloc(sizeof(cmp_audio_capture_t));
  if (!cap) {
    rc = CMP_ERROR_OOM;
    fprintf(stderr, "Error in cmp_audio_capture_create: Out of memory\n");
    return rc;
  }

  cap->is_recording = 0;
  cap->dummy_pcm_size = 0;
  *out_capture = cap;
  return rc;
}

int cmp_audio_capture_destroy(cmp_audio_capture_t *capture) {
  int rc = CMP_SUCCESS;

  if (!capture) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_audio_capture_destroy: Invalid argument "
                    "(capture=NULL)\n");
    return rc;
  }
  free(capture);
  return rc;
}

int cmp_audio_capture_start(cmp_audio_capture_t *capture) {
  int rc = CMP_SUCCESS;

  if (!capture) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(
        stderr,
        "Error in cmp_audio_capture_start: Invalid argument (capture=NULL)\n");
    return rc;
  }
  if (capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    fprintf(stderr, "Error in cmp_audio_capture_start: Already recording\n");
    return rc;
  }
  capture->is_recording = 1;
  capture->dummy_pcm_size = 44100; /* Mock 1 second of audio */
  return rc;
}

int cmp_audio_capture_stop(cmp_audio_capture_t *capture) {
  int rc = CMP_SUCCESS;

  if (!capture) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(
        stderr,
        "Error in cmp_audio_capture_stop: Invalid argument (capture=NULL)\n");
    return rc;
  }
  if (!capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    fprintf(stderr, "Error in cmp_audio_capture_stop: Not recording\n");
    return rc;
  }
  capture->is_recording = 0;
  return rc;
}

int cmp_audio_capture_get_wav(cmp_audio_capture_t *capture,
                              unsigned char **out_wav_data,
                              unsigned int *out_size) {
  int rc = CMP_SUCCESS;
  unsigned char *wav = NULL;
  unsigned int total_size;

  if (!capture || !out_wav_data || !out_size) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_audio_capture_get_wav: Invalid argument\n");
    return rc;
  }
  if (capture->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    fprintf(stderr, "Error in cmp_audio_capture_get_wav: Must stop recording "
                    "before getting WAV\n");
    return rc; /* Must stop before getting */
  }

  /* Mock WAV encoding */
  total_size = capture->dummy_pcm_size + 44; /* 44 bytes for WAV header */
  wav = (unsigned char *)malloc(total_size);
  if (!wav) {
    rc = CMP_ERROR_OOM;
    fprintf(stderr, "Error in cmp_audio_capture_get_wav: Out of memory\n");
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

int cmp_audio_capture_free_wav(unsigned char *wav_data) {
  int rc = CMP_SUCCESS;
  if (wav_data) {
    free(wav_data);
  }
  return rc;
}
