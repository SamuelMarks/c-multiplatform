#include "cmpc/cmp_network.h"

#include <string.h>

#define CMP_NETWORK_VTABLE_COMPLETE(vtable)                                     \
  ((vtable)->request != NULL && (vtable)->free_response != NULL)

int CMP_CALL cmp_network_config_init(CMPNetworkConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->env = NULL;
  config->allocator = NULL;
  return CMP_OK;
}

int CMP_CALL cmp_network_request_init(CMPNetworkRequest *request) {
  if (request == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  request->method = "GET";
  request->url = NULL;
  request->headers = NULL;
  request->body = NULL;
  request->body_size = 0u;
  request->timeout_ms = 0u;
  return CMP_OK;
}

int CMP_CALL cmp_network_init(CMPNetworkClient *client,
                            const CMPNetworkConfig *config) {
  CMPNetwork network;
  CMPAllocator allocator;
  int rc;

  if (client == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (client->network.vtable != NULL || client->network.ctx != NULL ||
      client->ready != CMP_FALSE) {
    return CMP_ERR_STATE;
  }
  if (config->env == NULL || config->env->vtable == NULL ||
      config->env->vtable->get_network == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(&network, 0, sizeof(network));
  rc = config->env->vtable->get_network(config->env->ctx, &network);
  if (rc != CMP_OK) {
    return rc;
  }

  if (network.ctx == NULL || network.vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!CMP_NETWORK_VTABLE_COMPLETE(network.vtable)) {
    return CMP_ERR_UNSUPPORTED;
  }

  client->network = network;
  client->allocator = allocator;
  client->ready = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_network_shutdown(CMPNetworkClient *client) {
  if (client == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (client->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (client->network.ctx == NULL || client->network.vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(client, 0, sizeof(*client));
  return CMP_OK;
}

int CMP_CALL cmp_network_request(CMPNetworkClient *client,
                               const CMPNetworkRequest *request,
                               CMPNetworkResponse *out_response) {
  int rc;

  if (client == NULL || request == NULL || out_response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (client->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (client->network.ctx == NULL || client->network.vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!CMP_NETWORK_VTABLE_COMPLETE(client->network.vtable)) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (client->allocator.alloc == NULL || client->allocator.realloc == NULL ||
      client->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->method == NULL || request->url == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->method[0] == '\0' || request->url[0] == '\0') {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->body_size > 0 && request->body == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = client->network.vtable->request(client->network.ctx, request,
                                       &client->allocator, out_response);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_network_response_free(CMPNetworkClient *client,
                                     CMPNetworkResponse *response) {
  int rc;

  if (client == NULL || response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (client->ready != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (client->network.ctx == NULL || client->network.vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (client->network.vtable->free_response == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (client->allocator.alloc == NULL || client->allocator.realloc == NULL ||
      client->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = client->network.vtable->free_response(client->network.ctx,
                                             &client->allocator, response);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(response, 0, sizeof(*response));
  return CMP_OK;
}

int CMP_CALL cmp_network_copy_response_body(const CMPNetworkResponse *response,
                                          void *dst, cmp_usize dst_capacity,
                                          cmp_usize *out_size) {
  if (response == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (response->body_size > 0 && response->body == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (response->body_size > 0 && dst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (response->body_size > dst_capacity) {
    return CMP_ERR_RANGE;
  }

  if (response->body_size > 0) {
    memcpy(dst, response->body, (size_t)response->body_size);
  }
  *out_size = response->body_size;
  return CMP_OK;
}
