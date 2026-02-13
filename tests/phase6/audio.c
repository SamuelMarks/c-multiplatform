#include "cmpc/cmp_audio.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

int CMP_CALL cmp_audio_test_set_read_u16_fail_after(cmp_u32 call_count);
int CMP_CALL cmp_audio_test_set_read_u32_fail_after(cmp_u32 call_count);
int CMP_CALL cmp_audio_test_read_u16_le(const cmp_u8 *data, cmp_usize size,
                                        cmp_usize offset, cmp_u16 *out_value);
int CMP_CALL cmp_audio_test_read_u32_le(const cmp_u8 *data, cmp_usize size,
                                        cmp_usize offset, cmp_u32 *out_value);
int CMP_CALL cmp_audio_test_decode_wav(const CMPAudioDecodeRequest *request,
                                       const CMPAllocator *allocator,
                                       CMPAudioData *out_audio);

typedef struct TestAudioState {
  int fail_decode;
  int fail_free;
  int decode_calls;
  int free_calls;
} TestAudioState;

typedef struct TestAlloc {
  int fail_alloc_on_call;
  int fail_realloc_on_call;
  int fail_free_on_call;
  int alloc_calls;
  int realloc_calls;
  int free_calls;
} TestAlloc;

static int test_audio_state_reset(TestAudioState *state) {
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(state, 0, sizeof(*state));
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

static void test_write_u16_le(cmp_u8 *out, cmp_u16 value) {
  out[0] = (cmp_u8)(value & 0xFFu);
  out[1] = (cmp_u8)((value >> 8u) & 0xFFu);
}

static void test_write_u32_le(cmp_u8 *out, cmp_u32 value) {
  out[0] = (cmp_u8)(value & 0xFFu);
  out[1] = (cmp_u8)((value >> 8u) & 0xFFu);
  out[2] = (cmp_u8)((value >> 16u) & 0xFFu);
  out[3] = (cmp_u8)((value >> 24u) & 0xFFu);
}

static int test_build_wav(cmp_u8 *out, cmp_usize out_size, cmp_u16 channels,
                          cmp_u32 sample_rate, cmp_u16 bits_per_sample,
                          const cmp_u8 *pcm, cmp_u32 pcm_size,
                          cmp_usize *out_written) {
  cmp_u32 byte_rate;
  cmp_u16 block_align;
  cmp_u32 riff_size;
  cmp_usize total_size;

  if (out == NULL || out_written == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  block_align = (cmp_u16)(channels * (bits_per_sample / 8u));
  byte_rate = sample_rate * (cmp_u32)block_align;
  riff_size = 4u + (8u + 16u) + (8u + pcm_size);
  total_size = (cmp_usize)riff_size + 8u;
  if (out_size < total_size) {
    return CMP_ERR_RANGE;
  }

  out[0] = 'R';
  out[1] = 'I';
  out[2] = 'F';
  out[3] = 'F';
  test_write_u32_le(out + 4u, riff_size);
  out[8] = 'W';
  out[9] = 'A';
  out[10] = 'V';
  out[11] = 'E';

  out[12] = 'f';
  out[13] = 'm';
  out[14] = 't';
  out[15] = ' ';
  test_write_u32_le(out + 16u, 16u);
  test_write_u16_le(out + 20u, 1u);
  test_write_u16_le(out + 22u, channels);
  test_write_u32_le(out + 24u, sample_rate);
  test_write_u32_le(out + 28u, byte_rate);
  test_write_u16_le(out + 32u, block_align);
  test_write_u16_le(out + 34u, bits_per_sample);

  out[36] = 'd';
  out[37] = 'a';
  out[38] = 't';
  out[39] = 'a';
  test_write_u32_le(out + 40u, pcm_size);

  if (pcm_size > 0u && pcm != NULL) {
    memcpy(out + 44u, pcm, pcm_size);
  }
  *out_written = 44u + pcm_size;
  return CMP_OK;
}
static int test_audio_decode(void *audio, const CMPAudioDecodeRequest *request,
                             const CMPAllocator *allocator,
                             CMPAudioData *out_audio) {
  TestAudioState *state;
  cmp_u8 *pcm;
  int rc;

  if (audio == NULL || request == NULL || allocator == NULL ||
      out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestAudioState *)audio;
  state->decode_calls += 1;

  if (state->fail_decode == 1) {
    return CMP_ERR_IO;
  }
  if (state->fail_decode == 2) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = allocator->alloc(allocator->ctx, 2u, (void **)&pcm);
  if (rc != CMP_OK) {
    return rc;
  }

  pcm[0] = 1u;
  pcm[1] = 0u;

  out_audio->format = CMP_AUDIO_FORMAT_S16;
  out_audio->channels = 1u;
  out_audio->sample_rate = 8000u;
  out_audio->frames = 1u;
  out_audio->data = pcm;
  out_audio->size = 2u;
  out_audio->flags = 0u;
  return CMP_OK;
}

static int test_audio_free(void *audio, const CMPAllocator *allocator,
                           CMPAudioData *audio_data) {
  TestAudioState *state;
  int rc;

  if (audio == NULL || allocator == NULL || audio_data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestAudioState *)audio;
  state->free_calls += 1;

  if (state->fail_free) {
    return CMP_ERR_IO;
  }

  if (audio_data->data != NULL) {
    rc = allocator->free(allocator->ctx, (void *)audio_data->data);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  memset(audio_data, 0, sizeof(*audio_data));
  return CMP_OK;
}

static const CMPAudioVTable g_test_audio_vtable = {test_audio_decode,
                                                   test_audio_free};

typedef struct TestEnvState {
  CMPEnv env;
  CMPAudio audio;
  int fail_get_audio;
} TestEnvState;

static int test_env_get_audio(void *env, CMPAudio *out_audio) {
  TestEnvState *state;

  if (env == NULL || out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestEnvState *)env;
  if (state->fail_get_audio == 1) {
    return CMP_ERR_IO;
  }
  if (state->fail_get_audio == 2) {
    return CMP_ERR_UNSUPPORTED;
  }

  *out_audio = state->audio;
  return CMP_OK;
}

static const CMPEnvVTable g_test_env_vtable = {
    NULL, NULL, NULL, NULL, NULL, test_env_get_audio, NULL, NULL, NULL};

static int test_env_reset(TestEnvState *env_state, TestAudioState *audio_state,
                          const CMPAudioVTable *vtable) {
  if (env_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(env_state, 0, sizeof(*env_state));
  env_state->env.ctx = env_state;
  env_state->env.vtable = &g_test_env_vtable;

  if (audio_state != NULL && vtable != NULL) {
    env_state->audio.ctx = audio_state;
    env_state->audio.vtable = vtable;
  }

  return CMP_OK;
}

static int test_audio_config_init(void) {
  CMPAudioConfig config;

  CMP_TEST_EXPECT(cmp_audio_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_audio_config_init(&config));
  CMP_TEST_ASSERT(config.env == NULL);
  CMP_TEST_ASSERT(config.allocator == NULL);
  return 0;
}

static int test_audio_request_init(void) {
  CMPAudioDecodeRequest request;

  CMP_TEST_EXPECT(cmp_audio_request_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_audio_request_init(&request));
  CMP_TEST_ASSERT(request.utf8_path == NULL);
  CMP_TEST_ASSERT(request.data == NULL);
  CMP_TEST_ASSERT(request.size == 0u);
  CMP_TEST_ASSERT(request.encoding == CMP_AUDIO_ENCODING_AUTO);
  return 0;
}

static int test_audio_init_and_shutdown(void) {
  CMPAudioDecoder decoder;
  CMPAudioConfig config;
  CMPAllocator bad_alloc;
  TestEnvState env_state;
  TestAudioState audio_state;

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_audio_init(NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_audio_config_init(&config));
  CMP_TEST_EXPECT(cmp_audio_init(NULL, &config), CMP_ERR_INVALID_ARGUMENT);

  memset(&bad_alloc, 0, sizeof(bad_alloc));
  config.allocator = &bad_alloc;
  CMP_TEST_EXPECT(cmp_audio_init(&decoder, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_audio_config_init(&config));
  config.env = NULL;
  CMP_TEST_OK(cmp_audio_init(&decoder, &config));
  CMP_TEST_OK(cmp_audio_shutdown(&decoder));

  CMP_TEST_OK(test_audio_state_reset(&audio_state));
  CMP_TEST_OK(test_env_reset(&env_state, &audio_state, &g_test_audio_vtable));
  config.env = &env_state.env;
  env_state.fail_get_audio = 1;
  CMP_TEST_EXPECT(cmp_audio_init(&decoder, &config), CMP_ERR_IO);

  env_state.fail_get_audio = 0;
  CMP_TEST_OK(cmp_audio_init(&decoder, &config));
  CMP_TEST_OK(cmp_audio_shutdown(&decoder));
  return 0;
}

static int test_audio_decode_backend(void) {
  CMPAudioDecoder decoder;
  CMPAudioConfig config;
  CMPAudioDecodeRequest request;
  CMPAudioData audio;
  TestEnvState env_state;
  TestAudioState audio_state;
  cmp_u8 encoded[2];

  CMP_TEST_OK(test_audio_state_reset(&audio_state));
  CMP_TEST_OK(test_env_reset(&env_state, &audio_state, &g_test_audio_vtable));
  CMP_TEST_OK(cmp_audio_config_init(&config));
  config.env = &env_state.env;

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_audio_init(&decoder, &config));

  CMP_TEST_OK(cmp_audio_request_init(&request));
  encoded[0] = 0u;
  encoded[1] = 1u;
  request.data = encoded;
  request.size = sizeof(encoded);

  CMP_TEST_OK(cmp_audio_decode(&decoder, &request, &audio));
  CMP_TEST_ASSERT((audio.flags & CMP_AUDIO_DATA_FLAG_BACKEND) != 0u);
  CMP_TEST_ASSERT(audio_state.decode_calls == 1);
  CMP_TEST_OK(cmp_audio_free(&decoder, &audio));
  CMP_TEST_ASSERT(audio_state.free_calls == 1);

  CMP_TEST_OK(cmp_audio_shutdown(&decoder));
  return 0;
}

static int test_audio_decode_fallback_wav(void) {
  CMPAudioDecoder decoder;
  CMPAudioConfig config;
  CMPAudioDecodeRequest request;
  CMPAudioData audio;
  static const cmp_u8 wav_data[] = {
      'R', 'I', 'F', 'F', 38u,  0u,  0u, 0u, 'W', 'A', 'V', 'E',
      'f', 'm', 't', ' ', 16u,  0u,  0u, 0u, 1u,  0u,  1u,  0u,
      64u, 31u, 0u,  0u,  128u, 62u, 0u, 0u, 2u,  0u,  16u, 0u,
      'd', 'a', 't', 'a', 2u,   0u,  0u, 0u, 1u,  0u};

  CMP_TEST_OK(cmp_audio_config_init(&config));
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_audio_init(&decoder, &config));

  CMP_TEST_OK(cmp_audio_request_init(&request));
  request.data = wav_data;
  request.size = sizeof(wav_data);

  CMP_TEST_OK(cmp_audio_decode(&decoder, &request, &audio));
  CMP_TEST_ASSERT(audio.format == CMP_AUDIO_FORMAT_S16);
  CMP_TEST_ASSERT(audio.channels == 1u);
  CMP_TEST_ASSERT(audio.sample_rate == 8000u);
  CMP_TEST_ASSERT(audio.frames == 1u);
  CMP_TEST_ASSERT(audio.size == 2u);
  CMP_TEST_OK(cmp_audio_free(&decoder, &audio));

  CMP_TEST_OK(cmp_audio_shutdown(&decoder));
  return 0;
}

static int test_audio_internal_helpers(void) {
  cmp_u16 value16;
  cmp_u32 value32;
  cmp_u8 data2[2] = {1u, 2u};
  cmp_u8 data4[4] = {1u, 2u, 3u, 4u};

  CMP_TEST_EXPECT(cmp_audio_test_read_u16_le(NULL, 2u, 0u, &value16),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_audio_test_read_u16_le(data2, 2u, 0u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_audio_test_read_u16_le(data2, 1u, 0u, &value16),
                  CMP_ERR_CORRUPT);
  CMP_TEST_OK(cmp_audio_test_read_u16_le(data2, 2u, 0u, &value16));
  CMP_TEST_ASSERT(value16 == 0x0201u);

  CMP_TEST_EXPECT(cmp_audio_test_read_u32_le(NULL, 4u, 0u, &value32),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_audio_test_read_u32_le(data4, 4u, 0u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_audio_test_read_u32_le(data4, 3u, 0u, &value32),
                  CMP_ERR_CORRUPT);
  CMP_TEST_OK(cmp_audio_test_read_u32_le(data4, 4u, 0u, &value32));
  CMP_TEST_ASSERT(value32 == 0x04030201u);
  return 0;
}

static int test_audio_decode_wav_errors(void) {
  CMPAudioDecodeRequest request;
  CMPAudioData audio;
  CMPAllocator allocator;
  TestAlloc alloc;
  cmp_u8 wav[128];
  cmp_u8 pcm[2] = {1u, 0u};
  cmp_usize size;
  cmp_u32 fail_index;

  CMP_TEST_OK(test_alloc_reset(&alloc));
  allocator.ctx = &alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(NULL, &allocator, &audio),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, NULL, &audio),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_audio_request_init(&request));
  request.data = NULL;
  request.size = 0u;
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, pcm, sizeof(pcm),
                             &size));
  request.data = wav;
  request.size = size;

  wav[0] = 'X';
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_UNSUPPORTED);
  wav[0] = 'R';

  wav[8] = 'X';
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_UNSUPPORTED);
  wav[8] = 'W';

  test_write_u32_le(wav + 4u, (cmp_u32)size);
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_CORRUPT);
  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, pcm, sizeof(pcm),
                             &size));

  test_write_u32_le(wav + 16u, 8u);
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_CORRUPT);
  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, pcm, sizeof(pcm),
                             &size));

  test_write_u32_le(wav + 16u, 0xFFFFFFFFu);
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_CORRUPT);
  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, pcm, sizeof(pcm),
                             &size));

  test_write_u16_le(wav + 20u, 3u);
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_UNSUPPORTED);
  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, pcm, sizeof(pcm),
                             &size));

  test_write_u16_le(wav + 34u, 8u);
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_UNSUPPORTED);
  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, pcm, sizeof(pcm),
                             &size));

  test_write_u16_le(wav + 32u, 4u);
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_CORRUPT);
  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, pcm, sizeof(pcm),
                             &size));

  test_write_u32_le(wav + 40u, 1u);
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_CORRUPT);
  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, pcm, sizeof(pcm),
                             &size));

  wav[36] = 'J';
  wav[37] = 'U';
  wav[38] = 'N';
  wav[39] = 'K';
  CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                  CMP_ERR_CORRUPT);
  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, pcm, sizeof(pcm),
                             &size));

  for (fail_index = 1u; fail_index <= 4u; fail_index += 1u) {
    CMP_TEST_OK(cmp_audio_test_set_read_u32_fail_after(fail_index));
    CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                    CMP_ERR_IO);
  }
  CMP_TEST_OK(cmp_audio_test_set_read_u32_fail_after(0u));

  for (fail_index = 1u; fail_index <= 4u; fail_index += 1u) {
    CMP_TEST_OK(cmp_audio_test_set_read_u16_fail_after(fail_index));
    CMP_TEST_EXPECT(cmp_audio_test_decode_wav(&request, &allocator, &audio),
                    CMP_ERR_IO);
  }
  CMP_TEST_OK(cmp_audio_test_set_read_u16_fail_after(0u));
  return 0;
}

static int test_audio_init_edge_cases(void) {
  CMPAudioDecoder decoder;
  CMPAudioConfig config;
  TestEnvState env_state;
  TestAudioState audio_state;
  CMPAudioVTable incomplete_vtable = {test_audio_decode, NULL};

  CMP_TEST_OK(cmp_audio_config_init(&config));
  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_audio_init(&decoder, &config), CMP_ERR_STATE);

  CMP_TEST_OK(test_audio_state_reset(&audio_state));
  CMP_TEST_OK(test_env_reset(&env_state, &audio_state, &g_test_audio_vtable));
  env_state.fail_get_audio = 2;
  config.env = &env_state.env;
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_audio_init(&decoder, &config));
  CMP_TEST_ASSERT(decoder.has_backend == CMP_FALSE);
  CMP_TEST_OK(cmp_audio_shutdown(&decoder));

  CMP_TEST_OK(test_audio_state_reset(&audio_state));
  CMP_TEST_OK(test_env_reset(&env_state, &audio_state, &g_test_audio_vtable));
  env_state.fail_get_audio = 0;
  env_state.audio.ctx = NULL;
  config.env = &env_state.env;
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_audio_init(&decoder, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_audio_state_reset(&audio_state));
  CMP_TEST_OK(test_env_reset(&env_state, &audio_state, &incomplete_vtable));
  config.env = &env_state.env;
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_OK(cmp_audio_init(&decoder, &config));
  CMP_TEST_ASSERT(decoder.has_backend == CMP_FALSE);
  CMP_TEST_OK(cmp_audio_shutdown(&decoder));
  return 0;
}

static int test_audio_api_error_paths(void) {
  CMPAudioDecoder decoder;
  CMPAudioDecodeRequest request;
  CMPAudioData audio;
  CMPAllocator allocator;
  TestAlloc alloc;
  TestAudioState audio_state;
  cmp_u8 dummy[12] = {0u};
  cmp_u8 *heap_buf;
  cmp_u8 wav[64];
  cmp_usize wav_size;

  CMP_TEST_EXPECT(cmp_audio_decode(NULL, NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_audio_request_init(&request));
  CMP_TEST_EXPECT(cmp_audio_decode(NULL, &request, &audio),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_audio_decode(&decoder, &request, &audio), CMP_ERR_STATE);

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_audio_decode(&decoder, &request, &audio),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_alloc_reset(&alloc));
  allocator.ctx = &alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  decoder.allocator = allocator;
  request.data = NULL;
  request.size = 1u;
  request.utf8_path = "x";
  CMP_TEST_EXPECT(cmp_audio_decode(&decoder, &request, &audio),
                  CMP_ERR_INVALID_ARGUMENT);

  request.data = dummy;
  request.size = 0u;
  CMP_TEST_EXPECT(cmp_audio_decode(&decoder, &request, &audio),
                  CMP_ERR_INVALID_ARGUMENT);

  request.data = NULL;
  request.size = 0u;
  request.utf8_path = NULL;
  CMP_TEST_EXPECT(cmp_audio_decode(&decoder, &request, &audio),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_audio_state_reset(&audio_state));
  audio_state.fail_decode = 1;
  decoder.has_backend = CMP_TRUE;
  decoder.audio.ctx = &audio_state;
  decoder.audio.vtable = &g_test_audio_vtable;
  request.data = dummy;
  request.size = sizeof(dummy);
  request.utf8_path = NULL;
  request.encoding = CMP_AUDIO_ENCODING_AUTO;
  CMP_TEST_EXPECT(cmp_audio_decode(&decoder, &request, &audio), CMP_ERR_IO);

  CMP_TEST_OK(test_audio_state_reset(&audio_state));
  audio_state.fail_decode = 2;
  decoder.audio.ctx = &audio_state;
  decoder.audio.vtable = &g_test_audio_vtable;
  request.data = NULL;
  request.size = 0u;
  request.utf8_path = "x";
  CMP_TEST_EXPECT(cmp_audio_decode(&decoder, &request, &audio),
                  CMP_ERR_UNSUPPORTED);

  request.data = dummy;
  request.size = sizeof(dummy);
  request.utf8_path = NULL;
  request.encoding = CMP_AUDIO_ENCODING_AUTO;
  CMP_TEST_EXPECT(cmp_audio_decode(&decoder, &request, &audio),
                  CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(test_build_wav(wav, sizeof(wav), 1u, 8000u, 16u, dummy,
                             sizeof(dummy), &wav_size));
  request.data = wav;
  request.size = wav_size;
  request.encoding = CMP_AUDIO_ENCODING_WAV;
  CMP_TEST_OK(cmp_audio_decode(&decoder, &request, &audio));
  CMP_TEST_OK(cmp_audio_free(&decoder, &audio));

  request.encoding = 99u;
  CMP_TEST_EXPECT(cmp_audio_decode(&decoder, &request, &audio),
                  CMP_ERR_UNSUPPORTED);

  CMP_TEST_EXPECT(cmp_audio_free(NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  memset(&decoder, 0, sizeof(decoder));
  CMP_TEST_EXPECT(cmp_audio_free(&decoder, &audio), CMP_ERR_STATE);

  memset(&decoder, 0, sizeof(decoder));
  decoder.ready = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_audio_free(&decoder, &audio), CMP_ERR_INVALID_ARGUMENT);

  decoder.allocator = allocator;
  audio.size = 1u;
  audio.data = NULL;
  CMP_TEST_EXPECT(cmp_audio_free(&decoder, &audio), CMP_ERR_INVALID_ARGUMENT);

  audio.size = 0u;
  audio.data = NULL;
  audio.flags = CMP_AUDIO_DATA_FLAG_BACKEND;
  decoder.has_backend = CMP_FALSE;
  decoder.audio.vtable = NULL;
  CMP_TEST_EXPECT(cmp_audio_free(&decoder, &audio), CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(test_audio_state_reset(&audio_state));
  audio_state.fail_free = 1;
  decoder.has_backend = CMP_TRUE;
  decoder.audio.ctx = &audio_state;
  decoder.audio.vtable = &g_test_audio_vtable;
  CMP_TEST_EXPECT(cmp_audio_free(&decoder, &audio), CMP_ERR_IO);

  heap_buf = NULL;
  CMP_TEST_OK(
      allocator.alloc(allocator.ctx, sizeof(dummy), (void **)&heap_buf));
  memcpy(heap_buf, dummy, sizeof(dummy));
  audio.flags = 0u;
  audio.data = heap_buf;
  audio.size = sizeof(dummy);
  alloc.fail_free_on_call = alloc.free_calls + 1;
  CMP_TEST_EXPECT(cmp_audio_free(&decoder, &audio), CMP_ERR_IO);
  alloc.fail_free_on_call = 0;
  CMP_TEST_OK(cmp_audio_free(&decoder, &audio));
  return 0;
}

int main(void) {
  if (test_audio_config_init() != 0) {
    return 1;
  }
  if (test_audio_request_init() != 0) {
    return 1;
  }
  if (test_audio_init_and_shutdown() != 0) {
    return 1;
  }
  if (test_audio_decode_backend() != 0) {
    return 1;
  }
  if (test_audio_decode_fallback_wav() != 0) {
    return 1;
  }
  if (test_audio_internal_helpers() != 0) {
    return 1;
  }
  if (test_audio_decode_wav_errors() != 0) {
    return 1;
  }
  if (test_audio_init_edge_cases() != 0) {
    return 1;
  }
  if (test_audio_api_error_paths() != 0) {
    return 1;
  }
  return 0;
}
