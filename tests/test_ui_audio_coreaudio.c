/* clang-format off */
#include "greatest.h"
#include "../include/ui_audio_sink.h"
#include "../include/ui_error.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__)
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
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

extern void
ui_audio_coreaudio_set_mock_api(const struct ui_coreaudio_api *mock);

static AudioComponent mock_comp_ret = (AudioComponent)-1;
static OSStatus mock_new_ret = noErr;
static OSStatus mock_dispose_ret = noErr;
static OSStatus mock_init_ret = noErr;
static OSStatus mock_uninit_ret = noErr;
static OSStatus mock_setprop_ret = noErr;
static OSStatus mock_start_ret = noErr;
static OSStatus mock_stop_ret = noErr;
static int mock_setprop_fail_at = -1;
static int mock_setprop_calls = 0;

static OSStatus (*g_captured_render_callback)(void *,
                                              AudioUnitRenderActionFlags *,
                                              const AudioTimeStamp *, UInt32,
                                              UInt32, AudioBufferList *) = NULL;

static AudioComponent
my_AudioComponentFindNext(AudioComponent inComponent,
                          const AudioComponentDescription *inDesc) {
  (void)inComponent;
  (void)inDesc;
  if (mock_comp_ret == (AudioComponent)-1) {
    return (AudioComponent)0xdeadbeef;
  }
  return mock_comp_ret;
}
static OSStatus
my_AudioComponentInstanceNew(AudioComponent inComponent,
                             AudioComponentInstance *outInstance) {
  (void)inComponent;
  if (mock_new_ret == noErr) {
    *outInstance = (AudioComponentInstance)0xcafebabe;
  } else {
    *outInstance = NULL;
  }
  return mock_new_ret;
}
static OSStatus
my_AudioComponentInstanceDispose(AudioComponentInstance inInstance) {
  (void)inInstance;
  return mock_dispose_ret;
}
static OSStatus my_AudioUnitInitialize(AudioUnit inUnit) {
  (void)inUnit;
  return mock_init_ret;
}
static OSStatus my_AudioUnitUninitialize(AudioUnit inUnit) {
  (void)inUnit;
  return mock_uninit_ret;
}
static OSStatus my_AudioUnitSetProperty(AudioUnit inUnit,
                                        AudioUnitPropertyID inID,
                                        AudioUnitScope inScope,
                                        AudioUnitElement inElement,
                                        const void *inData, UInt32 inDataSize) {
  (void)inUnit;
  (void)inID;
  (void)inScope;
  (void)inElement;
  (void)inData;
  (void)inDataSize;
  if (inID == kAudioUnitProperty_SetRenderCallback && inData) {
    const AURenderCallbackStruct *cb = (const AURenderCallbackStruct *)inData;
    g_captured_render_callback = cb->inputProc;
  }

  mock_setprop_calls++;
  if (mock_setprop_fail_at != -1 &&
      mock_setprop_calls == mock_setprop_fail_at) {
    return -1;
  }
  return mock_setprop_ret;
}
static OSStatus my_AudioOutputUnitStart(AudioUnit inUnit) {
  (void)inUnit;
  return mock_start_ret;
}
static OSStatus my_AudioOutputUnitStop(AudioUnit inUnit) {
  (void)inUnit;
  return mock_stop_ret;
}

static struct ui_coreaudio_api mock_api = {
    my_AudioComponentFindNext,        my_AudioComponentInstanceNew,
    my_AudioComponentInstanceDispose, my_AudioUnitInitialize,
    my_AudioUnitUninitialize,         my_AudioUnitSetProperty,
    my_AudioOutputUnitStart,          my_AudioOutputUnitStop};

static void reset_mocks(void) {
  mock_comp_ret = (AudioComponent)-1;
  mock_new_ret = noErr;
  mock_dispose_ret = noErr;
  mock_init_ret = noErr;
  mock_uninit_ret = noErr;
  mock_setprop_ret = noErr;
  mock_start_ret = noErr;
  mock_stop_ret = noErr;
  mock_setprop_fail_at = -1;
  mock_setprop_calls = 0;
  ui_audio_coreaudio_set_mock_api(&mock_api);
}

TEST test_coreaudio_get_backend_null(void) {
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_audio_sink_coreaudio_get_backend(NULL));
  PASS();
}

TEST test_coreaudio_get_backend_success(void) {
  struct ui_audio_sink_backend backend;
  ASSERT_EQ(UI_ERROR_NONE, ui_audio_sink_coreaudio_get_backend(&backend));
  ASSERT_EQ(UI_ERROR_NONE, ui_audio_sink_get_default_backend(&backend));
  ASSERT(backend.create_sink != NULL);
  PASS();
}

TEST test_coreaudio_create_sink_invalid_args(void) {
  struct ui_audio_sink_backend backend;
  struct ui_audio_sink_config config = {44100, 2, 4};
  struct ui_audio_sink *sink = NULL;

  ui_audio_sink_coreaudio_get_backend(&backend);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.create_sink(NULL, &config, &sink));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.create_sink(&backend, NULL, &sink));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.create_sink(&backend, &config, NULL));
  PASS();
}

TEST test_coreaudio_create_sink_success(void) {
  struct ui_audio_sink_backend backend;
  struct ui_audio_sink_config config = {44100, 2, 4};
  struct ui_audio_sink *sink = NULL;

  reset_mocks();
  ui_audio_sink_coreaudio_get_backend(&backend);
  ASSERT_EQ(UI_ERROR_NONE, backend.create_sink(&backend, &config, &sink));
  ASSERT(sink != NULL);

  ASSERT_EQ(UI_ERROR_NONE, backend.destroy_sink(&backend, sink));
  PASS();
}

TEST test_coreaudio_create_sink_float(void) {
  struct ui_audio_sink_backend backend;
  struct ui_audio_sink_config config = {
      44100, 2, 8}; /* float 32-bit = 8 bytes per frame */
  struct ui_audio_sink *sink = NULL;

  reset_mocks();
  ui_audio_sink_coreaudio_get_backend(&backend);
  ASSERT_EQ(UI_ERROR_NONE, backend.create_sink(&backend, &config, &sink));
  ASSERT(sink != NULL);

  ASSERT_EQ(UI_ERROR_NONE, backend.destroy_sink(&backend, sink));
  PASS();
}

TEST test_coreaudio_create_sink_failures(void) {
  struct ui_audio_sink_backend backend;
  struct ui_audio_sink_config config = {44100, 2, 4};
  struct ui_audio_sink *sink = NULL;

  ui_audio_sink_coreaudio_get_backend(&backend);

  reset_mocks();
  mock_comp_ret = (AudioComponent)0;
  ASSERT_EQ(UI_ERROR_IO_FAILED, backend.create_sink(&backend, &config, &sink));

  reset_mocks();
  mock_new_ret = -1;
  ASSERT_EQ(UI_ERROR_IO_FAILED, backend.create_sink(&backend, &config, &sink));

  reset_mocks();
  mock_setprop_fail_at = 1;
  ASSERT_EQ(UI_ERROR_IO_FAILED, backend.create_sink(&backend, &config, &sink));

  reset_mocks();
  mock_setprop_fail_at = 2;
  ASSERT_EQ(UI_ERROR_IO_FAILED, backend.create_sink(&backend, &config, &sink));

  reset_mocks();
  mock_init_ret = -1;
  ASSERT_EQ(UI_ERROR_IO_FAILED, backend.create_sink(&backend, &config, &sink));

  /* Test OOM */
  reset_mocks();
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY,
            backend.create_sink(&backend, &config, &sink));
  g_malloc_fail_countdown = -1;

  /* Test set_mock_api(NULL) */
  ui_audio_coreaudio_set_mock_api(NULL);
  ui_audio_coreaudio_set_mock_api(&mock_api); /* restore */

  /* Test render_callback */
  reset_mocks();
  g_captured_render_callback = NULL;
  ASSERT_EQ(UI_ERROR_NONE, backend.create_sink(&backend, &config, &sink));
  if (g_captured_render_callback) {
    AudioBufferList buf_list;
    buf_list.mNumberBuffers = 1;
    buf_list.mBuffers[0].mNumberChannels = 2;
    buf_list.mBuffers[0].mDataByteSize = 8;
    buf_list.mBuffers[0].mData = malloc(8);
    memset(buf_list.mBuffers[0].mData, 1, 8);

    g_captured_render_callback(NULL, NULL, NULL, 0, 0, &buf_list);

    /* Verify it zeroed the buffer */
    ASSERT_EQ(0, ((char *)buf_list.mBuffers[0].mData)[0]);
    free(buf_list.mBuffers[0].mData);
  }
  backend.destroy_sink(&backend, sink);

  PASS();
}

TEST test_coreaudio_destroy_sink_invalid(void) {
  struct ui_audio_sink_backend backend;
  ui_audio_sink_coreaudio_get_backend(&backend);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.destroy_sink(NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.destroy_sink(&backend, NULL));
  PASS();
}

TEST test_coreaudio_write_frames(void) {
  struct ui_audio_sink_backend backend;
  struct ui_audio_sink_config config = {44100, 2, 4};
  struct ui_audio_sink *sink = NULL;
  char buf[4] = {0};
  int written = 0;

  reset_mocks();
  ui_audio_sink_coreaudio_get_backend(&backend);
  backend.create_sink(&backend, &config, &sink);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.write_frames(NULL, sink, buf, 1, &written));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.write_frames(&backend, NULL, buf, 1, &written));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.write_frames(&backend, sink, NULL, 1, &written));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.write_frames(&backend, sink, buf, 1, NULL));

  ASSERT_EQ(UI_ERROR_NONE,
            backend.write_frames(&backend, sink, buf, 1, &written));
  ASSERT_EQ(1, written);

  backend.destroy_sink(&backend, sink);
  PASS();
}

TEST test_coreaudio_get_delay(void) {
  struct ui_audio_sink_backend backend;
  struct ui_audio_sink_config config = {44100, 2, 4};
  struct ui_audio_sink *sink = NULL;
  ui_int64 delay = 0;

  reset_mocks();
  ui_audio_sink_coreaudio_get_backend(&backend);
  backend.create_sink(&backend, &config, &sink);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.get_delay(NULL, sink, &delay));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.get_delay(&backend, NULL, &delay));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.get_delay(&backend, sink, NULL));

  ASSERT_EQ(UI_ERROR_NONE, backend.get_delay(&backend, sink, &delay));
  ASSERT_EQ(0, delay);

  backend.destroy_sink(&backend, sink);
  PASS();
}

TEST test_coreaudio_start_stop(void) {
  struct ui_audio_sink_backend backend;
  struct ui_audio_sink_config config = {44100, 2, 4};
  struct ui_audio_sink *sink = NULL;

  reset_mocks();
  ui_audio_sink_coreaudio_get_backend(&backend);
  backend.create_sink(&backend, &config, &sink);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.start(NULL, sink));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.start(&backend, NULL));

  mock_start_ret = noErr;
  ASSERT_EQ(UI_ERROR_NONE, backend.start(&backend, sink));

  mock_start_ret = -1;
  ASSERT_EQ(UI_ERROR_IO_FAILED, backend.start(&backend, sink));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.stop(NULL, sink));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.stop(&backend, NULL));

  mock_stop_ret = noErr;
  ASSERT_EQ(UI_ERROR_NONE, backend.stop(&backend, sink));

  mock_stop_ret = -1;
  ASSERT_EQ(UI_ERROR_IO_FAILED, backend.stop(&backend, sink));

  backend.destroy_sink(&backend, sink);
  PASS();
}

SUITE(coreaudio_suite) {
  RUN_TEST(test_coreaudio_get_backend_null);
  RUN_TEST(test_coreaudio_get_backend_success);
  RUN_TEST(test_coreaudio_create_sink_invalid_args);
  RUN_TEST(test_coreaudio_create_sink_success);
  RUN_TEST(test_coreaudio_create_sink_float);
  RUN_TEST(test_coreaudio_create_sink_failures);
  RUN_TEST(test_coreaudio_destroy_sink_invalid);
  RUN_TEST(test_coreaudio_write_frames);
  RUN_TEST(test_coreaudio_get_delay);
  RUN_TEST(test_coreaudio_start_stop);
}

#else

TEST test_coreaudio_unsupported(void) {
  struct ui_audio_sink_backend backend;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_audio_sink_coreaudio_get_backend(NULL));
  ASSERT_EQ(UI_ERROR_UNSUPPORTED,
            ui_audio_sink_coreaudio_get_backend(&backend));
  PASS();
}

SUITE(coreaudio_suite) { RUN_TEST(test_coreaudio_unsupported); }

#endif

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(coreaudio_suite);
  GREATEST_MAIN_END();
}
