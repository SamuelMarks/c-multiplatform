#include "cmpc/cmp_video.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestVideoState {
  int fail_open;
  int fail_close;
  int fail_read;
  int open_calls;
  int close_calls;
  int read_calls;
  CMPBool opened;
  CMPVideoFrame frame;
  CMPBool has_frame;
} TestVideoState;

static int test_video_state_reset(TestVideoState *state) {
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(state, 0, sizeof(*state));
  return CMP_OK;
}

static int test_video_open(void *video, const CMPVideoOpenRequest *request) {
  TestVideoState *state;

  if (video == NULL || request == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestVideoState *)video;
  state->open_calls += 1;
  if (state->fail_open) {
    return CMP_ERR_IO;
  }
  if (state->opened == CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  state->opened = CMP_TRUE;
  return CMP_OK;
}

static int test_video_close(void *video) {
  TestVideoState *state;

  if (video == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestVideoState *)video;
  state->close_calls += 1;
  if (state->fail_close) {
    return CMP_ERR_IO;
  }
  if (state->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  state->opened = CMP_FALSE;
  return CMP_OK;
}

static int test_video_read(void *video, CMPVideoFrame *out_frame,
                           CMPBool *out_has_frame) {
  TestVideoState *state;

  if (video == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestVideoState *)video;
  state->read_calls += 1;
  if (state->fail_read) {
    return CMP_ERR_IO;
  }
  if (state->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  if (state->has_frame == CMP_TRUE) {
    *out_frame = state->frame;
    *out_has_frame = CMP_TRUE;
  } else {
    memset(out_frame, 0, sizeof(*out_frame));
    *out_has_frame = CMP_FALSE;
  }

  return CMP_OK;
}

static const CMPVideoVTable g_test_video_vtable = {test_video_open,
                                                  test_video_close,
                                                  test_video_read};

typedef struct TestEnvState {
  CMPEnv env;
  CMPVideo video;
  int fail_get_video;
} TestEnvState;

static int test_env_get_video(void *env, CMPVideo *out_video) {
  TestEnvState *state;

  if (env == NULL || out_video == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestEnvState *)env;
  if (state->fail_get_video) {
    return CMP_ERR_IO;
  }

  *out_video = state->video;
  return CMP_OK;
}

static const CMPEnvVTable g_test_env_vtable = {NULL, NULL, NULL, NULL,
                                              test_env_get_video, NULL, NULL,
                                              NULL, NULL};

static int test_env_reset(TestEnvState *env_state, TestVideoState *video_state,
                          const CMPVideoVTable *vtable) {
  if (env_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(env_state, 0, sizeof(*env_state));
  env_state->env.ctx = env_state;
  env_state->env.vtable = &g_test_env_vtable;

  if (video_state != NULL && vtable != NULL) {
    env_state->video.ctx = video_state;
    env_state->video.vtable = vtable;
  }

  return CMP_OK;
}

static int test_video_config_init(void) {
  CMPVideoConfig config;

  CMP_TEST_EXPECT(cmp_video_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_video_config_init(&config));
  CMP_TEST_ASSERT(config.env == NULL);
  CMP_TEST_ASSERT(config.allocator == NULL);
  return 0;
}

static int test_video_request_init(void) {
  CMPVideoOpenRequest request;

  CMP_TEST_EXPECT(cmp_video_request_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_video_request_init(&request));
  CMP_TEST_ASSERT(request.utf8_path == NULL);
  CMP_TEST_ASSERT(request.data == NULL);
  CMP_TEST_ASSERT(request.size == 0u);
  CMP_TEST_ASSERT(request.encoding == CMP_VIDEO_ENCODING_AUTO);
  CMP_TEST_ASSERT(request.format == CMP_VIDEO_FORMAT_ANY);
  return 0;
}

static int test_video_init_and_shutdown(void) {
  CMPVideoDecoder decoder;
  CMPVideoConfig config;
  CMPAllocator bad_alloc;
  TestEnvState env_state;
  TestVideoState video_state;

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_video_init(NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_video_config_init(&config));
  CMP_TEST_EXPECT(cmp_video_init(NULL, &config), CMP_ERR_INVALID_ARGUMENT);

  memset(&bad_alloc, 0, sizeof(bad_alloc));
  config.allocator = &bad_alloc;
  CMP_TEST_EXPECT(cmp_video_init(&decoder, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_video_config_init(&config));
  config.env = NULL;
  CMP_TEST_OK(cmp_video_init(&decoder, &config));
  CMP_TEST_OK(cmp_video_shutdown(&decoder));

  CMP_TEST_OK(test_video_state_reset(&video_state));
  CMP_TEST_OK(test_env_reset(&env_state, &video_state, &g_test_video_vtable));
  config.env = &env_state.env;
  env_state.fail_get_video = 1;
  CMP_TEST_EXPECT(cmp_video_init(&decoder, &config), CMP_ERR_IO);

  env_state.fail_get_video = 0;
  CMP_TEST_OK(cmp_video_init(&decoder, &config));
  CMP_TEST_OK(cmp_video_shutdown(&decoder));
  return 0;
}

static int test_video_backend_path(void) {
  CMPVideoDecoder decoder;
  CMPVideoConfig config;
  CMPVideoOpenRequest request;
  CMPVideoFrame frame;
  CMPBool has_frame;
  TestEnvState env_state;
  TestVideoState video_state;
  cmp_u8 dummy[1];

  CMP_TEST_OK(test_video_state_reset(&video_state));
  video_state.has_frame = CMP_TRUE;
  video_state.frame.format = CMP_VIDEO_FORMAT_RGBA8;
  video_state.frame.width = 1u;
  video_state.frame.height = 1u;
  video_state.frame.size = 4u;
  video_state.frame.timestamp_ms = 0u;
  video_state.frame.data = dummy;

  CMP_TEST_OK(test_env_reset(&env_state, &video_state, &g_test_video_vtable));
  CMP_TEST_OK(cmp_video_config_init(&config));
  config.env = &env_state.env;

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_video_init(&decoder, &config));

  CMP_TEST_OK(cmp_video_request_init(&request));
  request.data = dummy;
  request.size = sizeof(dummy);

  CMP_TEST_OK(cmp_video_open(&decoder, &request));
  CMP_TEST_OK(cmp_video_read_frame(&decoder, &frame, &has_frame));
  CMP_TEST_ASSERT(has_frame == CMP_TRUE);
  CMP_TEST_ASSERT(video_state.read_calls == 1);
  CMP_TEST_OK(cmp_video_close(&decoder));

  CMP_TEST_OK(cmp_video_shutdown(&decoder));
  return 0;
}

static int test_video_fallback_m3v0(void) {
  CMPVideoDecoder decoder;
  CMPVideoConfig config;
  CMPVideoOpenRequest request;
  CMPVideoFrame frame;
  CMPBool has_frame;
  static const cmp_u8 video_data[] = {
      'M', '3', 'V', '0',
      1u, 0u, 0u, 0u,
      1u, 0u, 0u, 0u,
      1u, 0u, 0u, 0u,
      30u, 0u, 0u, 0u,
      1u, 0u, 0u, 0u,
      2u, 0u, 0u, 0u,
      1u, 2u, 3u, 4u,
      5u, 6u, 7u, 8u};

  CMP_TEST_OK(cmp_video_config_init(&config));
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_video_init(&decoder, &config));

  CMP_TEST_OK(cmp_video_request_init(&request));
  request.data = video_data;
  request.size = sizeof(video_data);

  CMP_TEST_OK(cmp_video_open(&decoder, &request));

  CMP_TEST_OK(cmp_video_read_frame(&decoder, &frame, &has_frame));
  CMP_TEST_ASSERT(has_frame == CMP_TRUE);
  CMP_TEST_ASSERT(((const cmp_u8 *)frame.data)[0] == 1u);

  CMP_TEST_OK(cmp_video_read_frame(&decoder, &frame, &has_frame));
  CMP_TEST_ASSERT(has_frame == CMP_TRUE);
  CMP_TEST_ASSERT(((const cmp_u8 *)frame.data)[0] == 5u);

  CMP_TEST_OK(cmp_video_read_frame(&decoder, &frame, &has_frame));
  CMP_TEST_ASSERT(has_frame == CMP_FALSE);

  CMP_TEST_OK(cmp_video_close(&decoder));
  CMP_TEST_OK(cmp_video_shutdown(&decoder));
  return 0;
}

int main(void) {
  if (test_video_config_init() != 0) {
    return 1;
  }
  if (test_video_request_init() != 0) {
    return 1;
  }
  if (test_video_init_and_shutdown() != 0) {
    return 1;
  }
  if (test_video_backend_path() != 0) {
    return 1;
  }
  if (test_video_fallback_m3v0() != 0) {
    return 1;
  }
  return 0;
}
