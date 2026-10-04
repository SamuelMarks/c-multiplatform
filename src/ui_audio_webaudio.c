/**
 * @file ui_audio_webaudio.c
 * @brief WebAudio implementation for audio sink.
 */

/* clang-format off */
#include "ui_audio_sink.h"
#include "ui_error.h"
#include "ui_types.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#endif
/* clang-format on */

#if defined(__EMSCRIPTEN__)

struct ui_audio_sink {
  int ctx_id;
  int node_id;
  int sample_rate;
  int frame_size;
  int channels;
};

/* Using JS interop via EM_ASM for WebAudio initialization and write.
   This avoids needing complex C-side structs. */

static ui_error_t wa_create_sink(struct ui_audio_sink_backend *backend,
                                 const struct ui_audio_sink_config *config,
                                 struct ui_audio_sink **out_sink) {
  struct ui_audio_sink *sink = NULL;

  if (!backend || !config || !out_sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sink = (struct ui_audio_sink *)malloc(sizeof(struct ui_audio_sink));
  if (!sink) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  sink->sample_rate = config->sample_rate;
  sink->frame_size = config->frame_size;
  sink->channels = config->channels;

  /* EM_ASM returns a handle integer representing the JS object (simplified) */
  sink->ctx_id = MAIN_THREAD_EM_ASM_INT(
      {
        if (!window.uiWebAudioCtx) {
          window.uiWebAudioCtx =
              new (window.AudioContext ||
                   window.webkitAudioContext)({sampleRate : $0});
        }
        return 1;
      },
      config->sample_rate);

  if (!sink->ctx_id) {
    free(sink);
    return UI_ERROR_IO_FAILED;
  }

  *out_sink = sink;
  return UI_ERROR_NONE;
}

static ui_error_t wa_destroy_sink(struct ui_audio_sink_backend *backend,
                                  struct ui_audio_sink *sink) {
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* we don't close the shared context, just free memory */
  free(sink);
  return UI_ERROR_NONE;
}

static ui_error_t wa_write_frames(struct ui_audio_sink_backend *backend,
                                  struct ui_audio_sink *sink,
                                  const void *frames, int num_frames,
                                  int *out_frames_written) {
  if (!backend || !sink || !frames || !out_frames_written) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* We would push to an AudioWorklet or ScriptProcessorNode here */
  *out_frames_written = num_frames;
  return UI_ERROR_NONE;
}

static ui_error_t wa_get_delay(struct ui_audio_sink_backend *backend,
                               struct ui_audio_sink *sink,
                               ui_int64 *out_delay_us) {
  if (!backend || !sink || !out_delay_us) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_delay_us = 0;
  return UI_ERROR_NONE;
}

static ui_error_t wa_start(struct ui_audio_sink_backend *backend,
                           struct ui_audio_sink *sink) {
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  MAIN_THREAD_EM_ASM({
    if (window.uiWebAudioCtx &&window.uiWebAudioCtx.state == = 'suspended') {
      window.uiWebAudioCtx.resume();
    }
  });
  return UI_ERROR_NONE;
}

static ui_error_t wa_stop(struct ui_audio_sink_backend *backend,
                          struct ui_audio_sink *sink) {
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  MAIN_THREAD_EM_ASM({
    if (window.uiWebAudioCtx &&window.uiWebAudioCtx.state == = 'running') {
      window.uiWebAudioCtx.suspend();
    }
  });
  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_webaudio_get_backend(struct ui_audio_sink_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_sink = wa_create_sink;
  out_backend->destroy_sink = wa_destroy_sink;
  out_backend->write_frames = wa_write_frames;
  out_backend->get_delay = wa_get_delay;
  out_backend->start = wa_start;
  out_backend->stop = wa_stop;
  out_backend->user_data = NULL;

  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_get_default_backend(struct ui_audio_sink_backend *out_backend) {
  return ui_audio_sink_webaudio_get_backend(out_backend);
}

#else

ui_error_t
ui_audio_sink_webaudio_get_backend(struct ui_audio_sink_backend *out_backend) {
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

#endif /* defined(__EMSCRIPTEN__) */
