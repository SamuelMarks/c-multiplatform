/* clang-format off */
#include "ui_media.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#if defined(_MSC_VER)
#define NUM_FORMAT "%I64d"
#else
#define NUM_FORMAT "%lld"
#endif

/* Dummy media source implementation for testing */
struct dummy_user_data {
  int packets_read;
  int max_packets;
};

static enum ui_error dummy_open(struct ui_media_source *source,
                                const char *uri) {
  struct dummy_user_data *data;
  if (!source || !uri)
    return UI_ERROR_INVALID_ARGUMENT;

  if (g_malloc_fail_countdown == 0) {
    return UI_ERROR_OUT_OF_MEMORY;
    return UI_ERROR_NONE;
  }
  if (g_malloc_fail_countdown > 0) {
    g_malloc_fail_countdown--;
  }

  data = (struct dummy_user_data *)malloc(sizeof(struct dummy_user_data));
  if (!data)
    return UI_ERROR_OUT_OF_MEMORY;

  data->packets_read = 0;
  data->max_packets = 5;

  source->user_data = data;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_read_packet(struct ui_media_source *source,
                                       struct ui_media_packet *out_packet) {
  struct dummy_user_data *data;
  if (!source || !out_packet || !source->user_data)
    return UI_ERROR_INVALID_ARGUMENT;

  data = (struct dummy_user_data *)source->user_data;

  if (data->packets_read >= data->max_packets) {
    return UI_ERROR_EOF;
    return UI_ERROR_NONE;
  }

  out_packet->data = NULL;
  out_packet->size = 1024;
  out_packet->pts = data->packets_read * 1000;
  out_packet->dts = data->packets_read * 1000;
  out_packet->stream_index = 0;
  out_packet->is_keyframe = (data->packets_read == 0) ? 1 : 0;

  data->packets_read++;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_seek(struct ui_media_source *source,
                                ui_int64 timestamp_us) {
  struct dummy_user_data *data;
  if (!source || !source->user_data)
    return UI_ERROR_INVALID_ARGUMENT;

  data = (struct dummy_user_data *)source->user_data;
  data->packets_read = (int)(timestamp_us / 1000);
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_get_duration(struct ui_media_source *source,
                                        ui_int64 *out_duration_us) {
  struct dummy_user_data *data;
  if (!source || !source->user_data || !out_duration_us)
    return UI_ERROR_INVALID_ARGUMENT;

  data = (struct dummy_user_data *)source->user_data;
  *out_duration_us = (ui_int64)data->max_packets * 1000;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_close(struct ui_media_source *source) {
  if (!source)
    return UI_ERROR_INVALID_ARGUMENT;
  if (source->user_data) {
    free(source->user_data);
    source->user_data = NULL;
    return UI_ERROR_NONE;
  }
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_media_source source;
  struct ui_media_packet pkt;
  ui_int64 duration = 0;
  enum ui_error err;

  source.open = dummy_open;
  source.read_packet = dummy_read_packet;
  source.seek = dummy_seek;
  source.get_duration = dummy_get_duration;
  source.close = dummy_close;
  source.user_data = NULL;

  /* Invalid arguments */
  if (source.open(NULL, "dummy://test") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (source.open(&source, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (source.read_packet(NULL, &pkt) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (source.read_packet(&source, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (source.seek(NULL, 3000) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (source.get_duration(NULL, &duration) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (source.get_duration(&source, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (source.close(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Need to ensure source.user_data is null for these to fail */
  if (source.read_packet(&source, &pkt) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (source.seek(&source, 3000) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (source.get_duration(&source, &duration) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = source.open(&source, "dummy://test");
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  err = source.get_duration(&source, &duration);
  if (err != UI_ERROR_NONE || duration != 5000) {
    source.close(&source);
    return 1;
  }

  err = source.read_packet(&source, &pkt);
  if (err != UI_ERROR_NONE || pkt.pts != 0 || pkt.is_keyframe != 1) {
    source.close(&source);
    return 1;
  }

  err = source.read_packet(&source, &pkt);
  if (err != UI_ERROR_NONE || pkt.pts != 1000 || pkt.is_keyframe != 0) {
    source.close(&source);
    return 1;
  }

  err = source.seek(&source, 3000);
  if (err != UI_ERROR_NONE) {
    source.close(&source);
    return 1;
  }

  err = source.read_packet(&source, &pkt);
  if (err != UI_ERROR_NONE || pkt.pts != 3000) {
    source.close(&source);
    return 1;
  }

  /* Exhaust the stream to test EOF */
  source.read_packet(&source, &pkt); /* 4000 */
  source.read_packet(&source, &pkt); /* 5000 / EOF */

  err = source.read_packet(&source, &pkt);
  if (err != UI_ERROR_EOF) {
    source.close(&source);
    return 1;
  }

  source.close(&source);
  return 0;
}

static int run_oom_tests(void) {
  struct ui_media_source source;
  enum ui_error err;

  source.open = dummy_open;
  source.read_packet = dummy_read_packet;
  source.seek = dummy_seek;
  source.get_duration = dummy_get_duration;
  source.close = dummy_close;
  source.user_data = NULL;

  printf("Running media OOM tests...\n");

  /* Creation OOM */
  g_malloc_fail_countdown = 0;
  err = source.open(&source, "dummy://test");
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All test_ui_media passed.\n");
  return 0;
}
