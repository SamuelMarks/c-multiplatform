/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_network {
  cmp_network_status_t current_status;
};

int cmp_network_create(cmp_network_t **out_network) {
  int rc = CMP_SUCCESS;
  cmp_network_t *network = NULL;

  if (!out_network) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_network_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_network_t), (void **)&network);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_network_create: Out of memory\n");
    return rc;
  }

  memset(network, 0, sizeof(cmp_network_t));
  network->current_status = CMP_NETWORK_STATUS_ONLINE;
  *out_network = network;
  return rc;
}

int cmp_network_destroy(cmp_network_t *network) {
  int rc = CMP_SUCCESS;

  if (!network) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_network_destroy: Invalid argument\n");
    return rc;
  }
  CMP_FREE(network);
  return rc;
}

int cmp_network_get_status(const cmp_network_t *network,
                           cmp_network_status_t *out_status) {
  int rc = CMP_SUCCESS;

  if (!network || !out_status) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_network_get_status: Invalid argument\n");
    return rc;
  }
  *out_status = network->current_status;
  return rc;
}

int cmp_network_set_status(cmp_network_t *network,
                           cmp_network_status_t status) {
  int rc = CMP_SUCCESS;

  if (!network) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_network_set_status: Invalid argument\n");
    return rc;
  }

  if (status != CMP_NETWORK_STATUS_OFFLINE &&
      status != CMP_NETWORK_STATUS_ONLINE &&
      status != CMP_NETWORK_STATUS_METERED) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_network_set_status: Invalid status\n");
    return rc;
  }

  network->current_status = status;
  /* Additional OS hook triggers would execute here */
  return rc;
}
