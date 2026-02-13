#include "test_utils.h"

#include "cmpc/cmp_network.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocatorState {
  int alloc_calls;
  int realloc_calls;
  int free_calls;
  int fail_alloc;
  int fail_realloc;
  int fail_free;
} TestAllocatorState;

static int test_allocator_state_reset(TestAllocatorState *state) {
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(state, 0, sizeof(*state));
  return CMP_OK;
}

static int CMP_CALL test_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  TestAllocatorState *state;

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestAllocatorState *)ctx;
  if (state != NULL) {
    state->alloc_calls += 1;
    if (state->fail_alloc) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
  }

  *out_ptr = malloc((size_t)size);
  if (*out_ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  return CMP_OK;
}

static int CMP_CALL test_realloc(void *ctx, void *ptr, cmp_usize size,
                                 void **out_ptr) {
  TestAllocatorState *state;
  void *mem;

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestAllocatorState *)ctx;
  if (state != NULL) {
    state->realloc_calls += 1;
    if (state->fail_realloc) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
  }

  mem = realloc(ptr, (size_t)size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL test_free(void *ctx, void *ptr) {
  TestAllocatorState *state;

  state = (TestAllocatorState *)ctx;
  if (state != NULL) {
    state->free_calls += 1;
    if (state->fail_free) {
      return CMP_ERR_IO;
    }
  }

  free(ptr);
  return CMP_OK;
}

typedef struct TestNetworkState {
  int fail_request;
  int fail_free;
  int request_calls;
  int free_calls;
  const CMPAllocator *last_allocator;
  CMPNetworkRequest last_request;
  cmp_u32 status_code;
  const cmp_u8 *body_data;
  cmp_usize body_size;
} TestNetworkState;

static int test_network_state_reset(TestNetworkState *state) {
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(state, 0, sizeof(*state));
  return CMP_OK;
}

static int test_network_request(void *net, const CMPNetworkRequest *request,
                                const CMPAllocator *allocator,
                                CMPNetworkResponse *out_response) {
  TestNetworkState *state;
  void *copy;
  int rc;

  if (net == NULL || request == NULL || allocator == NULL ||
      out_response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestNetworkState *)net;
  state->request_calls += 1;
  state->last_request = *request;
  state->last_allocator = allocator;

  if (state->fail_request) {
    return CMP_ERR_IO;
  }

  out_response->status_code = state->status_code;
  out_response->body_size = state->body_size;
  out_response->body = NULL;

  if (state->body_size > 0) {
    rc = allocator->alloc(allocator->ctx, state->body_size, &copy);
    if (rc != CMP_OK) {
      return rc;
    }
    memcpy(copy, state->body_data, (size_t)state->body_size);
    out_response->body = copy;
  }

  return CMP_OK;
}

static int test_network_free_response(void *net, const CMPAllocator *allocator,
                                      CMPNetworkResponse *response) {
  TestNetworkState *state;
  int rc;

  if (net == NULL || allocator == NULL || response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestNetworkState *)net;
  state->free_calls += 1;
  state->last_allocator = allocator;

  if (state->fail_free) {
    return CMP_ERR_IO;
  }

  if (response->body != NULL) {
    rc = allocator->free(allocator->ctx, (void *)response->body);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  response->body = NULL;
  response->body_size = 0;
  response->status_code = 0;
  return CMP_OK;
}

static const CMPNetworkVTable g_test_network_vtable = {
    test_network_request, test_network_free_response};

typedef struct TestEnvState {
  CMPEnv env;
  CMPNetwork network;
  int fail_get_network;
} TestEnvState;

static int test_env_get_network(void *env, CMPNetwork *out_network) {
  TestEnvState *state;

  if (env == NULL || out_network == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestEnvState *)env;
  if (state->fail_get_network) {
    return CMP_ERR_IO;
  }

  *out_network = state->network;
  return CMP_OK;
}

static const CMPEnvVTable g_test_env_vtable = {
    NULL, NULL, NULL, NULL, NULL, NULL, test_env_get_network, NULL, NULL};

static const CMPEnvVTable g_test_env_vtable_no_network = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static int test_env_reset(TestEnvState *env_state, TestNetworkState *net_state,
                          const CMPNetworkVTable *vtable) {
  if (env_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(env_state, 0, sizeof(*env_state));
  env_state->env.ctx = env_state;
  env_state->env.vtable = &g_test_env_vtable;

  if (net_state != NULL && vtable != NULL) {
    env_state->network.ctx = net_state;
    env_state->network.vtable = vtable;
  }

  return CMP_OK;
}

static int test_client_reset(CMPNetworkClient *client,
                             TestNetworkState *net_state,
                             const CMPNetworkVTable *vtable,
                             const CMPAllocator *allocator, CMPBool ready) {
  if (client == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(client, 0, sizeof(*client));
  if (vtable != NULL) {
    client->network.ctx = net_state;
    client->network.vtable = vtable;
  }
  if (allocator != NULL) {
    client->allocator = *allocator;
  }
  client->ready = ready;
  return CMP_OK;
}

static int test_network_config_init(void) {
  CMPNetworkConfig config;

  CMP_TEST_EXPECT(cmp_network_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_network_config_init(&config));
  CMP_TEST_ASSERT(config.env == NULL);
  CMP_TEST_ASSERT(config.allocator == NULL);
  return 0;
}

static int test_network_request_init(void) {
  CMPNetworkRequest request;

  CMP_TEST_EXPECT(cmp_network_request_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_network_request_init(&request));
  CMP_TEST_ASSERT(request.method != NULL);
  CMP_TEST_ASSERT(strcmp(request.method, "GET") == 0);
  CMP_TEST_ASSERT(request.url == NULL);
  CMP_TEST_ASSERT(request.headers == NULL);
  CMP_TEST_ASSERT(request.body == NULL);
  CMP_TEST_ASSERT(request.body_size == 0u);
  CMP_TEST_ASSERT(request.timeout_ms == 0u);
  return 0;
}

static int test_network_init_errors(void) {
  CMPNetworkClient client;
  CMPNetworkConfig config;
  TestEnvState env_state;
  TestNetworkState net_state;
  CMPNetworkVTable vtable_missing;
  CMPAllocator allocator;
  CMPEnv env;

  memset(&client, 0, sizeof(client));
  CMP_TEST_OK(cmp_network_config_init(&config));

  CMP_TEST_EXPECT(cmp_network_init(NULL, &config), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_network_init(&client, NULL), CMP_ERR_INVALID_ARGUMENT);

  client.ready = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_STATE);
  memset(&client, 0, sizeof(client));
  client.network.ctx = &net_state;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_STATE);
  memset(&client, 0, sizeof(client));

  config.env = NULL;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_INVALID_ARGUMENT);

  env.ctx = NULL;
  env.vtable = NULL;
  config.env = &env;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_INVALID_ARGUMENT);

  env.vtable = &g_test_env_vtable_no_network;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_network_state_reset(&net_state));
  CMP_TEST_OK(test_env_reset(&env_state, &net_state, &g_test_network_vtable));
  config.env = &env_state.env;

  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  config.allocator = NULL;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));

  allocator.ctx = NULL;
  allocator.alloc = NULL;
  allocator.realloc = NULL;
  allocator.free = NULL;
  config.allocator = &allocator;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_INVALID_ARGUMENT);
  config.allocator = NULL;

  env_state.fail_get_network = 1;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_IO);
  env_state.fail_get_network = 0;

  env_state.network.ctx = NULL;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_INVALID_ARGUMENT);

  env_state.network.ctx = &net_state;
  env_state.network.vtable = NULL;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_INVALID_ARGUMENT);

  vtable_missing = g_test_network_vtable;
  vtable_missing.request = NULL;
  env_state.network.vtable = &vtable_missing;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_UNSUPPORTED);

  vtable_missing = g_test_network_vtable;
  vtable_missing.free_response = NULL;
  env_state.network.vtable = &vtable_missing;
  CMP_TEST_EXPECT(cmp_network_init(&client, &config), CMP_ERR_UNSUPPORTED);

  return 0;
}

static int test_network_init_success(void) {
  CMPNetworkClient client;
  CMPNetworkConfig config;
  TestEnvState env_state;
  TestNetworkState net_state;
  TestAllocatorState alloc_state;
  CMPAllocator allocator;

  CMP_TEST_OK(test_network_state_reset(&net_state));
  CMP_TEST_OK(test_env_reset(&env_state, &net_state, &g_test_network_vtable));
  CMP_TEST_OK(test_allocator_state_reset(&alloc_state));

  allocator.ctx = &alloc_state;
  allocator.alloc = test_alloc;
  allocator.realloc = test_realloc;
  allocator.free = test_free;

  CMP_TEST_OK(cmp_network_config_init(&config));
  config.env = &env_state.env;
  config.allocator = &allocator;

  memset(&client, 0, sizeof(client));
  CMP_TEST_OK(cmp_network_init(&client, &config));
  CMP_TEST_ASSERT(client.ready == CMP_TRUE);
  CMP_TEST_ASSERT(client.network.ctx == &net_state);
  CMP_TEST_ASSERT(client.network.vtable == &g_test_network_vtable);
  CMP_TEST_ASSERT(client.allocator.ctx == &alloc_state);
  return 0;
}

static int test_network_shutdown_errors(void) {
  CMPNetworkClient client;
  TestNetworkState net_state;
  CMPAllocator allocator;

  CMP_TEST_EXPECT(cmp_network_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);

  memset(&client, 0, sizeof(client));
  CMP_TEST_EXPECT(cmp_network_shutdown(&client), CMP_ERR_STATE);

  CMP_TEST_OK(test_network_state_reset(&net_state));
  allocator.ctx = NULL;
  allocator.alloc = test_alloc;
  allocator.realloc = test_realloc;
  allocator.free = test_free;
  CMP_TEST_OK(test_client_reset(&client, &net_state, &g_test_network_vtable,
                                &allocator, CMP_TRUE));
  client.network.ctx = NULL;
  CMP_TEST_EXPECT(cmp_network_shutdown(&client), CMP_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_network_shutdown_success(void) {
  CMPNetworkClient client;
  CMPNetworkConfig config;
  TestEnvState env_state;
  TestNetworkState net_state;

  CMP_TEST_OK(test_network_state_reset(&net_state));
  CMP_TEST_OK(test_env_reset(&env_state, &net_state, &g_test_network_vtable));
  CMP_TEST_OK(cmp_network_config_init(&config));
  config.env = &env_state.env;

  memset(&client, 0, sizeof(client));
  CMP_TEST_OK(cmp_network_init(&client, &config));
  CMP_TEST_OK(cmp_network_shutdown(&client));
  CMP_TEST_ASSERT(client.ready == CMP_FALSE);
  CMP_TEST_ASSERT(client.network.ctx == NULL);
  CMP_TEST_ASSERT(client.network.vtable == NULL);
  return 0;
}

static int test_network_request_errors(void) {
  CMPNetworkClient client;
  CMPNetworkRequest request;
  CMPNetworkResponse response;
  TestNetworkState net_state;
  TestAllocatorState alloc_state;
  CMPAllocator allocator;
  CMPNetworkVTable vtable_missing;
  cmp_u8 body[2];

  memset(&client, 0, sizeof(client));
  memset(&request, 0, sizeof(request));
  memset(&response, 0, sizeof(response));

  CMP_TEST_EXPECT(cmp_network_request(NULL, &request, &response),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_network_request(&client, NULL, &response),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_STATE);

  CMP_TEST_OK(test_network_state_reset(&net_state));
  CMP_TEST_OK(test_allocator_state_reset(&alloc_state));
  allocator.ctx = &alloc_state;
  allocator.alloc = test_alloc;
  allocator.realloc = test_realloc;
  allocator.free = test_free;
  CMP_TEST_OK(test_client_reset(&client, &net_state, &g_test_network_vtable,
                                &allocator, CMP_TRUE));

  client.network.ctx = NULL;
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_INVALID_ARGUMENT);
  client.network.ctx = &net_state;

  client.network.vtable = NULL;
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_INVALID_ARGUMENT);
  client.network.vtable = &g_test_network_vtable;

  vtable_missing = g_test_network_vtable;
  vtable_missing.request = NULL;
  client.network.vtable = &vtable_missing;
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_UNSUPPORTED);
  client.network.vtable = &g_test_network_vtable;

  client.allocator.alloc = NULL;
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_INVALID_ARGUMENT);
  client.allocator.alloc = test_alloc;

  CMP_TEST_OK(cmp_network_request_init(&request));
  request.method = NULL;
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_INVALID_ARGUMENT);

  request.method = "";
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_INVALID_ARGUMENT);

  request.method = "GET";
  request.url = NULL;
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_INVALID_ARGUMENT);

  request.url = "";
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_INVALID_ARGUMENT);

  request.url = "https://example.com";
  request.body = NULL;
  request.body_size = 2u;
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_INVALID_ARGUMENT);

  request.body = body;
  request.body_size = (cmp_usize)sizeof(body);
  net_state.fail_request = 1;
  CMP_TEST_EXPECT(cmp_network_request(&client, &request, &response),
                  CMP_ERR_IO);
  net_state.fail_request = 0;

  return 0;
}

static int test_network_request_success(void) {
  CMPNetworkClient client;
  CMPNetworkConfig config;
  CMPNetworkRequest request;
  CMPNetworkResponse response;
  TestEnvState env_state;
  TestNetworkState net_state;
  TestAllocatorState alloc_state;
  CMPAllocator allocator;
  cmp_u8 request_body[3];
  cmp_u8 response_body[5];
  cmp_u8 copy[5];
  cmp_usize out_size;

  request_body[0] = 1u;
  request_body[1] = 2u;
  request_body[2] = 3u;
  response_body[0] = 9u;
  response_body[1] = 8u;
  response_body[2] = 7u;
  response_body[3] = 6u;
  response_body[4] = 5u;

  CMP_TEST_OK(test_network_state_reset(&net_state));
  net_state.status_code = 201u;
  net_state.body_data = response_body;
  net_state.body_size = (cmp_usize)sizeof(response_body);

  CMP_TEST_OK(test_env_reset(&env_state, &net_state, &g_test_network_vtable));
  CMP_TEST_OK(test_allocator_state_reset(&alloc_state));

  allocator.ctx = &alloc_state;
  allocator.alloc = test_alloc;
  allocator.realloc = test_realloc;
  allocator.free = test_free;

  CMP_TEST_OK(cmp_network_config_init(&config));
  config.env = &env_state.env;
  config.allocator = &allocator;

  memset(&client, 0, sizeof(client));
  CMP_TEST_OK(cmp_network_init(&client, &config));

  CMP_TEST_OK(cmp_network_request_init(&request));
  request.method = "POST";
  request.url = "https://example.com/api";
  request.headers = "X-Test: 1";
  request.body = request_body;
  request.body_size = (cmp_usize)sizeof(request_body);
  request.timeout_ms = 1234u;

  memset(&response, 0, sizeof(response));
  CMP_TEST_OK(cmp_network_request(&client, &request, &response));
  CMP_TEST_ASSERT(net_state.request_calls == 1);
  CMP_TEST_ASSERT(net_state.last_allocator == &client.allocator);
  CMP_TEST_ASSERT(net_state.last_request.timeout_ms == 1234u);
  CMP_TEST_ASSERT(net_state.last_request.body_size ==
                  (cmp_usize)sizeof(request_body));
  CMP_TEST_ASSERT(response.status_code == 201u);
  CMP_TEST_ASSERT(response.body_size == (cmp_usize)sizeof(response_body));
  CMP_TEST_ASSERT(response.body != NULL);
  CMP_TEST_ASSERT(alloc_state.alloc_calls == 1);

  CMP_TEST_OK(
      cmp_network_copy_response_body(&response, copy, sizeof(copy), &out_size));
  CMP_TEST_ASSERT(out_size == (cmp_usize)sizeof(response_body));
  CMP_TEST_ASSERT(copy[0] == 9u);
  CMP_TEST_ASSERT(copy[4] == 5u);

  CMP_TEST_OK(cmp_network_response_free(&client, &response));
  CMP_TEST_ASSERT(net_state.free_calls == 1);
  CMP_TEST_ASSERT(alloc_state.free_calls == 1);
  CMP_TEST_ASSERT(response.body == NULL);
  CMP_TEST_ASSERT(response.body_size == 0u);

  CMP_TEST_OK(cmp_network_shutdown(&client));
  return 0;
}

static int test_network_response_free_errors(void) {
  CMPNetworkClient client;
  CMPNetworkResponse response;
  TestNetworkState net_state;
  TestAllocatorState alloc_state;
  CMPAllocator allocator;
  CMPNetworkVTable vtable_missing;

  memset(&client, 0, sizeof(client));
  memset(&response, 0, sizeof(response));

  CMP_TEST_EXPECT(cmp_network_response_free(NULL, &response),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_network_response_free(&client, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_network_response_free(&client, &response), CMP_ERR_STATE);

  CMP_TEST_OK(test_network_state_reset(&net_state));
  CMP_TEST_OK(test_allocator_state_reset(&alloc_state));
  allocator.ctx = &alloc_state;
  allocator.alloc = test_alloc;
  allocator.realloc = test_realloc;
  allocator.free = test_free;
  CMP_TEST_OK(test_client_reset(&client, &net_state, &g_test_network_vtable,
                                &allocator, CMP_TRUE));

  client.network.ctx = NULL;
  CMP_TEST_EXPECT(cmp_network_response_free(&client, &response),
                  CMP_ERR_INVALID_ARGUMENT);
  client.network.ctx = &net_state;

  vtable_missing = g_test_network_vtable;
  vtable_missing.free_response = NULL;
  client.network.vtable = &vtable_missing;
  CMP_TEST_EXPECT(cmp_network_response_free(&client, &response),
                  CMP_ERR_UNSUPPORTED);
  client.network.vtable = &g_test_network_vtable;

  client.allocator.alloc = NULL;
  CMP_TEST_EXPECT(cmp_network_response_free(&client, &response),
                  CMP_ERR_INVALID_ARGUMENT);
  client.allocator.alloc = test_alloc;

  net_state.fail_free = 1;
  CMP_TEST_EXPECT(cmp_network_response_free(&client, &response), CMP_ERR_IO);
  net_state.fail_free = 0;

  return 0;
}

static int test_network_copy_response_body(void) {
  CMPNetworkResponse response;
  cmp_u8 buffer[8];
  cmp_usize out_size;

  CMP_TEST_EXPECT(
      cmp_network_copy_response_body(NULL, buffer, sizeof(buffer), &out_size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_network_copy_response_body(&response, buffer, sizeof(buffer), NULL),
      CMP_ERR_INVALID_ARGUMENT);

  memset(&response, 0, sizeof(response));
  response.body_size = 4u;
  response.body = NULL;
  CMP_TEST_EXPECT(cmp_network_copy_response_body(&response, buffer,
                                                 sizeof(buffer), &out_size),
                  CMP_ERR_INVALID_ARGUMENT);

  response.body = buffer;
  CMP_TEST_EXPECT(cmp_network_copy_response_body(&response, NULL,
                                                 sizeof(buffer), &out_size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_network_copy_response_body(&response, buffer, 2u, &out_size),
      CMP_ERR_RANGE);

  buffer[0] = 11u;
  buffer[1] = 12u;
  buffer[2] = 13u;
  buffer[3] = 14u;
  CMP_TEST_OK(cmp_network_copy_response_body(&response, buffer + 4,
                                             sizeof(buffer) - 4u, &out_size));
  CMP_TEST_ASSERT(out_size == 4u);
  CMP_TEST_ASSERT(buffer[4] == 11u);
  CMP_TEST_ASSERT(buffer[7] == 14u);

  response.body_size = 0u;
  response.body = NULL;
  CMP_TEST_OK(cmp_network_copy_response_body(&response, NULL, 0u, &out_size));
  CMP_TEST_ASSERT(out_size == 0u);

  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_network_config_init() == 0);
  CMP_TEST_ASSERT(test_network_request_init() == 0);
  CMP_TEST_ASSERT(test_network_init_errors() == 0);
  CMP_TEST_ASSERT(test_network_init_success() == 0);
  CMP_TEST_ASSERT(test_network_shutdown_errors() == 0);
  CMP_TEST_ASSERT(test_network_shutdown_success() == 0);
  CMP_TEST_ASSERT(test_network_request_errors() == 0);
  CMP_TEST_ASSERT(test_network_request_success() == 0);
  CMP_TEST_ASSERT(test_network_response_free_errors() == 0);
  CMP_TEST_ASSERT(test_network_copy_response_body() == 0);
  return 0;
}
