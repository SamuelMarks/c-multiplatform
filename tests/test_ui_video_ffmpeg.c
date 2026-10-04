/* clang-format off */
#include "greatest.h"
#include "../include/ui_video_decoder.h"
#include "../include/ui_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

extern void ui_video_ffmpeg_set_mock_api(const struct ui_ffmpeg_api *mock);

static int mock_find_ret = 1;
static int mock_alloc_ret = 1;
static int mock_open_ret = 0;
static int mock_pkt_alloc_ret = 1;
static int mock_frame_alloc_ret = 1;
static int mock_send_ret = 0;
static int mock_recv_ret = 0;

static void *my_avcodec_alloc_context3(const void *codec) {
  (void)codec;
  if (mock_alloc_ret)
    return (void *)0xcafebabe;
  return NULL;
}
static void my_avcodec_free_context(void **avctx) {
  if (avctx)
    *avctx = NULL;
}
static const void *my_avcodec_find_decoder(int id) {
  (void)id;
  if (mock_find_ret)
    return (const void *)0xdeadbeef;
  return NULL;
}
static int my_avcodec_open2(void *avctx, const void *codec, void **options) {
  (void)avctx;
  (void)codec;
  (void)options;
  return mock_open_ret;
}
static void *my_av_packet_alloc(void) {
  if (mock_pkt_alloc_ret)
    return (void *)0xbeefcafe;
  return NULL;
}
static void my_av_packet_free(void **pkt) {
  if (pkt)
    *pkt = NULL;
}
static int my_avcodec_send_packet(void *avctx, const void *avpkt) {
  (void)avctx;
  (void)avpkt;
  return mock_send_ret;
}
static void *my_av_frame_alloc(void) {
  if (mock_frame_alloc_ret)
    return (void *)0xbadc0fee;
  return NULL;
}
static void my_av_frame_free(void **frame) {
  if (frame)
    *frame = NULL;
}
static int my_avcodec_receive_frame(void *avctx, void *frame) {
  (void)avctx;
  (void)frame;
  return mock_recv_ret;
}

static struct ui_ffmpeg_api mock_api = {
    my_avcodec_alloc_context3, my_avcodec_free_context, my_avcodec_find_decoder,
    my_avcodec_open2,          my_av_packet_alloc,      my_av_packet_free,
    my_avcodec_send_packet,    my_av_frame_alloc,       my_av_frame_free,
    my_avcodec_receive_frame};

static void reset_mocks(void) {
  mock_find_ret = 1;
  mock_alloc_ret = 1;
  mock_open_ret = 0;
  mock_pkt_alloc_ret = 1;
  mock_frame_alloc_ret = 1;
  mock_send_ret = 0;
  mock_recv_ret = 0;
  ui_video_ffmpeg_set_mock_api(&mock_api);
}

TEST test_ffmpeg_get_backend_null(void) {
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_video_decoder_ffmpeg_get_backend(NULL));
  PASS();
}

TEST test_ffmpeg_get_backend_success(void) {
  struct ui_video_decoder_backend backend;
  ASSERT_EQ(UI_ERROR_NONE, ui_video_decoder_ffmpeg_get_backend(&backend));
  ASSERT_EQ(UI_ERROR_NONE, ui_video_decoder_get_default_backend(&backend));
  ASSERT(backend.create_decoder != NULL);
  PASS();
}

TEST test_ffmpeg_create_decoder_invalid_args(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;

  ui_video_decoder_ffmpeg_get_backend(&backend);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.create_decoder(NULL, &config, &dec));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.create_decoder(&backend, NULL, &dec));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.create_decoder(&backend, &config, NULL));
  PASS();
}

TEST test_ffmpeg_create_decoder_success(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;

  reset_mocks();
  ui_video_decoder_ffmpeg_get_backend(&backend);
  ASSERT_EQ(UI_ERROR_NONE, backend.create_decoder(&backend, &config, &dec));
  ASSERT(dec != NULL);

  ASSERT_EQ(UI_ERROR_NONE, backend.destroy_decoder(&backend, dec));
  PASS();
}

TEST test_ffmpeg_create_decoder_failures(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;

  ui_video_decoder_ffmpeg_get_backend(&backend);

  reset_mocks();
  mock_find_ret = 0;
  ASSERT_EQ(UI_ERROR_UNSUPPORTED,
            backend.create_decoder(&backend, &config, &dec));

  reset_mocks();
  mock_alloc_ret = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY,
            backend.create_decoder(&backend, &config, &dec));

  reset_mocks();
  mock_open_ret = -1;
  ASSERT_EQ(UI_ERROR_IO_FAILED,
            backend.create_decoder(&backend, &config, &dec));

  reset_mocks();
  mock_pkt_alloc_ret = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY,
            backend.create_decoder(&backend, &config, &dec));

  reset_mocks();
  mock_frame_alloc_ret = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY,
            backend.create_decoder(&backend, &config, &dec));

  PASS();
}

TEST test_ffmpeg_decode_packet(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;
  char buf[4] = {0};

  reset_mocks();
  ui_video_decoder_ffmpeg_get_backend(&backend);
  backend.create_decoder(&backend, &config, &dec);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.decode_packet(NULL, dec, buf, 1, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.decode_packet(&backend, NULL, buf, 1, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.decode_packet(&backend, dec, NULL, 1, 0));

  ASSERT_EQ(UI_ERROR_NONE, backend.decode_packet(&backend, dec, buf, 1, 0));

  mock_send_ret = -1;
  ASSERT_EQ(UI_ERROR_IO_FAILED,
            backend.decode_packet(&backend, dec, buf, 1, 0));

  backend.destroy_decoder(&backend, dec);
  PASS();
}

TEST test_ffmpeg_get_release_frame(void) {
  struct ui_video_decoder_backend backend;
  struct ui_video_decoder_config config = {1920, 1080, 0};
  struct ui_video_decoder *dec = NULL;
  struct ui_video_frame frame = {0};

  reset_mocks();
  ui_video_decoder_ffmpeg_get_backend(&backend);
  backend.create_decoder(&backend, &config, &dec);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.get_frame(NULL, dec, &frame));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            backend.get_frame(&backend, NULL, &frame));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, backend.get_frame(&backend, dec, NULL));

  mock_recv_ret = -11; /* EAGAIN */
  ASSERT_EQ(UI_ERROR_QUEUE_EMPTY, backend.get_frame(&backend, dec, &frame));

  mock_recv_ret = 0; /* SUCCESS */
  ASSERT_EQ(UI_ERROR_NONE, backend.get_frame(&backend, dec, &frame));

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

SUITE(ffmpeg_suite) {
  RUN_TEST(test_ffmpeg_get_backend_null);
  RUN_TEST(test_ffmpeg_get_backend_success);
  RUN_TEST(test_ffmpeg_create_decoder_invalid_args);
  RUN_TEST(test_ffmpeg_create_decoder_success);
  RUN_TEST(test_ffmpeg_create_decoder_failures);
  RUN_TEST(test_ffmpeg_decode_packet);
  RUN_TEST(test_ffmpeg_get_release_frame);
}

#else

TEST test_ffmpeg_unsupported(void) {
  struct ui_video_decoder_backend backend;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_video_decoder_ffmpeg_get_backend(NULL));
  ASSERT_EQ(UI_ERROR_UNSUPPORTED,
            ui_video_decoder_ffmpeg_get_backend(&backend));
  PASS();
}

SUITE(ffmpeg_suite) { RUN_TEST(test_ffmpeg_unsupported); }

#endif

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ffmpeg_suite);
  GREATEST_MAIN_END();
}
