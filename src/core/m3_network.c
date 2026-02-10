#include "m3/m3_network.h"

#include <string.h>

#define M3_NETWORK_VTABLE_COMPLETE(vtable)                                     \
  ((vtable)->request != NULL && (vtable)->free_response != NULL)

int M3_CALL m3_network_config_init(M3NetworkConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  config->env = NULL;
  config->allocator = NULL;
  return M3_OK;
}

int M3_CALL m3_network_request_init(M3NetworkRequest *request) {
  if (request == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  request->method = "GET";
  request->url = NULL;
  request->headers = NULL;
  request->body = NULL;
  request->body_size = 0u;
  request->timeout_ms = 0u;
  return M3_OK;
}

int M3_CALL m3_network_init(M3NetworkClient *client,
                            const M3NetworkConfig *config) {
  M3Network network;
  M3Allocator allocator;
  int rc;

  if (client == NULL || config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (client->network.vtable != NULL || client->network.ctx != NULL ||
      client->ready != M3_FALSE) {
    return M3_ERR_STATE;
  }
  if (config->env == NULL || config->env->vtable == NULL ||
      config->env->vtable->get_network == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (config->allocator == NULL) {
    rc = m3_get_default_allocator(&allocator);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(&network, 0, sizeof(network));
  rc = config->env->vtable->get_network(config->env->ctx, &network);
  if (rc != M3_OK) {
    return rc;
  }

  if (network.ctx == NULL || network.vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!M3_NETWORK_VTABLE_COMPLETE(network.vtable)) {
    return M3_ERR_UNSUPPORTED;
  }

  client->network = network;
  client->allocator = allocator;
  client->ready = M3_TRUE;
  return M3_OK;
}

int M3_CALL m3_network_shutdown(M3NetworkClient *client) {
  if (client == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (client->ready != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (client->network.ctx == NULL || client->network.vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(client, 0, sizeof(*client));
  return M3_OK;
}

int M3_CALL m3_network_request(M3NetworkClient *client,
                               const M3NetworkRequest *request,
                               M3NetworkResponse *out_response) {
  int rc;

  if (client == NULL || request == NULL || out_response == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (client->ready != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (client->network.ctx == NULL || client->network.vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!M3_NETWORK_VTABLE_COMPLETE(client->network.vtable)) {
    return M3_ERR_UNSUPPORTED;
  }
  if (client->allocator.alloc == NULL || client->allocator.realloc == NULL ||
      client->allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (request->method == NULL || request->url == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (request->method[0] == '\0' || request->url[0] == '\0') {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (request->body_size > 0 && request->body == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = client->network.vtable->request(client->network.ctx, request,
                                       &client->allocator, out_response);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_network_response_free(M3NetworkClient *client,
                                     M3NetworkResponse *response) {
  int rc;

  if (client == NULL || response == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (client->ready != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (client->network.ctx == NULL || client->network.vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (client->network.vtable->free_response == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  if (client->allocator.alloc == NULL || client->allocator.realloc == NULL ||
      client->allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = client->network.vtable->free_response(client->network.ctx,
                                             &client->allocator, response);
  if (rc != M3_OK) {
    return rc;
  }

  memset(response, 0, sizeof(*response));
  return M3_OK;
}

int M3_CALL m3_network_copy_response_body(const M3NetworkResponse *response,
                                          void *dst, m3_usize dst_capacity,
                                          m3_usize *out_size) {
  if (response == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (response->body_size > 0 && response->body == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (response->body_size > 0 && dst == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (response->body_size > dst_capacity) {
    return M3_ERR_RANGE;
  }

  if (response->body_size > 0) {
    memcpy(dst, response->body, (size_t)response->body_size);
  }
  *out_size = response->body_size;
  return M3_OK;
}
