#include "cmpc/cmp_image.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestImageState {
  int fail_decode;
  int fail_free;
  int decode_calls;
  int free_calls;
  CMPImageDecodeRequest last_request;
} TestImageState;

static int test_image_state_reset(TestImageState *state) {
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(state, 0, sizeof(*state));
  return CMP_OK;
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

  if (state->fail_decode) {
    return CMP_ERR_IO;
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
  if (state->fail_get_image) {
    return CMP_ERR_IO;
  }

  *out_image = state->image;
  return CMP_OK;
}

static const CMPEnvVTable g_test_env_vtable = {NULL, NULL, NULL,
                                              test_env_get_image, NULL, NULL,
                                              NULL, NULL, NULL};

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

  memset(&bad_alloc, 0, sizeof(bad_alloc));
  config.allocator = &bad_alloc;
  CMP_TEST_EXPECT(cmp_image_init(&decoder, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_image_config_init(&config));
  config.env = NULL;
  CMP_TEST_OK(cmp_image_init(&decoder, &config));
  CMP_TEST_OK(cmp_image_shutdown(&decoder));

  CMP_TEST_OK(test_image_state_reset(&image_state));
  CMP_TEST_OK(test_env_reset(&env_state, &image_state, &g_test_image_vtable));
  config.env = &env_state.env;
  env_state.fail_get_image = 1;
  CMP_TEST_EXPECT(cmp_image_init(&decoder, &config), CMP_ERR_IO);

  env_state.fail_get_image = 0;
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
  static const cmp_u8 ppm_data[] = {
      'P', '6', '\n', '2', ' ', '1', '\n', '2', '5', '5', '\n',
      255u, 0u, 0u, 0u, 255u, 0u};

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
  return 0;
}
