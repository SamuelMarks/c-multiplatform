#ifndef CMP_NETWORK_H
#define CMP_NETWORK_H

/**
 * @file cmp_network.h
 * @brief Network plugin helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"

/**
 * @brief Network client configuration.
 */
typedef struct CMPNetworkConfig {
  CMPEnv *env; /**< Environment backend used to acquire the network interface. */
  const CMPAllocator *allocator; /**< Allocator used for response storage (NULL
                                   uses default). */
} CMPNetworkConfig;

/**
 * @brief Network client state.
 */
typedef struct CMPNetworkClient {
  CMPNetwork network;     /**< Network interface handle. */
  CMPAllocator allocator; /**< Allocator used for response storage. */
  CMPBool ready;          /**< CMP_TRUE when initialized. */
} CMPNetworkClient;

/**
 * @brief Initialize a network configuration with defaults.
 * @param config Configuration to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_network_config_init(CMPNetworkConfig *config);

/**
 * @brief Initialize a network request with defaults.
 * @param request Request structure to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_network_request_init(CMPNetworkRequest *request);

/**
 * @brief Initialize a network client using the supplied configuration.
 * @param client Client instance to initialize.
 * @param config Network configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_network_init(CMPNetworkClient *client,
                                   const CMPNetworkConfig *config);

/**
 * @brief Shut down a network client and release its state.
 * @param client Client instance to shut down.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_network_shutdown(CMPNetworkClient *client);

/**
 * @brief Execute a network request through the configured backend.
 * @param client Initialized network client.
 * @param request Request parameters.
 * @param out_response Receives the response data (must be freed with
 * cmp_network_response_free).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_network_request(CMPNetworkClient *client,
                                      const CMPNetworkRequest *request,
                                      CMPNetworkResponse *out_response);

/**
 * @brief Release resources in a network response.
 * @param client Initialized network client.
 * @param response Response to free.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_network_response_free(CMPNetworkClient *client,
                                            CMPNetworkResponse *response);

/**
 * @brief Copy a response body into a caller-provided buffer.
 * @param response Response to copy.
 * @param dst Destination buffer (may be NULL when response->body_size is 0).
 * @param dst_capacity Size of the destination buffer in bytes.
 * @param out_size Receives the number of bytes copied.
 * @return CMP_OK on success, CMP_ERR_RANGE if the buffer is too small, or a
 * failure code.
 */
CMP_API int CMP_CALL
cmp_network_copy_response_body(const CMPNetworkResponse *response, void *dst,
                              cmp_usize dst_capacity, cmp_usize *out_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_NETWORK_H */
