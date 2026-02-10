#ifndef M3_NETWORK_H
#define M3_NETWORK_H

/**
 * @file m3_network.h
 * @brief Network plugin helpers for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"

/**
 * @brief Network client configuration.
 */
typedef struct M3NetworkConfig {
  M3Env *env; /**< Environment backend used to acquire the network interface. */
  const M3Allocator *allocator; /**< Allocator used for response storage (NULL
                                   uses default). */
} M3NetworkConfig;

/**
 * @brief Network client state.
 */
typedef struct M3NetworkClient {
  M3Network network;     /**< Network interface handle. */
  M3Allocator allocator; /**< Allocator used for response storage. */
  M3Bool ready;          /**< M3_TRUE when initialized. */
} M3NetworkClient;

/**
 * @brief Initialize a network configuration with defaults.
 * @param config Configuration to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_network_config_init(M3NetworkConfig *config);

/**
 * @brief Initialize a network request with defaults.
 * @param request Request structure to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_network_request_init(M3NetworkRequest *request);

/**
 * @brief Initialize a network client using the supplied configuration.
 * @param client Client instance to initialize.
 * @param config Network configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_network_init(M3NetworkClient *client,
                                   const M3NetworkConfig *config);

/**
 * @brief Shut down a network client and release its state.
 * @param client Client instance to shut down.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_network_shutdown(M3NetworkClient *client);

/**
 * @brief Execute a network request through the configured backend.
 * @param client Initialized network client.
 * @param request Request parameters.
 * @param out_response Receives the response data (must be freed with
 * m3_network_response_free).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_network_request(M3NetworkClient *client,
                                      const M3NetworkRequest *request,
                                      M3NetworkResponse *out_response);

/**
 * @brief Release resources in a network response.
 * @param client Initialized network client.
 * @param response Response to free.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_network_response_free(M3NetworkClient *client,
                                            M3NetworkResponse *response);

/**
 * @brief Copy a response body into a caller-provided buffer.
 * @param response Response to copy.
 * @param dst Destination buffer (may be NULL when response->body_size is 0).
 * @param dst_capacity Size of the destination buffer in bytes.
 * @param out_size Receives the number of bytes copied.
 * @return M3_OK on success, M3_ERR_RANGE if the buffer is too small, or a
 * failure code.
 */
M3_API int M3_CALL
m3_network_copy_response_body(const M3NetworkResponse *response, void *dst,
                              m3_usize dst_capacity, m3_usize *out_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_NETWORK_H */
