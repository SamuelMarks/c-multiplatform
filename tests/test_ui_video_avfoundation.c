/* clang-format off */
#include "greatest.h"
#include "../include/ui_video_decoder.h"
#include "../include/ui_error.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>

#if defined(__APPLE__)
#ifndef inline
#define inline __inline
#endif
#include <VideoToolbox/VideoToolbox.h>
#undef inline
#endif
/* clang-format on */

#if defined(__APPLE__)

struct ui_vt_api {
  OSStatus (*VTDecompressionSessionCreate)(
      CFAllocatorRef allocator,
      CMVideoFormatDescriptionRef videoFormatDescription,
      CFDictionaryRef videoDecoderSpecification,
      CFDictionaryRef destinationImageBufferAttributes,
      const VTDecompressionOutputCallbackRecord *outputCallback,
      VTDecompressionSessionRef *decompressionSessionOut);
  void (*VTDecompressionSessionInvalidate)(VTDecompressionSessionRef session);
  OSStatus (*VTDecompressionSessionDecodeFrame)(
      VTDecompressionSessionRef session, CMSampleBufferRef sampleBuffer,
      VTDecodeFrameFlags decodeFlags, void *sourceFrameRefCon,
      VTDecodeInfoFlags *infoFlagsOut);
};

extern void ui_video_avfoundation_set_mock_api(const struct ui_vt_api *mock);

static OSStatus mock_create_ret = noErr;
static OSStatus mock_decode_ret = noErr;
static VTDecompressionOutputCallback captured_output_callback = NULL;

static OSStatus my_VTDecompressionSessionCreate(
    CFAllocatorRef allocator,
    CMVideoFormatDescriptionRef videoFormatDescription,
    CFDictionaryRef videoDecoderSpecification,
    CFDictionaryRef destinationImageBufferAttributes,
    const VTDecompressionOutputCallbackRecord *outputCallback,
    VTDecompressionSessionRef *decompressionSessionOut) {
  (void)allocator;
  (void)videoFormatDescription;
  (void)videoDecoderSpecification;
  (void)destinationImageBufferAttributes;
  if (outputCallback) {
    captured_output_callback = outputCallback->decompressionOutputCallback;
  }
  if (mock_create_ret == noErr) {
    *decompressionSessionOut = (VTDecompressionSessionRef)0xcafebabe;
  } else {
    *decompressionSessionOut = NULL;
  }
  return mock_create_ret;
}

static void
my_VTDecompressionSessionInvalidate(VTDecompressionSessionRef session) {
  (void)session;
}

static OSStatus my_VTDecompressionSessionDecodeFrame(
    VTDecompressionSessionRef session, CMSampleBufferRef sampleBuffer,
    VTDecodeFrameFlags decodeFlags, void *sourceFrameRefCon,
    VTDecodeInfoFlags *infoFlagsOut) {
  (void)session;
  (void)sampleBuffer;
  (void)decodeFlags;
  (void)sourceFrameRefCon;
  (void)infoFlagsOut;
  return mock_decode_ret;
}

static struct ui_vt_api mock_api = {my_VTDecompressionSessionCreate,
                                    my_VTDecompressionSessionInvalidate,
                                    my_VTDecompressionSessionDecodeFrame};

static void reset_mocks(void) {
  mock_create_ret = noErr;
  mock_decode_ret = noErr;
  ui_video_avfoundation_set_mock_api(&mock_api);
}

TEST test_avf_get_backend_null(void) {
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_video_decoder_avfoundation_get_backend(NULL));
  PASS();
}

TEST test_avf_get_backend_success(void) {
  struct ui_video_decoder_backend backend;
  ASSERT_EQ(UI_ERROR_NONE, ui_video_decoder_avfoundation_get_backend(&backend));
  ASSERT_EQ(UI_ERROR_NONE, ui_video_decoder_get_default_backend(&backend));
  ASSERT(backend.create_decoder != NULL);
  PASS();
}

TEST test_avf_create_decoder_invalid_args(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;

  ui_video_decoder_avfoundation_get_backend(&backend);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.create_decoder(NULL, &config, &dec));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.create_decoder(&backend, NULL, &dec));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.create_decoder(&backend, &config, NULL));
  PASS();
}

TEST test_avf_create_decoder_success(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;

  reset_mocks();
  ui_video_decoder_avfoundation_get_backend(&backend);
  ASSERT_EQ(UI_ERROR_NONE, backend.create_decoder(&backend, &config, &dec));
  ASSERT(dec != NULL);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.destroy_decoder(NULL, dec));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.destroy_decoder(&backend, NULL));
  ASSERT_EQ(UI_ERROR_NONE, backend.destroy_decoder(&backend, dec));
  PASS();
}

TEST test_avf_create_decoder_failures(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;

  ui_video_decoder_avfoundation_get_backend(&backend);

  reset_mocks();
  mock_create_ret = -1;
  ASSERT_EQ(UI_ERROR_IO_FAILED,
            backend.create_decoder(&backend, &config, &dec));

  /* Test OOM */
  reset_mocks();
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY,
            backend.create_decoder(&backend, &config, &dec));
  g_malloc_fail_countdown = -1;

  /* Test set_mock_api(NULL) */
  ui_video_avfoundation_set_mock_api(NULL);
  ui_video_avfoundation_set_mock_api(&mock_api); /* restore */

  /* Test output_callback */
  reset_mocks();
  captured_output_callback = NULL;
  ASSERT_EQ(UI_ERROR_NONE, backend.create_decoder(&backend, &config, &dec));
  if (captured_output_callback) {
    CMTime dummy_time = {0, 0, 0, 0};
    captured_output_callback(NULL, NULL, 0, 0, NULL, dummy_time, dummy_time);
  }
  backend.destroy_decoder(&backend, dec);

  PASS();
}

TEST test_avf_decode_packet(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;
  char buf[4] = {0};

  reset_mocks();
  ui_video_decoder_avfoundation_get_backend(&backend);
  backend.create_decoder(&backend, &config, &dec);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.decode_packet(NULL, dec, buf, 1, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.decode_packet(&backend, NULL, buf, 1, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.decode_packet(&backend, dec, NULL, 1, 0));

  ASSERT_EQ(UI_ERROR_NONE, backend.decode_packet(&backend, dec, buf, 1, 0));

  mock_decode_ret = -1;
  ASSERT_EQ(UI_ERROR_IO_FAILED,
            backend.decode_packet(&backend, dec, buf, 1, 0));

  backend.destroy_decoder(&backend, dec);
  PASS();
}

TEST test_avf_get_release_frame(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;
  struct ui_video_frame frame = {0};

  reset_mocks();
  ui_video_decoder_avfoundation_get_backend(&backend);
  backend.create_decoder(&backend, &config, &dec);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.get_frame(NULL, dec, &frame));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.get_frame(&backend, NULL, &frame));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.get_frame(&backend, dec, NULL));

  ASSERT_EQ(UI_ERROR_QUEUE_EMPTY, backend.get_frame(&backend, dec, &frame));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.release_frame(NULL, dec, &frame));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.release_frame(&backend, NULL, &frame));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.release_frame(&backend, dec, NULL));

  ASSERT_EQ(UI_ERROR_NONE, backend.release_frame(&backend, dec, &frame));

  backend.destroy_decoder(&backend, dec);
  PASS();
}

SUITE(avf_suite) {
  RUN_TEST(test_avf_get_backend_null);
  RUN_TEST(test_avf_get_backend_success);
  RUN_TEST(test_avf_create_decoder_invalid_args);
  RUN_TEST(test_avf_create_decoder_success);
  RUN_TEST(test_avf_create_decoder_failures);
  RUN_TEST(test_avf_decode_packet);
  RUN_TEST(test_avf_get_release_frame);
}

#else

TEST test_avf_unsupported(void) {
  struct ui_video_decoder_backend backend;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_video_decoder_avfoundation_get_backend(NULL));
  ASSERT_EQ(UI_ERROR_UNSUPPORTED,
            ui_video_decoder_avfoundation_get_backend(&backend));
  PASS();
}

SUITE(avf_suite) { RUN_TEST(test_avf_unsupported); }

#endif

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(avf_suite);
  GREATEST_MAIN_END();
}
