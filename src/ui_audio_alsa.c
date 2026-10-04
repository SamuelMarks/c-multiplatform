/**
 * @file ui_audio_alsa.c
 * @brief ALSA implementation for audio sink.
 */

/* clang-format off */
#include "ui_audio_sink.h"
#include "ui_error.h"
#include "ui_types.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#endif
/* clang-format on */

#if defined(__linux__) && !defined(__ANDROID__)

#define SND_PCM_STREAM_PLAYBACK 0
#define SND_PCM_ACCESS_RW_INTERLEAVED 3
#define SND_PCM_FORMAT_S16_LE 2
#define SND_PCM_FORMAT_S32_LE 10
#define SND_PCM_FORMAT_FLOAT_LE 14

struct ui_alsa_api {
  int (*snd_pcm_open)(void **pcm, const char *name, int stream, int mode);
  int (*snd_pcm_hw_params_malloc)(void **params);
  int (*snd_pcm_hw_params_any)(void *pcm, void *params);
  int (*snd_pcm_hw_params_set_access)(void *pcm, void *params, int access);
  int (*snd_pcm_hw_params_set_format)(void *pcm, void *params, int format);
  int (*snd_pcm_hw_params_set_rate_near)(void *pcm, void *params,
                                         unsigned int *val, int *dir);
  int (*snd_pcm_hw_params_set_channels)(void *pcm, void *params,
                                        unsigned int val);
  int (*snd_pcm_hw_params)(void *pcm, void *params);
  void (*snd_pcm_hw_params_free)(void *params);
  int (*snd_pcm_prepare)(void *pcm);
  long (*snd_pcm_writei)(void *pcm, const void *buffer, unsigned long size);
  int (*snd_pcm_recover)(void *pcm, int err, int silent);
  int (*snd_pcm_delay)(void *pcm, long *delayp);
  int (*snd_pcm_drop)(void *pcm);
  int (*snd_pcm_close)(void *pcm);
  const char *(*snd_strerror)(int errnum);
};

static struct ui_alsa_api g_alsa = {0};
static int g_alsa_loaded = 0;
static void *g_alsa_handle = NULL;

ui_error_t ui_audio_alsa_load_api(void);
ui_error_t ui_audio_alsa_load_api(void) {
  if (g_alsa_loaded) {
    return UI_ERROR_NONE;
  }

  g_alsa_handle = dlopen("libasound.so.2", RTLD_LAZY);
  if (!g_alsa_handle) {
    g_alsa_handle = dlopen("libasound.so", RTLD_LAZY);
    if (!g_alsa_handle) {
      return UI_ERROR_UNSUPPORTED;
    }
  }

#define LOAD_SYM(name)                                                         \
  do {                                                                         \
    g_alsa.name = (void *)dlsym(g_alsa_handle, #name);                         \
    if (!g_alsa.name) {                                                        \
      dlclose(g_alsa_handle);                                                  \
      g_alsa_handle = NULL;                                                    \
      return UI_ERROR_UNSUPPORTED;                                             \
    }                                                                          \
  } while (0)

  LOAD_SYM(snd_pcm_open);
  LOAD_SYM(snd_pcm_hw_params_malloc);
  LOAD_SYM(snd_pcm_hw_params_any);
  LOAD_SYM(snd_pcm_hw_params_set_access);
  LOAD_SYM(snd_pcm_hw_params_set_format);
  LOAD_SYM(snd_pcm_hw_params_set_rate_near);
  LOAD_SYM(snd_pcm_hw_params_set_channels);
  LOAD_SYM(snd_pcm_hw_params);
  LOAD_SYM(snd_pcm_hw_params_free);
  LOAD_SYM(snd_pcm_prepare);
  LOAD_SYM(snd_pcm_writei);
  LOAD_SYM(snd_pcm_recover);
  LOAD_SYM(snd_pcm_delay);
  LOAD_SYM(snd_pcm_drop);
  LOAD_SYM(snd_pcm_close);
  LOAD_SYM(snd_strerror);

#undef LOAD_SYM

  g_alsa_loaded = 1;
  return UI_ERROR_NONE;
}

/* For testing purposes, to inject mock API */
void ui_audio_alsa_set_mock_api(const struct ui_alsa_api *mock);
void ui_audio_alsa_set_mock_api(const struct ui_alsa_api *mock) {
  if (mock) {
    g_alsa = *mock;
    g_alsa_loaded = 1;
  } else {
    memset(&g_alsa, 0, sizeof(g_alsa));
    g_alsa_loaded = 0;
  }
}

struct ui_audio_sink {
  void *pcm;
  int frame_size;
  int sample_rate;
};

static ui_error_t alsa_create_sink(struct ui_audio_sink_backend *backend,
                                   const struct ui_audio_sink_config *config,
                                   struct ui_audio_sink **out_sink) {
  struct ui_audio_sink *sink = NULL;
  void *hwparams = NULL;
  int err;
  unsigned int rate;
  int format;
  ui_error_t rc;

  if (!backend || !config || !out_sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_audio_alsa_load_api();
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  sink = (struct ui_audio_sink *)malloc(sizeof(struct ui_audio_sink));
  if (!sink) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  sink->pcm = NULL;
  sink->frame_size = config->frame_size;
  sink->sample_rate = config->sample_rate;

  err = g_alsa.snd_pcm_open(&sink->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (err < 0) {
    free(sink);
    return UI_ERROR_IO_FAILED;
  }

  err = g_alsa.snd_pcm_hw_params_malloc(&hwparams);
  if (err < 0) {
    g_alsa.snd_pcm_close(sink->pcm);
    free(sink);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  err = g_alsa.snd_pcm_hw_params_any(sink->pcm, hwparams);
  if (err < 0) {
    goto hw_params_fail;
  }

  err = g_alsa.snd_pcm_hw_params_set_access(sink->pcm, hwparams,
                                            SND_PCM_ACCESS_RW_INTERLEAVED);
  if (err < 0) {
    goto hw_params_fail;
  }

  if (config->frame_size == config->channels * 2) {
    format = SND_PCM_FORMAT_S16_LE;
  } else if (config->frame_size == config->channels * 4) {
    format = SND_PCM_FORMAT_FLOAT_LE; /* We assume float or S32, this is just
                                         simple logic */
  } else {
    goto hw_params_fail;
  }

  err = g_alsa.snd_pcm_hw_params_set_format(sink->pcm, hwparams, format);
  if (err < 0) {
    goto hw_params_fail;
  }

  rate = (unsigned int)config->sample_rate;
  err = g_alsa.snd_pcm_hw_params_set_rate_near(sink->pcm, hwparams, &rate, 0);
  if (err < 0) {
    goto hw_params_fail;
  }

  err = g_alsa.snd_pcm_hw_params_set_channels(sink->pcm, hwparams,
                                              config->channels);
  if (err < 0) {
    goto hw_params_fail;
  }

  err = g_alsa.snd_pcm_hw_params(sink->pcm, hwparams);
  if (err < 0) {
    goto hw_params_fail;
  }

  g_alsa.snd_pcm_hw_params_free(hwparams);

  *out_sink = sink;
  return UI_ERROR_NONE;

hw_params_fail:
  g_alsa.snd_pcm_hw_params_free(hwparams);
  g_alsa.snd_pcm_close(sink->pcm);
  free(sink);
  return UI_ERROR_IO_FAILED;
}

static ui_error_t alsa_destroy_sink(struct ui_audio_sink_backend *backend,
                                    struct ui_audio_sink *sink) {
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (sink->pcm) {
    g_alsa.snd_pcm_drop(sink->pcm);
    g_alsa.snd_pcm_close(sink->pcm);
  }
  free(sink);
  return UI_ERROR_NONE;
}

static ui_error_t alsa_write_frames(struct ui_audio_sink_backend *backend,
                                    struct ui_audio_sink *sink,
                                    const void *frames, int num_frames,
                                    int *out_frames_written) {
  long res;
  if (!backend || !sink || !frames || !out_frames_written) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  res = g_alsa.snd_pcm_writei(sink->pcm, frames, num_frames);
  if (res == -32) { /* -EPIPE */
    res = g_alsa.snd_pcm_recover(sink->pcm, (int)res, 1);
    if (res >= 0) {
      res = g_alsa.snd_pcm_writei(sink->pcm, frames, num_frames);
    }
  }

  if (res < 0) {
    *out_frames_written = 0;
    return UI_ERROR_IO_FAILED;
  }

  *out_frames_written = (int)res;
  return UI_ERROR_NONE;
}

static ui_error_t alsa_get_delay(struct ui_audio_sink_backend *backend,
                                 struct ui_audio_sink *sink,
                                 ui_int64 *out_delay_us) {
  long delay_frames = 0;
  int err;
  if (!backend || !sink || !out_delay_us) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = g_alsa.snd_pcm_delay(sink->pcm, &delay_frames);
  if (err < 0) {
    return UI_ERROR_IO_FAILED;
  }

  if (delay_frames < 0) {
    delay_frames = 0;
  }

  if (sink->sample_rate > 0) {
    *out_delay_us = (ui_int64)delay_frames * 1000000 / sink->sample_rate;
  } else {
    *out_delay_us = 0;
  }

  return UI_ERROR_NONE;
}

static ui_error_t alsa_start(struct ui_audio_sink_backend *backend,
                             struct ui_audio_sink *sink) {
  int err;
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = g_alsa.snd_pcm_prepare(sink->pcm);
  if (err < 0) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

static ui_error_t alsa_stop(struct ui_audio_sink_backend *backend,
                            struct ui_audio_sink *sink) {
  int err;
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = g_alsa.snd_pcm_drop(sink->pcm);
  if (err < 0) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_alsa_get_backend(struct ui_audio_sink_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_sink = alsa_create_sink;
  out_backend->destroy_sink = alsa_destroy_sink;
  out_backend->write_frames = alsa_write_frames;
  out_backend->get_delay = alsa_get_delay;
  out_backend->start = alsa_start;
  out_backend->stop = alsa_stop;
  out_backend->user_data = NULL;

  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_get_default_backend(struct ui_audio_sink_backend *out_backend) {
  return ui_audio_sink_alsa_get_backend(out_backend);
}

#else

ui_error_t
ui_audio_sink_alsa_get_backend(struct ui_audio_sink_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_sink = NULL;
  out_backend->destroy_sink = NULL;
  out_backend->write_frames = NULL;
  out_backend->get_delay = NULL;
  out_backend->start = NULL;
  out_backend->stop = NULL;
  out_backend->user_data = NULL;

  return UI_ERROR_UNSUPPORTED;
}

#endif /* defined(__linux__) && !defined(__ANDROID__) */
