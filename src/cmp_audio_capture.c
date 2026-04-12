/* clang-format off */
#include "cmp_audio_capture.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_audio_capture {
  int is_recording;
  unsigned int dummy_pcm_size;
};

int cmp_audio_capture_create(cmp_audio_capture_t **out_capture) {
  cmp_audio_capture_t *cap;
  if (!out_capture) {
    return -1;
  }
  cap = (cmp_audio_capture_t *)malloc(sizeof(cmp_audio_capture_t));
  if (!cap) {
    return -2;
  }
  cap->is_recording = 0;
  cap->dummy_pcm_size = 0;
  *out_capture = cap;
  return 0;
}

int cmp_audio_capture_destroy(cmp_audio_capture_t *capture) {
  if (!capture) {
    return -1;
  }
  free(capture);
  return 0;
}

int cmp_audio_capture_start(cmp_audio_capture_t *capture) {
  if (!capture) {
    return -1;
  }
  if (capture->is_recording) {
    return -3;
  }
  capture->is_recording = 1;
  capture->dummy_pcm_size = 44100; /* Mock 1 second of audio */
  return 0;
}

int cmp_audio_capture_stop(cmp_audio_capture_t *capture) {
  if (!capture) {
    return -1;
  }
  if (!capture->is_recording) {
    return -3;
  }
  capture->is_recording = 0;
  return 0;
}

int cmp_audio_capture_get_wav(cmp_audio_capture_t *capture,
                              unsigned char **out_wav_data,
                              unsigned int *out_size) {
  unsigned char *wav;
  unsigned int total_size;

  if (!capture || !out_wav_data || !out_size) {
    return -1;
  }
  if (capture->is_recording) {
    return -3; /* Must stop before getting */
  }

  /* Mock WAV encoding */
  total_size = capture->dummy_pcm_size + 44; /* 44 bytes for WAV header */
  wav = (unsigned char *)malloc(total_size);
  if (!wav) {
    return -2;
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
  return 0;
}

int cmp_audio_capture_free_wav(unsigned char *wav_data) {
  if (wav_data) {
    free(wav_data);
  }
  return 0;
}
