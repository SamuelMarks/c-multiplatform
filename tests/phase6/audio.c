#include "cmpc/cmp_audio.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestAudioState {
  int fail_decode;
  int fail_free;
  int decode_calls;
  int free_calls;
} TestAudioState;

static int test_audio_state_reset(TestAudioState *state) {
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(state, 0, sizeof(*state));
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

  if (state->fail_decode) {
    return CMP_ERR_IO;
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
  if (state->fail_get_audio) {
    return CMP_ERR_IO;
  }

  *out_audio = state->audio;
  return CMP_OK;
}

static const CMPEnvVTable g_test_env_vtable = {NULL, NULL, NULL, NULL, NULL,
                                              test_env_get_audio, NULL, NULL,
                                              NULL};

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
      'R', 'I', 'F', 'F', 38u, 0u, 0u, 0u, 'W', 'A', 'V', 'E',
      'f', 'm', 't', ' ', 16u, 0u, 0u, 0u, 1u, 0u, 1u, 0u,
      64u, 31u, 0u, 0u, 128u, 62u, 0u, 0u, 2u, 0u, 16u, 0u,
      'd', 'a', 't', 'a', 2u, 0u, 0u, 0u, 1u, 0u};

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
  return 0;
}
