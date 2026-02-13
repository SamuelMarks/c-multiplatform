#include "cmpc/cmp_core.h"
#include "cmpc/cmp_video.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

int CMP_CALL cmp_video_test_set_read_u32_fail_after(cmp_u32 call_count);
int CMP_CALL cmp_video_test_set_mul_overflow_fail_after(cmp_u32 call_count);
int CMP_CALL cmp_video_test_mul_overflow(cmp_usize a, cmp_usize b,
                                         cmp_usize *out_value);
int CMP_CALL cmp_video_test_read_u32_le(const cmp_u8 *data, cmp_usize size,
                                        cmp_usize offset, cmp_u32 *out_value);
int CMP_CALL cmp_video_test_fallback_parse(const CMPVideoOpenRequest *request);
int CMP_CALL cmp_video_test_fallback_open(CMPVideoDecoder *decoder,
                                          const CMPVideoOpenRequest *request);
int CMP_CALL cmp_video_test_fallback_close(CMPVideoDecoder *decoder);
int CMP_CALL cmp_video_test_fallback_read_frame_raw(CMPVideoDecoder *decoder,
                                                    CMPVideoFrame *out_frame,
                                                    CMPBool *out_has_frame);
int CMP_CALL cmp_video_test_fallback_read_frame_case(
    cmp_u32 width, cmp_u32 height, cmp_u32 fps_num, cmp_u32 fps_den,
    cmp_u32 frame_count, cmp_u32 frame_index, cmp_usize frame_size,
    CMPVideoFrame *out_frame, CMPBool *out_has_frame);

#define TEST_VIDEO_M3V0_HEADER_SIZE 28u

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

typedef struct TestAlloc {
  int fail_alloc_on_call;
  int fail_realloc_on_call;
  int fail_free_on_call;
  int alloc_calls;
  int realloc_calls;
  int free_calls;
} TestAlloc;

static int test_alloc_reset(TestAlloc *alloc) {
  if (alloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(alloc, 0, sizeof(*alloc));
  return CMP_OK;
}

static int test_alloc_fn(void *ctx, cmp_usize size, void **out_ptr) {
  TestAlloc *alloc;
  void *ptr;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  alloc = (TestAlloc *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on_call > 0 &&
      alloc->alloc_calls == alloc->fail_alloc_on_call) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  ptr = malloc((size_t)size);
  if (ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = ptr;
  return CMP_OK;
}

static int test_realloc_fn(void *ctx, void *ptr, cmp_usize size,
                           void **out_ptr) {
  TestAlloc *alloc;
  void *next;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  alloc = (TestAlloc *)ctx;
  alloc->realloc_calls += 1;
  if (alloc->fail_realloc_on_call > 0 &&
      alloc->realloc_calls == alloc->fail_realloc_on_call) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  next = realloc(ptr, (size_t)size);
  if (next == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = next;
  return CMP_OK;
}

static int test_free_fn(void *ctx, void *ptr) {
  TestAlloc *alloc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAlloc *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on_call > 0 &&
      alloc->free_calls == alloc->fail_free_on_call) {
    return CMP_ERR_IO;
  }

  free(ptr);
  return CMP_OK;
}

static void test_write_u32_le(cmp_u8 *out, cmp_u32 value) {
  out[0] = (cmp_u8)(value & 0xFFu);
  out[1] = (cmp_u8)((value >> 8u) & 0xFFu);
  out[2] = (cmp_u8)((value >> 16u) & 0xFFu);
  out[3] = (cmp_u8)((value >> 24u) & 0xFFu);
}

static int test_build_m3v0(cmp_u8 *out, cmp_usize out_size, cmp_u32 width,
                           cmp_u32 height, cmp_u32 format, cmp_u32 fps_num,
                           cmp_u32 fps_den, cmp_u32 frame_count,
                           const cmp_u8 *frames, cmp_usize frame_size,
                           cmp_usize *out_written) {
  cmp_usize total_size;

  if (out == NULL || out_written == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  total_size = (cmp_usize)frame_count * frame_size;
  if (out_size < TEST_VIDEO_M3V0_HEADER_SIZE + total_size) {
    return CMP_ERR_RANGE;
  }

  out[0] = 'M';
  out[1] = '3';
  out[2] = 'V';
  out[3] = '0';
  test_write_u32_le(out + 4u, width);
  test_write_u32_le(out + 8u, height);
  test_write_u32_le(out + 12u, format);
  test_write_u32_le(out + 16u, fps_num);
  test_write_u32_le(out + 20u, fps_den);
  test_write_u32_le(out + 24u, frame_count);

  if (total_size > 0u && frames != NULL) {
    memcpy(out + TEST_VIDEO_M3V0_HEADER_SIZE, frames, (size_t)total_size);
  }
  *out_written = TEST_VIDEO_M3V0_HEADER_SIZE + total_size;
  return CMP_OK;
}

static int test_video_decoder_init(CMPVideoDecoder *decoder,
                                   const CMPAllocator *allocator) {
  if (decoder == NULL || allocator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(decoder, 0, sizeof(*decoder));
  decoder->allocator = *allocator;
  return CMP_OK;
}
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
  if (state->fail_open == 1) {
    return CMP_ERR_IO;
  }
  if (state->fail_open == 2) {
    return CMP_ERR_UNSUPPORTED;
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

static const CMPVideoVTable g_test_video_vtable = {
    test_video_open, test_video_close, test_video_read};

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
  if (state->fail_get_video == 1) {
    return CMP_ERR_IO;
  }
  if (state->fail_get_video == 2) {
    return CMP_ERR_UNSUPPORTED;
  }

  *out_video = state->video;
  return CMP_OK;
}

static const CMPEnvVTable g_test_env_vtable = {
    NULL, NULL, NULL, NULL, test_env_get_video, NULL, NULL, NULL, NULL};

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

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_video_init(&decoder, &config), CMP_ERR_STATE);

  memset(&decoder, 0, sizeof(decoder));
  memset(&bad_alloc, 0, sizeof(bad_alloc));
  config.allocator = &bad_alloc;
  CMP_TEST_EXPECT(cmp_video_init(&decoder, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_video_config_init(&config));
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_video_init(&decoder, &config), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));

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
  static const cmp_u8 video_data[] = {'M', '3', 'V', '0', 1u, 0u, 0u, 0u,  1u,
                                      0u,  0u,  0u,  1u,  0u, 0u, 0u, 30u, 0u,
                                      0u,  0u,  1u,  0u,  0u, 0u, 2u, 0u,  0u,
                                      0u,  1u,  2u,  3u,  4u, 5u, 6u, 7u,  8u};

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

static int test_video_internal_helpers(void) {
  cmp_usize out_value;
  cmp_u32 value;
  cmp_u8 data[4] = {1u, 2u, 3u, 4u};

  CMP_TEST_EXPECT(cmp_video_test_mul_overflow(1u, 1u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_video_test_mul_overflow(0u, 123u, &out_value));
  CMP_TEST_OK(cmp_video_test_mul_overflow(2u, 3u, &out_value));
  CMP_TEST_EXPECT(
      cmp_video_test_mul_overflow(((cmp_usize) ~(cmp_usize)0), 2u, &out_value),
      CMP_ERR_OVERFLOW);

  CMP_TEST_EXPECT(cmp_video_test_read_u32_le(NULL, 4u, 0u, &value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_video_test_read_u32_le(data, 4u, 0u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_video_test_read_u32_le(data, 3u, 0u, &value),
                  CMP_ERR_CORRUPT);
  CMP_TEST_OK(cmp_video_test_read_u32_le(data, 4u, 0u, &value));
  CMP_TEST_ASSERT(value == 0x04030201u);
  return 0;
}

static int test_video_fallback_parse_errors(void) {
  CMPVideoOpenRequest request;
  cmp_u8 buffer[TEST_VIDEO_M3V0_HEADER_SIZE + 4u];
  cmp_u8 frame[4] = {0u, 1u, 2u, 3u};
  cmp_usize size;
  cmp_u32 fail_index;

  CMP_TEST_OK(test_build_m3v0(buffer, sizeof(buffer), 1u, 1u,
                              CMP_VIDEO_FORMAT_RGBA8, 1u, 1u, 1u, frame,
                              sizeof(frame), &size));
  CMP_TEST_OK(cmp_video_request_init(&request));
  request.data = buffer;
  request.size = size;

  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  request.data = NULL;
  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request),
                  CMP_ERR_INVALID_ARGUMENT);

  request.data = buffer;
  request.size = TEST_VIDEO_M3V0_HEADER_SIZE - 1u;
  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request),
                  CMP_ERR_INVALID_ARGUMENT);

  request.size = size;
  buffer[0] = 'X';
  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request), CMP_ERR_UNSUPPORTED);
  buffer[0] = 'M';

  test_write_u32_le(buffer + 4u, 0u);
  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request), CMP_ERR_CORRUPT);
  test_write_u32_le(buffer + 4u, 1u);

  test_write_u32_le(buffer + 8u, 0u);
  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request), CMP_ERR_CORRUPT);
  test_write_u32_le(buffer + 8u, 1u);

  test_write_u32_le(buffer + 16u, 0u);
  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request), CMP_ERR_CORRUPT);
  test_write_u32_le(buffer + 16u, 1u);

  test_write_u32_le(buffer + 20u, 0u);
  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request), CMP_ERR_CORRUPT);
  test_write_u32_le(buffer + 20u, 1u);

  test_write_u32_le(buffer + 24u, 0u);
  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request), CMP_ERR_CORRUPT);
  test_write_u32_le(buffer + 24u, 1u);

  test_write_u32_le(buffer + 12u, CMP_VIDEO_FORMAT_BGRA8);
  CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request), CMP_ERR_UNSUPPORTED);
  test_write_u32_le(buffer + 12u, CMP_VIDEO_FORMAT_RGBA8);

  for (fail_index = 1u; fail_index <= 7u; fail_index += 1u) {
    CMP_TEST_OK(cmp_video_test_set_read_u32_fail_after(fail_index));
    CMP_TEST_EXPECT(cmp_video_test_fallback_parse(&request), CMP_ERR_IO);
  }
  CMP_TEST_OK(cmp_video_test_set_read_u32_fail_after(0u));
  return 0;
}

static int test_video_fallback_open_errors(void) {
  CMPVideoOpenRequest request;
  CMPVideoDecoder decoder;
  CMPAllocator allocator;
  TestAlloc alloc;
  cmp_u8 buffer[TEST_VIDEO_M3V0_HEADER_SIZE + 4u];
  cmp_u8 frame[4] = {0u, 1u, 2u, 3u};
  cmp_usize size;
  cmp_u32 fail_index;

  CMP_TEST_OK(test_build_m3v0(buffer, sizeof(buffer), 1u, 1u,
                              CMP_VIDEO_FORMAT_RGBA8, 1u, 1u, 1u, frame,
                              sizeof(frame), &size));
  CMP_TEST_OK(cmp_video_request_init(&request));
  request.data = buffer;
  request.size = size;

  CMP_TEST_EXPECT(cmp_video_test_fallback_open(NULL, &request),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_alloc_reset(&alloc));
  allocator.ctx = &alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
  alloc.fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc.fail_alloc_on_call = 0;

  CMP_TEST_OK(test_alloc_reset(&alloc));
  CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
  buffer[0] = 'X';
  CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request),
                  CMP_ERR_UNSUPPORTED);
  buffer[0] = 'M';

  CMP_TEST_OK(test_alloc_reset(&alloc));
  CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
  alloc.fail_free_on_call = 1;
  buffer[0] = 'X';
  CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request), CMP_ERR_IO);
  buffer[0] = 'M';
  alloc.fail_free_on_call = 0;

  for (fail_index = 1u; fail_index <= 3u; fail_index += 1u) {
    CMP_TEST_OK(test_alloc_reset(&alloc));
    CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
    CMP_TEST_OK(cmp_video_test_set_mul_overflow_fail_after(fail_index));
    CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request),
                    CMP_ERR_OVERFLOW);
  }
  CMP_TEST_OK(cmp_video_test_set_mul_overflow_fail_after(0u));

  for (fail_index = 1u; fail_index <= 3u; fail_index += 1u) {
    CMP_TEST_OK(test_alloc_reset(&alloc));
    CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
    alloc.fail_free_on_call = 1;
    CMP_TEST_OK(cmp_video_test_set_mul_overflow_fail_after(fail_index));
    CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request),
                    CMP_ERR_IO);
  }
  CMP_TEST_OK(cmp_video_test_set_mul_overflow_fail_after(0u));

  CMP_TEST_OK(test_alloc_reset(&alloc));
  CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
  request.size = TEST_VIDEO_M3V0_HEADER_SIZE;
  CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request),
                  CMP_ERR_CORRUPT);
  request.size = size;

  CMP_TEST_OK(test_alloc_reset(&alloc));
  CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
  alloc.fail_free_on_call = 1;
  request.size = TEST_VIDEO_M3V0_HEADER_SIZE;
  CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request), CMP_ERR_IO);
  request.size = size;

  CMP_TEST_OK(test_alloc_reset(&alloc));
  CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
  alloc.fail_alloc_on_call = 2;
  CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc.fail_alloc_on_call = 0;

  CMP_TEST_OK(test_alloc_reset(&alloc));
  CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
  alloc.fail_alloc_on_call = 2;
  alloc.fail_free_on_call = 1;
  CMP_TEST_EXPECT(cmp_video_test_fallback_open(&decoder, &request), CMP_ERR_IO);
  alloc.fail_alloc_on_call = 0;
  alloc.fail_free_on_call = 0;
  return 0;
}

static int test_video_fallback_read_frame_errors(void) {
  CMPVideoDecoder decoder;
  CMPVideoFrame frame;
  CMPBool has_frame;

  CMP_TEST_EXPECT(
      cmp_video_test_fallback_read_frame_raw(NULL, &frame, &has_frame),
      CMP_ERR_INVALID_ARGUMENT);

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(
      cmp_video_test_fallback_read_frame_raw(&decoder, &frame, &has_frame),
      CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_video_test_fallback_read_frame_case(
                      1u, 1u, 1u, 1u, 1u, 0u, 4u, NULL, &has_frame),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_video_test_fallback_read_frame_case(1u, 1u, 1u, 1u, 1u,
                                                          0u, 4u, &frame, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_video_test_fallback_read_frame_case(1u, 1u, 1u, 1u, 0u, 0u,
                                                      4u, &frame, &has_frame));
  CMP_TEST_ASSERT(has_frame == CMP_FALSE);

  CMP_TEST_OK(cmp_video_test_fallback_read_frame_case(1u, 1u, 0u, 1u, 1u, 0u,
                                                      4u, &frame, &has_frame));
  CMP_TEST_ASSERT(has_frame == CMP_TRUE);
  CMP_TEST_ASSERT(frame.timestamp_ms == 0u);

  CMP_TEST_OK(cmp_video_test_fallback_read_frame_case(
      1u, 1u, 1u, 0xFFFFFFFFu, 3u, 2u, 4u, &frame, &has_frame));
  CMP_TEST_ASSERT(has_frame == CMP_TRUE);
  CMP_TEST_ASSERT(frame.timestamp_ms == 0xFFFFFFFFu);
  return 0;
}

static int test_video_fallback_close_errors(void) {
  CMPVideoOpenRequest request;
  CMPVideoDecoder decoder;
  CMPAllocator allocator;
  TestAlloc alloc;
  void *fallback_state;
  cmp_u8 buffer[TEST_VIDEO_M3V0_HEADER_SIZE + 4u];
  cmp_u8 frame[4] = {0u, 1u, 2u, 3u};
  cmp_usize size;

  CMP_TEST_OK(test_build_m3v0(buffer, sizeof(buffer), 1u, 1u,
                              CMP_VIDEO_FORMAT_RGBA8, 1u, 1u, 1u, frame,
                              sizeof(frame), &size));
  CMP_TEST_OK(cmp_video_request_init(&request));
  request.data = buffer;
  request.size = size;

  CMP_TEST_EXPECT(cmp_video_test_fallback_close(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_video_test_fallback_close(&decoder), CMP_ERR_STATE);

  CMP_TEST_OK(test_alloc_reset(&alloc));
  allocator.ctx = &alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
  CMP_TEST_OK(cmp_video_test_fallback_open(&decoder, &request));
  alloc.fail_free_on_call = alloc.free_calls + 1;
  CMP_TEST_EXPECT(cmp_video_test_fallback_close(&decoder), CMP_ERR_IO);
  alloc.fail_free_on_call = 0;
  CMP_TEST_OK(cmp_video_test_fallback_close(&decoder));

  CMP_TEST_OK(test_alloc_reset(&alloc));
  CMP_TEST_OK(test_video_decoder_init(&decoder, &allocator));
  CMP_TEST_OK(cmp_video_test_fallback_open(&decoder, &request));
  alloc.fail_free_on_call = alloc.free_calls + 2;
  CMP_TEST_EXPECT(cmp_video_test_fallback_close(&decoder), CMP_ERR_IO);
  alloc.fail_free_on_call = 0;
  fallback_state = decoder.fallback_state;
  CMP_TEST_ASSERT(fallback_state != NULL);
  decoder.fallback_state = NULL;
  decoder.using_fallback = CMP_FALSE;
  decoder.opened = CMP_FALSE;
  CMP_TEST_OK(allocator.free(allocator.ctx, fallback_state));
  return 0;
}

static int test_video_init_edge_cases(void) {
  CMPVideoDecoder decoder;
  CMPVideoConfig config;
  TestEnvState env_state;
  TestVideoState video_state;
  CMPVideoVTable incomplete_vtable = {test_video_open, NULL, test_video_read};

  CMP_TEST_OK(test_video_state_reset(&video_state));
  CMP_TEST_OK(test_env_reset(&env_state, &video_state, &g_test_video_vtable));
  env_state.fail_get_video = 2;
  CMP_TEST_OK(cmp_video_config_init(&config));
  config.env = &env_state.env;
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_video_init(&decoder, &config));
  CMP_TEST_ASSERT(decoder.has_backend == CMP_FALSE);
  CMP_TEST_OK(cmp_video_shutdown(&decoder));

  CMP_TEST_OK(test_video_state_reset(&video_state));
  CMP_TEST_OK(test_env_reset(&env_state, &video_state, &g_test_video_vtable));
  env_state.fail_get_video = 0;
  env_state.video.ctx = NULL;
  config.env = &env_state.env;
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_video_init(&decoder, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_video_state_reset(&video_state));
  CMP_TEST_OK(test_env_reset(&env_state, &video_state, &incomplete_vtable));
  config.env = &env_state.env;
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_video_init(&decoder, &config));
  CMP_TEST_ASSERT(decoder.has_backend == CMP_FALSE);
  CMP_TEST_OK(cmp_video_shutdown(&decoder));
  return 0;
}

static int test_video_api_error_paths(void) {
  CMPVideoDecoder decoder;
  CMPVideoOpenRequest request;
  CMPVideoFrame frame;
  CMPBool has_frame;
  TestVideoState video_state;
  CMPAllocator allocator;
  CMPAllocator alloc_cfg;
  TestAlloc alloc;
  cmp_u8 dummy[4] = {0u, 0u, 0u, 0u};
  cmp_u8 m3v0[64];
  cmp_usize m3v0_size;

  CMP_TEST_EXPECT(cmp_video_open(NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_get_default_allocator(&allocator));
  CMP_TEST_OK(cmp_video_request_init(&request));
  CMP_TEST_EXPECT(cmp_video_open(NULL, &request), CMP_ERR_INVALID_ARGUMENT);

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_video_open(&decoder, &request), CMP_ERR_STATE);

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.opened = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_video_open(&decoder, &request), CMP_ERR_STATE);

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  request.data = NULL;
  request.size = 1u;
  request.utf8_path = "x";
  CMP_TEST_EXPECT(cmp_video_open(&decoder, &request), CMP_ERR_INVALID_ARGUMENT);

  request.data = dummy;
  request.size = 0u;
  CMP_TEST_EXPECT(cmp_video_open(&decoder, &request), CMP_ERR_INVALID_ARGUMENT);

  request.data = NULL;
  request.size = 0u;
  request.utf8_path = NULL;
  CMP_TEST_EXPECT(cmp_video_open(&decoder, &request), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_video_state_reset(&video_state));
  video_state.fail_open = 1;
  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.has_backend = CMP_TRUE;
  decoder.allocator = allocator;
  decoder.video.ctx = &video_state;
  decoder.video.vtable = &g_test_video_vtable;
  request.data = dummy;
  request.size = sizeof(dummy);
  request.utf8_path = NULL;
  request.encoding = CMP_VIDEO_ENCODING_AUTO;
  request.format = CMP_VIDEO_FORMAT_ANY;
  CMP_TEST_EXPECT(cmp_video_open(&decoder, &request), CMP_ERR_IO);

  CMP_TEST_OK(test_video_state_reset(&video_state));
  video_state.fail_open = 2;
  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.has_backend = CMP_TRUE;
  decoder.allocator = allocator;
  decoder.video.ctx = &video_state;
  decoder.video.vtable = &g_test_video_vtable;
  request.data = NULL;
  request.size = 0u;
  request.utf8_path = "x";
  CMP_TEST_EXPECT(cmp_video_open(&decoder, &request), CMP_ERR_UNSUPPORTED);

  request.data = dummy;
  request.size = sizeof(dummy);
  request.utf8_path = NULL;
  request.format = CMP_VIDEO_FORMAT_BGRA8;
  CMP_TEST_EXPECT(cmp_video_open(&decoder, &request), CMP_ERR_UNSUPPORTED);

  request.format = CMP_VIDEO_FORMAT_RGBA8;
  request.encoding = 99u;
  CMP_TEST_EXPECT(cmp_video_open(&decoder, &request), CMP_ERR_UNSUPPORTED);

  CMP_TEST_EXPECT(cmp_video_close(NULL), CMP_ERR_INVALID_ARGUMENT);
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_video_close(&decoder), CMP_ERR_STATE);

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_video_close(&decoder), CMP_ERR_STATE);

  CMP_TEST_OK(test_video_state_reset(&video_state));
  video_state.fail_open = 0;
  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.has_backend = CMP_TRUE;
  decoder.allocator = allocator;
  decoder.video.ctx = &video_state;
  decoder.video.vtable = &g_test_video_vtable;
  CMP_TEST_OK(test_build_m3v0(m3v0, sizeof(m3v0), 1u, 1u,
                              CMP_VIDEO_FORMAT_RGBA8, 1u, 1u, 1u, dummy, 4u,
                              &m3v0_size));
  request.data = m3v0;
  request.size = m3v0_size;
  request.utf8_path = NULL;
  request.encoding = CMP_VIDEO_ENCODING_AUTO;
  request.format = CMP_VIDEO_FORMAT_RGBA8;
  CMP_TEST_OK(cmp_video_open(&decoder, &request));
  video_state.fail_close = 1;
  CMP_TEST_EXPECT(cmp_video_close(&decoder), CMP_ERR_IO);

  CMP_TEST_OK(test_alloc_reset(&alloc));
  alloc_cfg.ctx = &alloc;
  alloc_cfg.alloc = test_alloc_fn;
  alloc_cfg.realloc = test_realloc_fn;
  alloc_cfg.free = test_free_fn;
  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.allocator = alloc_cfg;
  request.data = m3v0;
  request.size = m3v0_size;
  request.utf8_path = NULL;
  request.encoding = CMP_VIDEO_ENCODING_AUTO;
  request.format = CMP_VIDEO_FORMAT_RGBA8;
  CMP_TEST_OK(cmp_video_open(&decoder, &request));
  alloc.fail_free_on_call = alloc.free_calls + 1;
  CMP_TEST_EXPECT(cmp_video_close(&decoder), CMP_ERR_IO);
  alloc.fail_free_on_call = 0;
  CMP_TEST_OK(cmp_video_close(&decoder));

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.opened = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_video_close(&decoder), CMP_ERR_UNSUPPORTED);

  CMP_TEST_EXPECT(cmp_video_read_frame(NULL, &frame, &has_frame),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_video_read_frame(&decoder, NULL, &has_frame),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_video_read_frame(&decoder, &frame, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_video_read_frame(&decoder, &frame, &has_frame),
                  CMP_ERR_STATE);

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_video_read_frame(&decoder, &frame, &has_frame),
                  CMP_ERR_STATE);

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.opened = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_video_read_frame(&decoder, &frame, &has_frame),
                  CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(test_video_state_reset(&video_state));
  video_state.fail_read = 1;
  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.opened = CMP_TRUE;
  decoder.video.ctx = &video_state;
  decoder.video.vtable = &g_test_video_vtable;
  CMP_TEST_EXPECT(cmp_video_read_frame(&decoder, &frame, &has_frame),
                  CMP_ERR_IO);

  CMP_TEST_EXPECT(cmp_video_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_video_shutdown(&decoder), CMP_ERR_STATE);

  CMP_TEST_OK(test_video_state_reset(&video_state));
  video_state.fail_close = 1;
  video_state.opened = CMP_TRUE;
  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.opened = CMP_TRUE;
  decoder.video.ctx = &video_state;
  decoder.video.vtable = &g_test_video_vtable;
  CMP_TEST_EXPECT(cmp_video_shutdown(&decoder), CMP_ERR_IO);
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
  if (test_video_internal_helpers() != 0) {
    return 1;
  }
  if (test_video_fallback_parse_errors() != 0) {
    return 1;
  }
  if (test_video_fallback_open_errors() != 0) {
    return 1;
  }
  if (test_video_fallback_read_frame_errors() != 0) {
    return 1;
  }
  if (test_video_fallback_close_errors() != 0) {
    return 1;
  }
  if (test_video_init_edge_cases() != 0) {
    return 1;
  }
  if (test_video_api_error_paths() != 0) {
    return 1;
  }
  return 0;
}
