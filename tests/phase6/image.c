#include "cmpc/cmp_core.h"
#include "cmpc/cmp_image.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

int CMP_CALL cmp_image_test_mul_overflow(cmp_usize a, cmp_usize b,
                                         cmp_usize *out_value);
int CMP_CALL cmp_image_test_set_mul_overflow_fail_after(cmp_u32 call_count);
int CMP_CALL cmp_image_test_ppm_skip_ws(const cmp_u8 *data, cmp_usize size,
                                        cmp_usize *offset);
int CMP_CALL cmp_image_test_ppm_read_uint(const cmp_u8 *data, cmp_usize size,
                                          cmp_usize *offset,
                                          cmp_u32 *out_value);
int CMP_CALL cmp_image_test_decode_ppm(const CMPImageDecodeRequest *request,
                                       const CMPAllocator *allocator,
                                       CMPImageData *out_image);
int CMP_CALL cmp_image_test_decode_raw_rgba8(
    const CMPImageDecodeRequest *request, const CMPAllocator *allocator,
    CMPImageData *out_image);

typedef struct TestImageState {
  int fail_decode;
  int fail_decode_rc;
  int fail_free;
  int decode_calls;
  int free_calls;
  CMPImageDecodeRequest last_request;
} TestImageState;

typedef struct TestAlloc {
  int fail_alloc_on_call;
  int fail_realloc_on_call;
  int fail_free_on_call;
  int alloc_calls;
  int realloc_calls;
  int free_calls;
} TestAlloc;

static int test_image_state_reset(TestImageState *state) {
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(state, 0, sizeof(*state));
  state->fail_decode_rc = CMP_ERR_IO;
  return CMP_OK;
}

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
static int test_image_decode(void *image, const CMPImageDecodeRequest *request,
                             const CMPAllocator *allocator,
                             CMPImageData *out_image) {
  TestImageState *state;
  cmp_u8 *pixels;
  int rc;

  if (image == NULL || request == NULL || allocator == NULL ||
      out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestImageState *)image;
  state->decode_calls += 1;
  state->last_request = *request;

  if (state->fail_decode == 1) {
    return state->fail_decode_rc;
  }
  if (state->fail_decode == 2) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = allocator->alloc(allocator->ctx, 4u, (void **)&pixels);
  if (rc != CMP_OK) {
    return rc;
  }

  pixels[0] = 0x11u;
  pixels[1] = 0x22u;
  pixels[2] = 0x33u;
  pixels[3] = 0x44u;

  out_image->format = CMP_IMAGE_FORMAT_RGBA8;
  out_image->width = 1u;
  out_image->height = 1u;
  out_image->stride = 4u;
  out_image->data = pixels;
  out_image->size = 4u;
  out_image->flags = 0u;
  return CMP_OK;
}

static int test_image_free(void *image, const CMPAllocator *allocator,
                           CMPImageData *image_data) {
  TestImageState *state;
  int rc;

  if (image == NULL || allocator == NULL || image_data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestImageState *)image;
  state->free_calls += 1;

  if (state->fail_free) {
    return CMP_ERR_IO;
  }

  if (image_data->data != NULL) {
    rc = allocator->free(allocator->ctx, (void *)image_data->data);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  memset(image_data, 0, sizeof(*image_data));
  return CMP_OK;
}

static const CMPImageVTable g_test_image_vtable = {test_image_decode,
                                                   test_image_free};

typedef struct TestEnvState {
  CMPEnv env;
  CMPImage image;
  int fail_get_image;
} TestEnvState;

static int test_env_get_image(void *env, CMPImage *out_image) {
  TestEnvState *state;

  if (env == NULL || out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestEnvState *)env;
  if (state->fail_get_image == 1) {
    return CMP_ERR_IO;
  }
  if (state->fail_get_image == 2) {
    return CMP_ERR_UNSUPPORTED;
  }

  *out_image = state->image;
  return CMP_OK;
}

static const CMPEnvVTable g_test_env_vtable = {
    NULL, NULL, NULL, test_env_get_image, NULL, NULL, NULL, NULL, NULL};

static int test_env_reset(TestEnvState *env_state, TestImageState *image_state,
                          const CMPImageVTable *vtable) {
  if (env_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(env_state, 0, sizeof(*env_state));
  env_state->env.ctx = env_state;
  env_state->env.vtable = &g_test_env_vtable;

  if (image_state != NULL && vtable != NULL) {
    env_state->image.ctx = image_state;
    env_state->image.vtable = vtable;
  }

  return CMP_OK;
}

static int test_image_config_init(void) {
  CMPImageConfig config;

  CMP_TEST_EXPECT(cmp_image_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_image_config_init(&config));
  CMP_TEST_ASSERT(config.env == NULL);
  CMP_TEST_ASSERT(config.allocator == NULL);
  return 0;
}

static int test_image_request_init(void) {
  CMPImageDecodeRequest request;

  CMP_TEST_EXPECT(cmp_image_request_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_image_request_init(&request));
  CMP_TEST_ASSERT(request.utf8_path == NULL);
  CMP_TEST_ASSERT(request.data == NULL);
  CMP_TEST_ASSERT(request.size == 0u);
  CMP_TEST_ASSERT(request.encoding == CMP_IMAGE_ENCODING_AUTO);
  CMP_TEST_ASSERT(request.format == CMP_IMAGE_FORMAT_ANY);
  CMP_TEST_ASSERT(request.width == 0u);
  CMP_TEST_ASSERT(request.height == 0u);
  CMP_TEST_ASSERT(request.stride == 0u);
  return 0;
}

static int test_image_init_and_shutdown(void) {
  CMPImageDecoder decoder;
  CMPImageConfig config;
  CMPAllocator bad_alloc;
  TestEnvState env_state;
  TestImageState image_state;

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_image_init(NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_image_config_init(&config));
  CMP_TEST_EXPECT(cmp_image_init(NULL, &config), CMP_ERR_INVALID_ARGUMENT);

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_image_init(&decoder, &config), CMP_ERR_STATE);

  memset(&bad_alloc, 0, sizeof(bad_alloc));
  config.allocator = &bad_alloc;
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_image_init(&decoder, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_image_config_init(&config));
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_image_init(&decoder, &config), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));

  CMP_TEST_OK(cmp_image_config_init(&config));
  config.env = NULL;
  CMP_TEST_OK(cmp_image_init(&decoder, &config));
  CMP_TEST_OK(cmp_image_shutdown(&decoder));

  CMP_TEST_OK(test_image_state_reset(&image_state));
  CMP_TEST_OK(test_env_reset(&env_state, &image_state, &g_test_image_vtable));
  config.env = &env_state.env;
  env_state.fail_get_image = 1;
  CMP_TEST_EXPECT(cmp_image_init(&decoder, &config), CMP_ERR_IO);

  env_state.fail_get_image = 2;
  CMP_TEST_OK(cmp_image_init(&decoder, &config));
  CMP_TEST_ASSERT(decoder.has_backend == CMP_FALSE);
  CMP_TEST_OK(cmp_image_shutdown(&decoder));

  CMP_TEST_OK(test_image_state_reset(&image_state));
  CMP_TEST_OK(test_env_reset(&env_state, &image_state, &g_test_image_vtable));
  env_state.image.ctx = NULL;
  config.env = &env_state.env;
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_image_init(&decoder, &config), CMP_ERR_INVALID_ARGUMENT);

  env_state.fail_get_image = 0;
  env_state.image.ctx = &image_state;
  env_state.image.vtable = &g_test_image_vtable;
  CMP_TEST_OK(cmp_image_init(&decoder, &config));
  CMP_TEST_OK(cmp_image_shutdown(&decoder));
  return 0;
}

static int test_image_decode_backend(void) {
  CMPImageDecoder decoder;
  CMPImageConfig config;
  CMPImageDecodeRequest request;
  CMPImageData image;
  TestEnvState env_state;
  TestImageState image_state;
  cmp_u8 encoded[4];

  CMP_TEST_OK(test_image_state_reset(&image_state));
  CMP_TEST_OK(test_env_reset(&env_state, &image_state, &g_test_image_vtable));
  CMP_TEST_OK(cmp_image_config_init(&config));
  config.env = &env_state.env;

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_image_init(&decoder, &config));

  CMP_TEST_OK(cmp_image_request_init(&request));
  encoded[0] = 0u;
  request.data = encoded;
  request.size = sizeof(encoded);

  CMP_TEST_OK(cmp_image_decode(&decoder, &request, &image));
  CMP_TEST_ASSERT((image.flags & CMP_IMAGE_DATA_FLAG_BACKEND) != 0u);
  CMP_TEST_ASSERT(image_state.decode_calls == 1);
  CMP_TEST_OK(cmp_image_free(&decoder, &image));
  CMP_TEST_ASSERT(image_state.free_calls == 1);

  CMP_TEST_OK(cmp_image_shutdown(&decoder));
  return 0;
}

static int test_image_decode_fallback_ppm(void) {
  CMPImageDecoder decoder;
  CMPImageConfig config;
  CMPImageDecodeRequest request;
  CMPImageData image;
  static const cmp_u8 ppm_data[] = {'P',  '6', '\n', '2',  ' ',  '1',
                                    '\n', '2', '5',  '5',  '\n', 255u,
                                    0u,   0u,  0u,   255u, 0u};

  CMP_TEST_OK(cmp_image_config_init(&config));
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_image_init(&decoder, &config));

  CMP_TEST_OK(cmp_image_request_init(&request));
  request.data = ppm_data;
  request.size = sizeof(ppm_data);

  CMP_TEST_OK(cmp_image_decode(&decoder, &request, &image));
  CMP_TEST_ASSERT(image.format == CMP_IMAGE_FORMAT_RGBA8);
  CMP_TEST_ASSERT(image.width == 2u);
  CMP_TEST_ASSERT(image.height == 1u);
  CMP_TEST_ASSERT(image.size == 8u);
  CMP_TEST_ASSERT(((const cmp_u8 *)image.data)[0] == 255u);
  CMP_TEST_ASSERT(((const cmp_u8 *)image.data)[4] == 0u);
  CMP_TEST_OK(cmp_image_free(&decoder, &image));

  request.encoding = CMP_IMAGE_ENCODING_PPM;
  CMP_TEST_OK(cmp_image_decode(&decoder, &request, &image));
  CMP_TEST_OK(cmp_image_free(&decoder, &image));

  CMP_TEST_OK(cmp_image_shutdown(&decoder));
  return 0;
}

static int test_image_decode_fallback_raw(void) {
  CMPImageDecoder decoder;
  CMPImageConfig config;
  CMPImageDecodeRequest request;
  CMPImageData image;
  const cmp_u8 raw_data[4] = {1u, 2u, 3u, 4u};

  CMP_TEST_OK(cmp_image_config_init(&config));
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_image_init(&decoder, &config));

  CMP_TEST_OK(cmp_image_request_init(&request));
  request.encoding = CMP_IMAGE_ENCODING_RAW_RGBA8;
  request.data = raw_data;
  request.size = sizeof(raw_data);
  request.width = 1u;
  request.height = 1u;

  CMP_TEST_OK(cmp_image_decode(&decoder, &request, &image));
  CMP_TEST_ASSERT(image.size == 4u);
  CMP_TEST_ASSERT(((const cmp_u8 *)image.data)[2] == 3u);
  CMP_TEST_OK(cmp_image_free(&decoder, &image));

  CMP_TEST_OK(cmp_image_shutdown(&decoder));
  return 0;
}

static int test_image_internal_helpers(void) {
  cmp_usize value;
  cmp_usize offset;
  cmp_u32 out_value;
  const cmp_u8 data_comment_nl[] = {'#', 'x', '\n', 'Z'};
  const cmp_u8 data_comment_cr[] = {'#', 'x', '\r', 'Z'};
  const cmp_u8 data_space[] = {' ', 'Z'};
  const cmp_u8 data_tab[] = {'\t', 'Z'};
  const cmp_u8 data_nl[] = {'\n', 'Z'};
  const cmp_u8 data_cr[] = {'\r', 'Z'};
  const cmp_u8 data_ff[] = {'\f', 'Z'};
  const cmp_u8 data_vt[] = {'\v', 'Z'};
  const cmp_u8 data_ws_only[] = {' ', '\t'};
  const cmp_u8 data_digit[] = {'1'};
  const cmp_u8 data_nodigit[] = {'x'};
  const cmp_u8 data_digits[] = {'1', '2', 'x'};
  const cmp_u8 data_overflow[] = {'4', '2', '9', '4', '9', '6',
                                  '7', '2', '9', '6', '0'};

  CMP_TEST_EXPECT(cmp_image_test_mul_overflow(1u, 1u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_test_mul_overflow(
                      ((cmp_usize) ~(cmp_usize)0) / 2u + 1u, 2u, &value),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_image_test_mul_overflow(0u, 5u, &value));
  CMP_TEST_OK(cmp_image_test_mul_overflow(2u, 3u, &value));
  CMP_TEST_ASSERT(value == 6u);

  offset = 0u;
  CMP_TEST_EXPECT(cmp_image_test_ppm_skip_ws(NULL, 0u, &offset),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_image_test_ppm_skip_ws(data_space, sizeof(data_space), NULL),
      CMP_ERR_INVALID_ARGUMENT);

  offset = 0u;
  CMP_TEST_OK(
      cmp_image_test_ppm_skip_ws(data_space, sizeof(data_space), &offset));
  CMP_TEST_ASSERT(data_space[offset] == 'Z');
  offset = 0u;
  CMP_TEST_OK(cmp_image_test_ppm_skip_ws(data_tab, sizeof(data_tab), &offset));
  CMP_TEST_ASSERT(data_tab[offset] == 'Z');
  offset = 0u;
  CMP_TEST_OK(cmp_image_test_ppm_skip_ws(data_nl, sizeof(data_nl), &offset));
  CMP_TEST_ASSERT(data_nl[offset] == 'Z');
  offset = 0u;
  CMP_TEST_OK(cmp_image_test_ppm_skip_ws(data_cr, sizeof(data_cr), &offset));
  CMP_TEST_ASSERT(data_cr[offset] == 'Z');
  offset = 0u;
  CMP_TEST_OK(cmp_image_test_ppm_skip_ws(data_ff, sizeof(data_ff), &offset));
  CMP_TEST_ASSERT(data_ff[offset] == 'Z');
  offset = 0u;
  CMP_TEST_OK(cmp_image_test_ppm_skip_ws(data_vt, sizeof(data_vt), &offset));
  CMP_TEST_ASSERT(data_vt[offset] == 'Z');

  offset = 0u;
  CMP_TEST_OK(cmp_image_test_ppm_skip_ws(data_comment_nl,
                                         sizeof(data_comment_nl), &offset));
  CMP_TEST_ASSERT(data_comment_nl[offset] == 'Z');
  offset = 0u;
  CMP_TEST_OK(cmp_image_test_ppm_skip_ws(data_comment_cr,
                                         sizeof(data_comment_cr), &offset));
  CMP_TEST_ASSERT(data_comment_cr[offset] == 'Z');

  offset = 0u;
  CMP_TEST_EXPECT(
      cmp_image_test_ppm_skip_ws(data_ws_only, sizeof(data_ws_only), &offset),
      CMP_ERR_CORRUPT);

  offset = 0u;
  CMP_TEST_EXPECT(cmp_image_test_ppm_read_uint(NULL, 0u, &offset, &out_value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_test_ppm_read_uint(data_space, sizeof(data_space),
                                               NULL, &out_value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_test_ppm_read_uint(data_space, sizeof(data_space),
                                               &offset, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  offset = 0u;
  CMP_TEST_EXPECT(cmp_image_test_ppm_read_uint(
                      data_ws_only, sizeof(data_ws_only), &offset, &out_value),
                  CMP_ERR_CORRUPT);
  offset = 1u;
  CMP_TEST_EXPECT(cmp_image_test_ppm_read_uint(data_digit, sizeof(data_digit),
                                               &offset, &out_value),
                  CMP_ERR_CORRUPT);
  offset = 0u;
  CMP_TEST_EXPECT(cmp_image_test_ppm_read_uint(
                      data_nodigit, sizeof(data_nodigit), &offset, &out_value),
                  CMP_ERR_CORRUPT);
  offset = 0u;
  CMP_TEST_OK(cmp_image_test_ppm_read_uint(data_digits, sizeof(data_digits),
                                           &offset, &out_value));
  CMP_TEST_ASSERT(out_value == 12u);
  CMP_TEST_ASSERT(offset == 2u);
  offset = 0u;
  CMP_TEST_EXPECT(cmp_image_test_ppm_read_uint(data_overflow,
                                               sizeof(data_overflow), &offset,
                                               &out_value),
                  CMP_ERR_OVERFLOW);

  return 0;
}

static int test_image_decode_ppm_helpers(void) {
  CMPAllocator allocator;
  CMPImageDecodeRequest request;
  CMPImageData image;
  CMPAllocator bad_alloc;
  const cmp_u8 tiny[] = {'P', '6'};
  const cmp_u8 bad_magic[] = {'P', '3', '\n'};
  const cmp_u8 bad_digit[] = {'P', '6', '\n', 'x'};
  const cmp_u8 bad_height[] = {'P', '6', '\n', '1', ' ', 'x', '\n'};
  const cmp_u8 bad_maxval_token[] = {'P', '6',  '\n', '1', ' ',
                                     '1', '\n', 'x',  '\n'};
  const cmp_u8 bad_trailing_ws[] = {'P',  '6', '\n', '1', ' ',  '1',
                                    '\n', '2', '5',  '5', '\n', '#'};
  const cmp_u8 zero_width[] = {'P', '6', '\n', '0',  ' ', '1', '\n',
                               '2', '5', '5',  '\n', 0u,  0u,  0u};
  const cmp_u8 zero_height[] = {'P', '6', '\n', '1',  ' ', '0', '\n',
                                '2', '5', '5',  '\n', 0u,  0u,  0u};
  const cmp_u8 bad_max[] = {'P', '6', '\n', '1',  ' ', '1', '\n',
                            '1', '0', '0',  '\n', 0u,  0u,  0u};
  const cmp_u8 short_data[] = {'P', '6', '\n', '2',  ' ', '2', '\n',
                               '2', '5', '5',  '\n', 0u,  0u,  0u};
  const cmp_u8 valid_ppm[] = {'P', '6', '\n', '1',  ' ', '1', '\n',
                              '2', '5', '5',  '\n', 1u,  2u,  3u};
  const cmp_u8 overflow_header[] = {'P',  '6', '\n', '4', '2',  '9', '4', '9',
                                    '6',  '7', '2',  '9', '5',  ' ', '4', '2',
                                    '9',  '4', '9',  '6', '7',  '2', '9', '5',
                                    '\n', '2', '5',  '5', '\n', 0u};

  CMP_TEST_OK(cmp_get_default_allocator(&allocator));
  CMP_TEST_OK(cmp_image_request_init(&request));
  memset(&image, 0, sizeof(image));

  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(NULL, &allocator, &image),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, NULL, &image),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&bad_alloc, 0, sizeof(bad_alloc));
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &bad_alloc, &image),
                  CMP_ERR_INVALID_ARGUMENT);

  request.data = NULL;
  request.size = 0u;
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_INVALID_ARGUMENT);

  request.data = tiny;
  request.size = sizeof(tiny);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_CORRUPT);

  request.data = bad_magic;
  request.size = sizeof(bad_magic);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_UNSUPPORTED);

  request.data = bad_digit;
  request.size = sizeof(bad_digit);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_CORRUPT);

  request.data = bad_height;
  request.size = sizeof(bad_height);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_CORRUPT);

  request.data = bad_maxval_token;
  request.size = sizeof(bad_maxval_token);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_CORRUPT);

  request.data = bad_trailing_ws;
  request.size = sizeof(bad_trailing_ws);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_CORRUPT);

  request.data = zero_width;
  request.size = sizeof(zero_width);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_CORRUPT);

  request.data = zero_height;
  request.size = sizeof(zero_height);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_CORRUPT);

  request.data = bad_max;
  request.size = sizeof(bad_max);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_UNSUPPORTED);

  request.data = short_data;
  request.size = sizeof(short_data);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_CORRUPT);

  request.data = overflow_header;
  request.size = sizeof(overflow_header);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_OVERFLOW);

  CMP_TEST_OK(cmp_image_test_set_mul_overflow_fail_after(1u));
  request.data = valid_ppm;
  request.size = sizeof(valid_ppm);
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_image_test_set_mul_overflow_fail_after(2u));
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_image_test_set_mul_overflow_fail_after(3u));
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_image_test_set_mul_overflow_fail_after(0u));

  request.data = valid_ppm;
  request.size = sizeof(valid_ppm);
  request.format = CMP_IMAGE_FORMAT_BGRA8;
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_UNSUPPORTED);

  return 0;
}

static int test_image_decode_raw_helpers(void) {
  CMPAllocator allocator;
  CMPAllocator bad_alloc;
  CMPImageDecodeRequest request;
  CMPImageData image;
  cmp_u8 raw_data[16];

  CMP_TEST_OK(cmp_get_default_allocator(&allocator));
  CMP_TEST_OK(cmp_image_request_init(&request));
  memset(&image, 0, sizeof(image));
  test_write_u32_le(raw_data, 0x04030201u);
  test_write_u32_le(raw_data + 4u, 0x08070605u);
  test_write_u32_le(raw_data + 8u, 0x0c0b0a09u);
  test_write_u32_le(raw_data + 12u, 0x100f0e0du);

  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(NULL, &allocator, &image),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, NULL, &image),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&bad_alloc, 0, sizeof(bad_alloc));
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &bad_alloc, &image),
                  CMP_ERR_INVALID_ARGUMENT);

  request.data = NULL;
  request.size = 0u;
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_INVALID_ARGUMENT);

  request.data = raw_data;
  request.size = sizeof(raw_data);
  request.width = 0u;
  request.height = 1u;
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_INVALID_ARGUMENT);

  request.width = 1u;
  request.height = 0u;
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_INVALID_ARGUMENT);

  request.width = 1u;
  request.height = 1u;
  request.format = CMP_IMAGE_FORMAT_BGRA8;
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_UNSUPPORTED);

  request.format = CMP_IMAGE_FORMAT_RGBA8;
  CMP_TEST_OK(cmp_image_test_set_mul_overflow_fail_after(1u));
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_image_test_set_mul_overflow_fail_after(2u));
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_image_test_set_mul_overflow_fail_after(3u));
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_image_test_set_mul_overflow_fail_after(0u));

  request.format = CMP_IMAGE_FORMAT_RGBA8;
  request.size = 1u;
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_RANGE);

  request.size = sizeof(raw_data);
  request.width = 1u;
  request.height = 2u;
  request.stride = 8u;
  CMP_TEST_OK(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image));
  CMP_TEST_OK(allocator.free(allocator.ctx, (void *)image.data));

  request.width = 0xFFFFFFFFu;
  request.height = 0xFFFFFFFFu;
  request.size = (cmp_usize) ~(cmp_usize)0;
  request.stride = 0u;
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_OVERFLOW);

  return 0;
}

static int test_image_alloc_failures(void) {
  CMPAllocator allocator;
  CMPImageDecodeRequest request;
  CMPImageData image;
  TestAlloc alloc;
  cmp_u8 raw_data[4];
  static const cmp_u8 ppm_data[] = {'P', '6', '\n', '1',  ' ', '1', '\n',
                                    '2', '5', '5',  '\n', 1u,  2u,  3u};

  CMP_TEST_OK(test_alloc_reset(&alloc));
  allocator.ctx = &alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_image_request_init(&request));
  memset(&image, 0, sizeof(image));
  request.data = ppm_data;
  request.size = sizeof(ppm_data);
  request.format = CMP_IMAGE_FORMAT_RGBA8;

  alloc.fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_image_test_decode_ppm(&request, &allocator, &image),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc.fail_alloc_on_call = 0;
  CMP_TEST_OK(test_alloc_reset(&alloc));

  test_write_u32_le(raw_data, 0x04030201u);
  CMP_TEST_OK(cmp_image_request_init(&request));
  request.data = raw_data;
  request.size = sizeof(raw_data);
  request.width = 1u;
  request.height = 1u;
  request.format = CMP_IMAGE_FORMAT_RGBA8;

  alloc.fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_image_test_decode_raw_rgba8(&request, &allocator, &image),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc.fail_alloc_on_call = 0;
  return 0;
}

static int test_image_decode_error_paths(void) {
  CMPImageDecoder decoder;
  CMPImageConfig config;
  CMPImageDecodeRequest request;
  CMPImageData image;
  TestEnvState env_state;
  TestImageState image_state;
  cmp_u8 encoded[4];
  const cmp_u8 bad_magic[] = {'P', '3', '\n'};

  CMP_TEST_EXPECT(cmp_image_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_image_shutdown(&decoder), CMP_ERR_STATE);

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_image_config_init(&config));
  CMP_TEST_OK(cmp_image_request_init(&request));
  CMP_TEST_EXPECT(cmp_image_decode(NULL, &request, &image),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, &image), CMP_ERR_STATE);

  CMP_TEST_OK(test_image_state_reset(&image_state));
  CMP_TEST_OK(test_env_reset(&env_state, &image_state, &g_test_image_vtable));
  config.env = &env_state.env;
  CMP_TEST_OK(cmp_image_init(&decoder, &config));

  CMP_TEST_EXPECT(cmp_image_decode(&decoder, NULL, &image),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  decoder.allocator.alloc = NULL;
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, &image),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_get_default_allocator(&decoder.allocator));

  request.size = 4u;
  request.data = NULL;
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, &image),
                  CMP_ERR_INVALID_ARGUMENT);
  request.data = encoded;
  request.size = 0u;
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, &image),
                  CMP_ERR_INVALID_ARGUMENT);
  request.data = NULL;
  request.size = 0u;
  request.utf8_path = NULL;
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, &image),
                  CMP_ERR_INVALID_ARGUMENT);

  request.data = encoded;
  request.size = sizeof(encoded);
  request.encoding = CMP_IMAGE_ENCODING_AUTO;
  image_state.fail_decode = 1;
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, &image), CMP_ERR_IO);

  image_state.fail_decode_rc = CMP_ERR_UNSUPPORTED;
  request.data = bad_magic;
  request.size = sizeof(bad_magic);
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, &image),
                  CMP_ERR_UNSUPPORTED);

  request.data = NULL;
  request.size = 0u;
  request.utf8_path = "x";
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, &image),
                  CMP_ERR_UNSUPPORTED);

  request.data = encoded;
  request.size = sizeof(encoded);
  request.utf8_path = NULL;
  request.encoding = 99u;
  CMP_TEST_EXPECT(cmp_image_decode(&decoder, &request, &image),
                  CMP_ERR_UNSUPPORTED);

  image_state.fail_decode = 0;
  CMP_TEST_OK(cmp_image_shutdown(&decoder));

  return 0;
}

static int test_image_free_error_paths(void) {
  CMPImageDecoder decoder;
  CMPImageConfig config;
  CMPImageData image;
  TestEnvState env_state;
  TestImageState image_state;
  TestAlloc alloc;
  CMPAllocator alloc_cfg;
  cmp_u8 pixels_stack[4] = {1u, 2u, 3u, 4u};
  cmp_u8 *pixels_heap = NULL;

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_image_free(NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_image_free(&decoder, &image), CMP_ERR_STATE);
  CMP_TEST_OK(cmp_image_config_init(&config));
  CMP_TEST_OK(cmp_image_init(&decoder, &config));

  memset(&image, 0, sizeof(image));
  decoder.allocator.alloc = NULL;
  CMP_TEST_EXPECT(cmp_image_free(&decoder, &image), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_get_default_allocator(&decoder.allocator));

  memset(&image, 0, sizeof(image));
  image.size = 4u;
  image.data = NULL;
  CMP_TEST_EXPECT(cmp_image_free(&decoder, &image), CMP_ERR_INVALID_ARGUMENT);

  image.size = 4u;
  image.data = pixels_stack;
  image.flags = CMP_IMAGE_DATA_FLAG_BACKEND;
  decoder.has_backend = CMP_FALSE;
  CMP_TEST_EXPECT(cmp_image_free(&decoder, &image), CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(cmp_image_shutdown(&decoder));

  CMP_TEST_OK(test_image_state_reset(&image_state));
  CMP_TEST_OK(test_env_reset(&env_state, &image_state, &g_test_image_vtable));
  CMP_TEST_OK(cmp_image_config_init(&config));
  config.env = &env_state.env;
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_image_init(&decoder, &config));

  memset(&image, 0, sizeof(image));
  image.flags = CMP_IMAGE_DATA_FLAG_BACKEND;
  CMP_TEST_OK(decoder.allocator.alloc(decoder.allocator.ctx, 4u,
                                      (void **)&pixels_heap));
  if (pixels_heap != NULL) {
    pixels_heap[0] = 1u;
    pixels_heap[1] = 2u;
    pixels_heap[2] = 3u;
    pixels_heap[3] = 4u;
  }
  image.data = pixels_heap;
  image.size = 4u;
  image_state.fail_free = 1;
  CMP_TEST_EXPECT(cmp_image_free(&decoder, &image), CMP_ERR_IO);

  image_state.fail_free = 0;
  CMP_TEST_OK(cmp_image_free(&decoder, &image));
  pixels_heap = NULL;
  CMP_TEST_OK(cmp_image_shutdown(&decoder));

  CMP_TEST_OK(test_alloc_reset(&alloc));
  alloc_cfg.ctx = &alloc;
  alloc_cfg.alloc = test_alloc_fn;
  alloc_cfg.realloc = test_realloc_fn;
  alloc_cfg.free = test_free_fn;
  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.allocator = alloc_cfg;

  CMP_TEST_OK(alloc_cfg.alloc(alloc_cfg.ctx, 4u, (void **)&pixels_heap));
  image.flags = 0u;
  image.data = pixels_heap;
  image.size = 4u;
  alloc.fail_free_on_call = 1;
  CMP_TEST_EXPECT(cmp_image_free(&decoder, &image), CMP_ERR_IO);
  alloc.fail_free_on_call = 0;
  CMP_TEST_OK(cmp_image_free(&decoder, &image));
  pixels_heap = NULL;
  return 0;
}

int main(void) {
  if (test_image_config_init() != 0) {
    return 1;
  }
  if (test_image_request_init() != 0) {
    return 1;
  }
  if (test_image_init_and_shutdown() != 0) {
    return 1;
  }
  if (test_image_decode_backend() != 0) {
    return 1;
  }
  if (test_image_decode_fallback_ppm() != 0) {
    return 1;
  }
  if (test_image_decode_fallback_raw() != 0) {
    return 1;
  }
  if (test_image_internal_helpers() != 0) {
    return 1;
  }
  if (test_image_decode_ppm_helpers() != 0) {
    return 1;
  }
  if (test_image_decode_raw_helpers() != 0) {
    return 1;
  }
  if (test_image_alloc_failures() != 0) {
    return 1;
  }
  if (test_image_decode_error_paths() != 0) {
    return 1;
  }
  if (test_image_free_error_paths() != 0) {
    return 1;
  }
  return 0;
}
