/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_network {
  cmp_network_status_t current_status;
};

/**
 * @brief Create a network context.
 *
 * @param out_network Pointer to store the created context.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_network_create(cmp_network_t **out_network) {
  int rc = CMP_SUCCESS;
  cmp_network_t *network;

  rc = CMP_SUCCESS;
  network = NULL;

  if (out_network == NULL) {
    LOG_DEBUG("Error in cmp_network_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_network_t), (void **)&network);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_network_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  memset(network, 0, sizeof(cmp_network_t));
  network->current_status = CMP_NETWORK_STATUS_ONLINE;

  *out_network = network;
  return rc;
}

/**
 * @brief Destroy a network context.
 *
 * @param network The context to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_network_destroy(cmp_network_t *network) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (network == NULL) {
    LOG_DEBUG("Error in cmp_network_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(network);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_network_destroy: CMP_FREE failed\n");
    return rc;
  }
  return rc;
}

/**
 * @brief Get the current network status.
 *
 * @param network The network context.
 * @param out_status Pointer to store the status.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_network_get_status(const cmp_network_t *network,
                           cmp_network_status_t *out_status) {
  int rc = CMP_SUCCESS;
  if (network == NULL || out_status == NULL) {
    LOG_DEBUG("Error in cmp_network_get_status: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_status = network->current_status;
  return rc;
}

/**
 * @brief Set the network status (internal/mock).
 *
 * @param network The network context.
 * @param status The status to set.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_network_set_status(cmp_network_t *network,
                           cmp_network_status_t status) {
  int rc = CMP_SUCCESS;
  if (network == NULL) {
    LOG_DEBUG("Error in cmp_network_set_status: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (status != CMP_NETWORK_STATUS_OFFLINE &&
      status != CMP_NETWORK_STATUS_ONLINE &&
      status != CMP_NETWORK_STATUS_METERED) {
    LOG_DEBUG("Error in cmp_network_set_status: Invalid status\n");
    return CMP_ERROR_INVALID_ARG;
  }

  network->current_status = status;
  /* Additional OS hook triggers would execute here */
  return rc;
}
