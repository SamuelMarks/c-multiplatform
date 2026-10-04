/**
 * @file ui_video_avfoundation.c
 * @brief AVFoundation/VideoToolbox implementation for video decoder.
 */

/* clang-format off */
#include "ui_video_decoder.h"
#include "ui_error.h"
#include "ui_types.h"

#if defined(__APPLE__)
#ifndef inline
#define inline __inline
#endif
#include <VideoToolbox/VideoToolbox.h>
#include <CoreMedia/CoreMedia.h>
#undef inline

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#endif
/* clang-format on */

#include "ui_internal_mem.h"

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

static const struct ui_vt_api g_real_vt = {VTDecompressionSessionCreate,
                                           VTDecompressionSessionInvalidate,
                                           VTDecompressionSessionDecodeFrame};

static struct ui_vt_api g_vt = {VTDecompressionSessionCreate,
                                VTDecompressionSessionInvalidate,
                                VTDecompressionSessionDecodeFrame};

void ui_video_avfoundation_set_mock_api(const struct ui_vt_api *mock);
void ui_video_avfoundation_set_mock_api(const struct ui_vt_api *mock) {
  if (mock) {
    g_vt = *mock;
  } else {
    g_vt = g_real_vt;
  }
}

struct ui_video_decoder {
  VTDecompressionSessionRef session;
  int width;
  int height;
};

static void output_callback(void *decompressionOutputRefCon,
                            void *sourceFrameRefCon, OSStatus status,
                            VTDecodeInfoFlags infoFlags,
                            CVImageBufferRef imageBuffer,
                            CMTime presentationTimeStamp,
                            CMTime presentationDuration) {
  (void)decompressionOutputRefCon;
  (void)sourceFrameRefCon;
  (void)status;
  (void)infoFlags;
  (void)imageBuffer;
  (void)presentationTimeStamp;
  (void)presentationDuration;
}

static ui_error_t
avf_create_decoder(struct ui_video_decoder_backend *backend,
                   const struct ui_video_decoder_config *config,
                   struct ui_video_decoder **out_decoder) {
  struct ui_video_decoder *dec = NULL;
  OSStatus err;
  VTDecompressionOutputCallbackRecord cb;

  if (!backend || !config || !out_decoder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dec = (struct ui_video_decoder *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_video_decoder));
  if (!dec) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  dec->session = NULL;
  dec->width = config->width;
  dec->height = config->height;

  cb.decompressionOutputCallback = output_callback;
  cb.decompressionOutputRefCon = dec;

  err = g_vt.VTDecompressionSessionCreate(NULL, NULL, NULL, NULL, &cb,
                                          &dec->session);
  if (err != noErr || !dec->session) {
    C_MULTIPLATFORM_FREE(dec);
    return UI_ERROR_IO_FAILED;
  }

  *out_decoder = dec;
  return UI_ERROR_NONE;
}

static ui_error_t avf_destroy_decoder(struct ui_video_decoder_backend *backend,
                                      struct ui_video_decoder *decoder) {
  if (!backend || !decoder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (decoder->session) {
    g_vt.VTDecompressionSessionInvalidate(decoder->session);
  }
  C_MULTIPLATFORM_FREE(decoder);
  return UI_ERROR_NONE;
}

static ui_error_t avf_decode_packet(struct ui_video_decoder_backend *backend,
                                    struct ui_video_decoder *decoder,
                                    const void *packet_data, size_t packet_size,
                                    ui_int64 pts) {
  OSStatus err;
  if (!backend || !decoder || !packet_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)packet_size;
  (void)pts;

  err = g_vt.VTDecompressionSessionDecodeFrame(decoder->session, NULL, 0, NULL,
                                               NULL);
  if (err != noErr) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

static ui_error_t avf_get_frame(struct ui_video_decoder_backend *backend,
                                struct ui_video_decoder *decoder,
                                struct ui_video_frame *out_frame) {
  if (!backend || !decoder || !out_frame) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* Stub: no frames queued */
  return UI_ERROR_QUEUE_EMPTY;
}

static ui_error_t avf_release_frame(struct ui_video_decoder_backend *backend,
                                    struct ui_video_decoder *decoder,
                                    struct ui_video_frame *frame) {
  if (!backend || !decoder || !frame) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_video_decoder_avfoundation_get_backend(
    struct ui_video_decoder_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_decoder = avf_create_decoder;
  out_backend->destroy_decoder = avf_destroy_decoder;
  out_backend->decode_packet = avf_decode_packet;
  out_backend->get_frame = avf_get_frame;
  out_backend->release_frame = avf_release_frame;
  out_backend->user_data = NULL;

  return UI_ERROR_NONE;
}

ui_error_t ui_video_decoder_get_default_backend(
    struct ui_video_decoder_backend *out_backend) {
  return ui_video_decoder_avfoundation_get_backend(out_backend);
}

#else

ui_error_t ui_video_decoder_avfoundation_get_backend(
    struct ui_video_decoder_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_decoder = NULL;
  out_backend->destroy_decoder = NULL;
  out_backend->decode_packet = NULL;
  out_backend->get_frame = NULL;
  out_backend->release_frame = NULL;
  out_backend->user_data = NULL;

  return UI_ERROR_UNSUPPORTED;
}

#endif /* defined(__APPLE__) */
