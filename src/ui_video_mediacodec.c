/**
 * @file ui_video_mediacodec.c
 * @brief MediaCodec implementation for video decoder.
 */

/* clang-format off */
#include "ui_video_decoder.h"
#include "ui_error.h"
#include "ui_types.h"

#if defined(__ANDROID__)
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#endif
/* clang-format on */

#if defined(__ANDROID__)

struct ui_video_decoder {
  AMediaCodec *codec;
  AMediaFormat *format;
  int width;
  int height;
};

static ui_error_t
mc_create_decoder(struct ui_video_decoder_backend *backend,
                  const struct ui_video_decoder_config *config,
                  struct ui_video_decoder **out_decoder) {
  struct ui_video_decoder *dec = NULL;
  media_status_t status;

  if (!backend || !config || !out_decoder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dec = (struct ui_video_decoder *)malloc(sizeof(struct ui_video_decoder));
  if (!dec) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  dec->codec = NULL;
  dec->format = NULL;
  dec->width = config->width;
  dec->height = config->height;

  dec->codec = AMediaCodec_createDecoderByType("video/avc"); /* simplified */
  if (!dec->codec) {
    free(dec);
    return UI_ERROR_UNSUPPORTED;
  }

  dec->format = AMediaFormat_new();
  if (!dec->format) {
    AMediaCodec_delete(dec->codec);
    free(dec);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  AMediaFormat_setString(dec->format, AMEDIAFORMAT_KEY_MIME, "video/avc");
  AMediaFormat_setInt32(dec->format, AMEDIAFORMAT_KEY_WIDTH, config->width);
  AMediaFormat_setInt32(dec->format, AMEDIAFORMAT_KEY_HEIGHT, config->height);

  status = AMediaCodec_configure(dec->codec, dec->format, NULL, NULL, 0);
  if (status != AMEDIA_OK) {
    AMediaFormat_delete(dec->format);
    AMediaCodec_delete(dec->codec);
    free(dec);
    return UI_ERROR_IO_FAILED;
  }

  status = AMediaCodec_start(dec->codec);
  if (status != AMEDIA_OK) {
    AMediaFormat_delete(dec->format);
    AMediaCodec_delete(dec->codec);
    free(dec);
    return UI_ERROR_IO_FAILED;
  }

  *out_decoder = dec;
  return UI_ERROR_NONE;
}

static ui_error_t mc_destroy_decoder(struct ui_video_decoder_backend *backend,
                                     struct ui_video_decoder *decoder) {
  if (!backend || !decoder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (decoder->codec) {
    AMediaCodec_stop(decoder->codec);
    AMediaCodec_delete(decoder->codec);
  }
  if (decoder->format) {
    AMediaFormat_delete(decoder->format);
  }
  free(decoder);
  return UI_ERROR_NONE;
}

static ui_error_t mc_decode_packet(struct ui_video_decoder_backend *backend,
                                   struct ui_video_decoder *decoder,
                                   const void *packet_data, size_t packet_size,
                                   ui_int64 pts) {
  ssize_t buf_idx;
  size_t buf_size;
  uint8_t *buf;

  if (!backend || !decoder || !packet_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  buf_idx = AMediaCodec_dequeueInputBuffer(decoder->codec, 2000);
  if (buf_idx < 0) {
    return UI_ERROR_QUEUE_FULL;
  }

  buf = AMediaCodec_getInputBuffer(decoder->codec, buf_idx, &buf_size);
  if (!buf) {
    return UI_ERROR_IO_FAILED;
  }

  if (packet_size > buf_size) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  memcpy(buf, packet_data, packet_size);

  if (AMediaCodec_queueInputBuffer(decoder->codec, buf_idx, 0, packet_size, pts,
                                   0) != AMEDIA_OK) {
    return UI_ERROR_IO_FAILED;
  }

  return UI_ERROR_NONE;
}

static ui_error_t mc_get_frame(struct ui_video_decoder_backend *backend,
                               struct ui_video_decoder *decoder,
                               struct ui_video_frame *out_frame) {
  AMediaCodecBufferInfo info;
  ssize_t buf_idx;
  if (!backend || !decoder || !out_frame) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  buf_idx = AMediaCodec_dequeueOutputBuffer(decoder->codec, &info, 0);
  if (buf_idx >= 0) {
    /* stub extraction */
    out_frame->pts = info.presentationTimeUs;
    /* Normally we would extract the buffer here or release it for rendering.
       Since this is a stub we just store the index in user_data or so,
       but we can't modify frame easily without an index. We use data[0]. */
    out_frame->data[0] = (void *)(intptr_t)buf_idx;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_QUEUE_EMPTY;
}

static ui_error_t mc_release_frame(struct ui_video_decoder_backend *backend,
                                   struct ui_video_decoder *decoder,
                                   struct ui_video_frame *frame) {
  ssize_t buf_idx;
  if (!backend || !decoder || !frame) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  buf_idx = (ssize_t)(intptr_t)frame->data[0];
  AMediaCodec_releaseOutputBuffer(decoder->codec, buf_idx, false);
  return UI_ERROR_NONE;
}

ui_error_t ui_video_decoder_mediacodec_get_backend(
    struct ui_video_decoder_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_decoder = mc_create_decoder;
  out_backend->destroy_decoder = mc_destroy_decoder;
  out_backend->decode_packet = mc_decode_packet;
  out_backend->get_frame = mc_get_frame;
  out_backend->release_frame = mc_release_frame;
  out_backend->user_data = NULL;

  return UI_ERROR_NONE;
}

ui_error_t ui_video_decoder_get_default_backend(
    struct ui_video_decoder_backend *out_backend) {
  return ui_video_decoder_mediacodec_get_backend(out_backend);
}

#else

ui_error_t ui_video_decoder_mediacodec_get_backend(
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

#endif /* defined(__ANDROID__) */
