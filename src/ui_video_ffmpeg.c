/**
 * @file ui_video_ffmpeg.c
 * @brief FFmpeg implementation for video decoder fallback.
 */

/* clang-format off */
#include "ui_video_decoder.h"
#include "ui_error.h"
#include "ui_types.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if (defined(__linux__) && !defined(__ANDROID__)) || defined(__EMSCRIPTEN__)
#include <dlfcn.h>
#endif
/* clang-format on */

#if (defined(__linux__) && !defined(__ANDROID__)) || defined(__EMSCRIPTEN__)

struct ui_ffmpeg_api {
  void *(*avcodec_alloc_context3)(const void *codec);
  void (*avcodec_free_context)(void **avctx);
  const void *(*avcodec_find_decoder)(int id);
  int (*avcodec_open2)(void *avctx, const void *codec, void **options);
  void *(*av_packet_alloc)(void);
  void (*av_packet_free)(void **pkt);
  int (*avcodec_send_packet)(void *avctx, const void *avpkt);
  void *(*av_frame_alloc)(void);
  void (*av_frame_free)(void **frame);
  int (*avcodec_receive_frame)(void *avctx, void *frame);
};

static struct ui_ffmpeg_api g_ff = {0};
static int g_ff_loaded = 0;
static void *g_ff_handle = NULL;

ui_error_t ui_video_ffmpeg_load_api(void);
ui_error_t ui_video_ffmpeg_load_api(void) {
  if (g_ff_loaded) {
    return UI_ERROR_NONE;
  }

  g_ff_handle = dlopen("libavcodec.so", RTLD_LAZY);
  if (!g_ff_handle) {
    g_ff_handle = dlopen("libavcodec.so.58", RTLD_LAZY);
    if (!g_ff_handle) {
      g_ff_handle = dlopen("libavcodec.so.59", RTLD_LAZY);
      if (!g_ff_handle) {
        return UI_ERROR_UNSUPPORTED;
      }
    }
  }

#define LOAD_SYM(name)                                                         \
  do {                                                                         \
    g_ff.name = (void *)dlsym(g_ff_handle, #name);                             \
    if (!g_ff.name) {                                                          \
      dlclose(g_ff_handle);                                                    \
      g_ff_handle = NULL;                                                      \
      return UI_ERROR_UNSUPPORTED;                                             \
    }                                                                          \
  } while (0)

  LOAD_SYM(avcodec_alloc_context3);
  LOAD_SYM(avcodec_free_context);
  LOAD_SYM(avcodec_find_decoder);
  LOAD_SYM(avcodec_open2);
  LOAD_SYM(av_packet_alloc);
  LOAD_SYM(av_packet_free);
  LOAD_SYM(avcodec_send_packet);
  LOAD_SYM(av_frame_alloc);
  LOAD_SYM(av_frame_free);
  LOAD_SYM(avcodec_receive_frame);

#undef LOAD_SYM

  g_ff_loaded = 1;
  return UI_ERROR_NONE;
}

void ui_video_ffmpeg_set_mock_api(const struct ui_ffmpeg_api *mock);
void ui_video_ffmpeg_set_mock_api(const struct ui_ffmpeg_api *mock) {
  if (mock) {
    g_ff = *mock;
    g_ff_loaded = 1;
  } else {
    memset(&g_ff, 0, sizeof(g_ff));
    g_ff_loaded = 0;
  }
}

struct ui_video_decoder {
  void *ctx;
  void *pkt;
  void *frame;
  int width;
  int height;
};

static ui_error_t
ff_create_decoder(struct ui_video_decoder_backend *backend,
                  const struct ui_video_decoder_config *config,
                  struct ui_video_decoder **out_decoder) {
  struct ui_video_decoder *dec = NULL;
  const void *codec = NULL;
  ui_error_t rc;

  if (!backend || !config || !out_decoder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_video_ffmpeg_load_api();
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  dec = (struct ui_video_decoder *)malloc(sizeof(struct ui_video_decoder));
  if (!dec) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  dec->ctx = NULL;
  dec->pkt = NULL;
  dec->frame = NULL;
  dec->width = config->width;
  dec->height = config->height;

  codec = g_ff.avcodec_find_decoder(config->codec_id);
  if (!codec) {
    free(dec);
    return UI_ERROR_UNSUPPORTED;
  }

  dec->ctx = g_ff.avcodec_alloc_context3(codec);
  if (!dec->ctx) {
    free(dec);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  if (g_ff.avcodec_open2(dec->ctx, codec, NULL) < 0) {
    g_ff.avcodec_free_context(&dec->ctx);
    free(dec);
    return UI_ERROR_IO_FAILED;
  }

  dec->pkt = g_ff.av_packet_alloc();
  if (!dec->pkt) {
    g_ff.avcodec_free_context(&dec->ctx);
    free(dec);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  dec->frame = g_ff.av_frame_alloc();
  if (!dec->frame) {
    g_ff.av_packet_free(&dec->pkt);
    g_ff.avcodec_free_context(&dec->ctx);
    free(dec);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  *out_decoder = dec;
  return UI_ERROR_NONE;
}

static ui_error_t ff_destroy_decoder(struct ui_video_decoder_backend *backend,
                                     struct ui_video_decoder *decoder) {
  if (!backend || !decoder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (decoder->frame) {
    g_ff.av_frame_free(&decoder->frame);
  }
  if (decoder->pkt) {
    g_ff.av_packet_free(&decoder->pkt);
  }
  if (decoder->ctx) {
    g_ff.avcodec_free_context(&decoder->ctx);
  }
  free(decoder);
  return UI_ERROR_NONE;
}

static ui_error_t ff_decode_packet(struct ui_video_decoder_backend *backend,
                                   struct ui_video_decoder *decoder,
                                   const void *packet_data, size_t packet_size,
                                   ui_int64 pts) {
  if (!backend || !decoder || !packet_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  (void)packet_size;
  (void)pts;

  /* we would populate pkt here and send it */
  if (g_ff.avcodec_send_packet(decoder->ctx, decoder->pkt) < 0) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

static ui_error_t ff_get_frame(struct ui_video_decoder_backend *backend,
                               struct ui_video_decoder *decoder,
                               struct ui_video_frame *out_frame) {
  int ret;
  if (!backend || !decoder || !out_frame) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ret = g_ff.avcodec_receive_frame(decoder->ctx, decoder->frame);
  if (ret < 0) {
    return UI_ERROR_QUEUE_EMPTY;
  }
  /* we would extract data out to out_frame */
  return UI_ERROR_NONE;
}

static ui_error_t ff_release_frame(struct ui_video_decoder_backend *backend,
                                   struct ui_video_decoder *decoder,
                                   struct ui_video_frame *frame) {
  if (!backend || !decoder || !frame) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_video_decoder_ffmpeg_get_backend(
    struct ui_video_decoder_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_decoder = ff_create_decoder;
  out_backend->destroy_decoder = ff_destroy_decoder;
  out_backend->decode_packet = ff_decode_packet;
  out_backend->get_frame = ff_get_frame;
  out_backend->release_frame = ff_release_frame;
  out_backend->user_data = NULL;

  return UI_ERROR_NONE;
}

ui_error_t ui_video_decoder_get_default_backend(
    struct ui_video_decoder_backend *out_backend) {
  return ui_video_decoder_ffmpeg_get_backend(out_backend);
}

#else

ui_error_t ui_video_decoder_ffmpeg_get_backend(
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

#endif /* defined(__linux__) || defined(__EMSCRIPTEN__) */
