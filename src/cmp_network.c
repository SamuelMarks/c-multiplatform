/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_network {
  cmp_network_status_t current_status;
};

int cmp_network_create(cmp_network_t **out_network) {
  cmp_network_t *network;
  if (!out_network) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_network_t), (void **)&network) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(network, 0, sizeof(cmp_network_t));
  network->current_status = CMP_NETWORK_STATUS_ONLINE;
  *out_network = network;
  return CMP_SUCCESS;
}

int cmp_network_destroy(cmp_network_t *network) {
  if (!network) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(network);
  return CMP_SUCCESS;
}

int cmp_network_get_status(const cmp_network_t *network,
                           cmp_network_status_t *out_status) {
  if (!network || !out_status) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_status = network->current_status;
  return CMP_SUCCESS;
}

int cmp_network_set_status(cmp_network_t *network,
                           cmp_network_status_t status) {
  if (!network) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (status != CMP_NETWORK_STATUS_OFFLINE &&
      status != CMP_NETWORK_STATUS_ONLINE &&
      status != CMP_NETWORK_STATUS_METERED) {
    return CMP_ERROR_INVALID_ARG;
  }
  network->current_status = status;
  /* Additional OS hook triggers would execute here */
  return CMP_SUCCESS;
}
