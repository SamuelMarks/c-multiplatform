/**
 * @file ui_audio_coreaudio.c
 * @brief CoreAudio implementation for audio sink.
 */

/* clang-format off */
#include "ui_audio_sink.h"
#include "ui_error.h"
#include "ui_internal_mem.h"
#include "ui_types.h"

#if defined(__APPLE__)
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#endif
/* clang-format on */

#if defined(__APPLE__)

struct ui_coreaudio_api {
  AudioComponent (*AudioComponentFindNext)(
      AudioComponent inComponent, const AudioComponentDescription *inDesc);
  OSStatus (*AudioComponentInstanceNew)(AudioComponent inComponent,
                                        AudioComponentInstance *outInstance);
  OSStatus (*AudioComponentInstanceDispose)(AudioComponentInstance inInstance);
  OSStatus (*AudioUnitInitialize)(AudioUnit inUnit);
  OSStatus (*AudioUnitUninitialize)(AudioUnit inUnit);
  OSStatus (*AudioUnitSetProperty)(AudioUnit inUnit, AudioUnitPropertyID inID,
                                   AudioUnitScope inScope,
                                   AudioUnitElement inElement,
                                   const void *inData, UInt32 inDataSize);
  OSStatus (*AudioOutputUnitStart)(AudioUnit inUnit);
  OSStatus (*AudioOutputUnitStop)(AudioUnit inUnit);
};

static const struct ui_coreaudio_api g_real_coreaudio = {
    AudioComponentFindNext,        AudioComponentInstanceNew,
    AudioComponentInstanceDispose, AudioUnitInitialize,
    AudioUnitUninitialize,         AudioUnitSetProperty,
    AudioOutputUnitStart,          AudioOutputUnitStop};

static struct ui_coreaudio_api g_ca = {
    AudioComponentFindNext,        AudioComponentInstanceNew,
    AudioComponentInstanceDispose, AudioUnitInitialize,
    AudioUnitUninitialize,         AudioUnitSetProperty,
    AudioOutputUnitStart,          AudioOutputUnitStop};

/* For testing purposes, to inject mock API */
void ui_audio_coreaudio_set_mock_api(const struct ui_coreaudio_api *mock);
void ui_audio_coreaudio_set_mock_api(const struct ui_coreaudio_api *mock) {
  if (mock) {
    g_ca = *mock;
  } else {
    g_ca = g_real_coreaudio;
  }
}

struct ui_audio_sink {
  AudioComponentInstance audio_unit;
  int sample_rate;
  int frame_size;
  /* Simple ring buffer or write context could be added here */
};

static OSStatus render_callback(void *inRefCon,
                                AudioUnitRenderActionFlags *ioActionFlags,
                                const AudioTimeStamp *inTimeStamp,
                                UInt32 inBusNumber, UInt32 inNumberFrames,
                                AudioBufferList *ioData) {
  /* This is a push-based interface (write_frames) over a pull-based
     (render_callback) CoreAudio. A real implementation needs a thread-safe
     ringbuffer. For now, we just zero the buffer to prevent noise since this is
     a stub/skeleton. */
  (void)inRefCon;
  (void)ioActionFlags;
  (void)inTimeStamp;
  (void)inBusNumber;
  (void)inNumberFrames;

  if (ioData && ioData->mNumberBuffers > 0) {
    memset(ioData->mBuffers[0].mData, 0, ioData->mBuffers[0].mDataByteSize);
  }
  return noErr;
}

static ui_error_t ca_create_sink(struct ui_audio_sink_backend *backend,
                                 const struct ui_audio_sink_config *config,
                                 struct ui_audio_sink **out_sink) {
  struct ui_audio_sink *sink = NULL;
  AudioComponentDescription desc;
  AudioComponent comp;
  OSStatus err;
  AudioStreamBasicDescription stream_desc;
  AURenderCallbackStruct render_cb;

  if (!backend || !config || !out_sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sink = (struct ui_audio_sink *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_audio_sink));
  if (!sink) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  sink->audio_unit = NULL;
  sink->sample_rate = config->sample_rate;
  sink->frame_size = config->frame_size;

  desc.componentType = kAudioUnitType_Output;
  desc.componentSubType = kAudioUnitSubType_DefaultOutput;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;

  comp = g_ca.AudioComponentFindNext(NULL, &desc);
  if (!comp) {
    C_MULTIPLATFORM_FREE(sink);
    return UI_ERROR_IO_FAILED;
  }

  err = g_ca.AudioComponentInstanceNew(comp, &sink->audio_unit);
  if (err != noErr || !sink->audio_unit) {
    C_MULTIPLATFORM_FREE(sink);
    return UI_ERROR_IO_FAILED;
  }

  memset(&stream_desc, 0, sizeof(stream_desc));
  stream_desc.mSampleRate = (Float64)config->sample_rate;
  stream_desc.mFormatID = kAudioFormatLinearPCM;
  stream_desc.mFormatFlags =
      kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
  if (config->frame_size == config->channels * 4) {
    stream_desc.mFormatFlags =
        kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
  }
  stream_desc.mFramesPerPacket = 1;
  stream_desc.mChannelsPerFrame = (UInt32)config->channels;
  stream_desc.mBitsPerChannel =
      (UInt32)((config->frame_size / config->channels) * 8);
  stream_desc.mBytesPerPacket = (UInt32)config->frame_size;
  stream_desc.mBytesPerFrame = (UInt32)config->frame_size;

  err = g_ca.AudioUnitSetProperty(
      sink->audio_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
      0, &stream_desc, sizeof(stream_desc));
  if (err != noErr) {
    g_ca.AudioComponentInstanceDispose(sink->audio_unit);
    C_MULTIPLATFORM_FREE(sink);
    return UI_ERROR_IO_FAILED;
  }

  render_cb.inputProc = render_callback;
  render_cb.inputProcRefCon = sink;
  err = g_ca.AudioUnitSetProperty(
      sink->audio_unit, kAudioUnitProperty_SetRenderCallback,
      kAudioUnitScope_Input, 0, &render_cb, sizeof(render_cb));
  if (err != noErr) {
    g_ca.AudioComponentInstanceDispose(sink->audio_unit);
    C_MULTIPLATFORM_FREE(sink);
    return UI_ERROR_IO_FAILED;
  }

  err = g_ca.AudioUnitInitialize(sink->audio_unit);
  if (err != noErr) {
    g_ca.AudioComponentInstanceDispose(sink->audio_unit);
    C_MULTIPLATFORM_FREE(sink);
    return UI_ERROR_IO_FAILED;
  }

  *out_sink = sink;
  return UI_ERROR_NONE;
}

static ui_error_t ca_destroy_sink(struct ui_audio_sink_backend *backend,
                                  struct ui_audio_sink *sink) {
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (sink->audio_unit) {
    g_ca.AudioOutputUnitStop(sink->audio_unit);
    g_ca.AudioUnitUninitialize(sink->audio_unit);
    g_ca.AudioComponentInstanceDispose(sink->audio_unit);
  }
  C_MULTIPLATFORM_FREE(sink);
  return UI_ERROR_NONE;
}

static ui_error_t ca_write_frames(struct ui_audio_sink_backend *backend,
                                  struct ui_audio_sink *sink,
                                  const void *frames, int num_frames,
                                  int *out_frames_written) {
  if (!backend || !sink || !frames || !out_frames_written) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* In a real implementation with a ringbuffer, we would push here. */
  *out_frames_written = num_frames;
  return UI_ERROR_NONE;
}

static ui_error_t ca_get_delay(struct ui_audio_sink_backend *backend,
                               struct ui_audio_sink *sink,
                               ui_int64 *out_delay_us) {
  if (!backend || !sink || !out_delay_us) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_delay_us = 0;
  return UI_ERROR_NONE;
}

static ui_error_t ca_start(struct ui_audio_sink_backend *backend,
                           struct ui_audio_sink *sink) {
  OSStatus err;
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = g_ca.AudioOutputUnitStart(sink->audio_unit);
  if (err != noErr) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

static ui_error_t ca_stop(struct ui_audio_sink_backend *backend,
                          struct ui_audio_sink *sink) {
  OSStatus err;
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = g_ca.AudioOutputUnitStop(sink->audio_unit);
  if (err != noErr) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_coreaudio_get_backend(struct ui_audio_sink_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_sink = ca_create_sink;
  out_backend->destroy_sink = ca_destroy_sink;
  out_backend->write_frames = ca_write_frames;
  out_backend->get_delay = ca_get_delay;
  out_backend->start = ca_start;
  out_backend->stop = ca_stop;
  out_backend->user_data = NULL;

  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_get_default_backend(struct ui_audio_sink_backend *out_backend) {
  return ui_audio_sink_coreaudio_get_backend(out_backend);
}

#else

ui_error_t
ui_audio_sink_coreaudio_get_backend(struct ui_audio_sink_backend *out_backend) {
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

#endif /* defined(__APPLE__) */
