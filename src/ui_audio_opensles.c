/**
 * @file ui_audio_opensles.c
 * @brief OpenSL ES implementation for audio sink.
 */

/* clang-format off */
#include "ui_audio_sink.h"
#include "ui_error.h"
#include "ui_types.h"

#if defined(__ANDROID__)
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#endif
/* clang-format on */

#if defined(__ANDROID__)

/* In a real implementation we would define a struct of function pointers for
   mocking. But since this is Android only, we can just compile the actual
   OpenSLES code. We still provide a mock struct for 100% branch test coverage
   if run on Android. */

struct ui_opensles_api {
  SLresult (*slCreateEngine)(SLObjectItf *pEngine, SLuint32 numOptions,
                             const SLEngineOption *pEngineOptions,
                             SLuint32 numInterfaces,
                             const SLInterfaceID *pInterfaceIds,
                             const SLboolean *pInterfaceRequired);
};

static SLresult real_slCreateEngine(SLObjectItf *pEngine, SLuint32 numOptions,
                                    const SLEngineOption *pEngineOptions,
                                    SLuint32 numInterfaces,
                                    const SLInterfaceID *pInterfaceIds,
                                    const SLboolean *pInterfaceRequired) {
  return slCreateEngine(pEngine, numOptions, pEngineOptions, numInterfaces,
                        pInterfaceIds, pInterfaceRequired);
}

static struct ui_opensles_api g_sles = {real_slCreateEngine};

void ui_audio_opensles_set_mock_api(const struct ui_opensles_api *mock);
void ui_audio_opensles_set_mock_api(const struct ui_opensles_api *mock) {
  if (mock) {
    g_sles = *mock;
  } else {
    g_sles.slCreateEngine = real_slCreateEngine;
  }
}

struct ui_audio_sink {
  SLObjectItf engine_obj;
  SLEngineItf engine_engine;
  SLObjectItf mix_obj;
  SLObjectItf player_obj;
  SLPlayItf player_play;
  SLAndroidSimpleBufferQueueItf player_bq;

  int sample_rate;
  int frame_size;
};

static void bq_callback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  (void)bq;
  (void)context;
}

static ui_error_t sl_create_sink(struct ui_audio_sink_backend *backend,
                                 const struct ui_audio_sink_config *config,
                                 struct ui_audio_sink **out_sink) {
  struct ui_audio_sink *sink = NULL;
  SLresult res;

  const SLInterfaceID engine_mix_req[] = {SL_IID_ENGINE};
  const SLboolean engine_mix_req_req[] = {SL_BOOLEAN_TRUE};
  const SLInterfaceID mix_req[] = {SL_IID_ENVIRONMENTALREVERB};
  const SLboolean mix_req_req[] = {SL_BOOLEAN_FALSE};
  const SLInterfaceID player_req[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
  const SLboolean player_req_req[] = {SL_BOOLEAN_TRUE};

  SLDataLocator_AndroidSimpleBufferQueue loc_bufq;
  SLDataFormat_PCM format_pcm;
  SLDataSource audio_src;

  SLDataLocator_OutputMix loc_outmix;
  SLDataSink audio_snk;

  if (!backend || !config || !out_sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sink = (struct ui_audio_sink *)malloc(sizeof(struct ui_audio_sink));
  if (!sink) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  sink->engine_obj = NULL;
  sink->engine_engine = NULL;
  sink->mix_obj = NULL;
  sink->player_obj = NULL;
  sink->player_play = NULL;
  sink->player_bq = NULL;
  sink->sample_rate = config->sample_rate;
  sink->frame_size = config->frame_size;

  res = g_sles.slCreateEngine(&sink->engine_obj, 0, NULL, 0, NULL, NULL);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  res = (*sink->engine_obj)->Realize(sink->engine_obj, SL_BOOLEAN_FALSE);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  res =
      (*sink->engine_obj)
          ->GetInterface(sink->engine_obj, SL_IID_ENGINE, &sink->engine_engine);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  res = (*sink->engine_engine)
            ->CreateOutputMix(sink->engine_engine, &sink->mix_obj, 1, mix_req,
                              mix_req_req);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  res = (*sink->mix_obj)->Realize(sink->mix_obj, SL_BOOLEAN_FALSE);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  loc_bufq.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
  loc_bufq.numBuffers = 2;

  format_pcm.formatType = SL_DATAFORMAT_PCM;
  format_pcm.numChannels = config->channels;
  format_pcm.samplesPerSec = config->sample_rate * 1000;
  format_pcm.bitsPerSample =
      SL_PCMSAMPLEFORMAT_FIXED_16; /* simplified for stub */
  if (config->frame_size == config->channels * 4) {
    format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_32;
  }
  format_pcm.containerSize = format_pcm.bitsPerSample;
  format_pcm.channelMask =
      (config->channels == 2) ? (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT)
                              : SL_SPEAKER_FRONT_CENTER;
  format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

  audio_src.pLocator = &loc_bufq;
  audio_src.pFormat = &format_pcm;

  loc_outmix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
  loc_outmix.outputMix = sink->mix_obj;
  audio_snk.pLocator = &loc_outmix;
  audio_snk.pFormat = NULL;

  res = (*sink->engine_engine)
            ->CreateAudioPlayer(sink->engine_engine, &sink->player_obj,
                                &audio_src, &audio_snk, 1, player_req,
                                player_req_req);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  res = (*sink->player_obj)->Realize(sink->player_obj, SL_BOOLEAN_FALSE);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  res = (*sink->player_obj)
            ->GetInterface(sink->player_obj, SL_IID_PLAY, &sink->player_play);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  res = (*sink->player_obj)
            ->GetInterface(sink->player_obj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                           &sink->player_bq);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  res =
      (*sink->player_bq)->RegisterCallback(sink->player_bq, bq_callback, sink);
  if (res != SL_RESULT_SUCCESS) {
    goto fail;
  }

  *out_sink = sink;
  return UI_ERROR_NONE;

fail:
  if (sink->player_obj) {
    (*sink->player_obj)->Destroy(sink->player_obj);
  }
  if (sink->mix_obj) {
    (*sink->mix_obj)->Destroy(sink->mix_obj);
  }
  if (sink->engine_obj) {
    (*sink->engine_obj)->Destroy(sink->engine_obj);
  }
  free(sink);
  return UI_ERROR_IO_FAILED;
}

static ui_error_t sl_destroy_sink(struct ui_audio_sink_backend *backend,
                                  struct ui_audio_sink *sink) {
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (sink->player_obj) {
    (*sink->player_obj)->Destroy(sink->player_obj);
  }
  if (sink->mix_obj) {
    (*sink->mix_obj)->Destroy(sink->mix_obj);
  }
  if (sink->engine_obj) {
    (*sink->engine_obj)->Destroy(sink->engine_obj);
  }
  free(sink);
  return UI_ERROR_NONE;
}

static ui_error_t sl_write_frames(struct ui_audio_sink_backend *backend,
                                  struct ui_audio_sink *sink,
                                  const void *frames, int num_frames,
                                  int *out_frames_written) {
  SLresult res;
  if (!backend || !sink || !frames || !out_frames_written) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  res = (*sink->player_bq)
            ->Enqueue(sink->player_bq, frames, num_frames * sink->frame_size);
  if (res != SL_RESULT_SUCCESS) {
    return UI_ERROR_IO_FAILED;
  }

  *out_frames_written = num_frames;
  return UI_ERROR_NONE;
}

static ui_error_t sl_get_delay(struct ui_audio_sink_backend *backend,
                               struct ui_audio_sink *sink,
                               ui_int64 *out_delay_us) {
  if (!backend || !sink || !out_delay_us) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_delay_us = 0;
  return UI_ERROR_NONE;
}

static ui_error_t sl_start(struct ui_audio_sink_backend *backend,
                           struct ui_audio_sink *sink) {
  SLresult res;
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  res = (*sink->player_play)
            ->SetPlayState(sink->player_play, SL_PLAYSTATE_PLAYING);
  if (res != SL_RESULT_SUCCESS) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

static ui_error_t sl_stop(struct ui_audio_sink_backend *backend,
                          struct ui_audio_sink *sink) {
  SLresult res;
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  res = (*sink->player_play)
            ->SetPlayState(sink->player_play, SL_PLAYSTATE_STOPPED);
  if (res != SL_RESULT_SUCCESS) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_opensles_get_backend(struct ui_audio_sink_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_sink = sl_create_sink;
  out_backend->destroy_sink = sl_destroy_sink;
  out_backend->write_frames = sl_write_frames;
  out_backend->get_delay = sl_get_delay;
  out_backend->start = sl_start;
  out_backend->stop = sl_stop;
  out_backend->user_data = NULL;

  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_get_default_backend(struct ui_audio_sink_backend *out_backend) {
  return ui_audio_sink_opensles_get_backend(out_backend);
}

#else

ui_error_t
ui_audio_sink_opensles_get_backend(struct ui_audio_sink_backend *out_backend) {
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

#endif /* defined(__ANDROID__) */
