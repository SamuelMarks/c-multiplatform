#ifndef CMP_NET_H
#define CMP_NET_H

/**
 * @file cmp_net.h
 * @brief Asynchronous networking operations for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_core.h"
/* clang-format on */

/**
 * @brief Network fetch asynchronous callback signature.
 * @param ctx The context for the callback.
 * @param url The URL that was fetched.
 * @param data The response body data buffer (allocated, must be freed by user)
 * or NULL.
 * @param size The size of the response data.
 * @param status CMP_OK on success or an error code.
 * @param status_code HTTP status code (e.g. 200).
 * @param user User context passed to cmp_net_fetch_async.
 */
typedef void(CMP_CALL *CMPNetFetchCallback)(void *ctx, const char *url,
                                            void *data, cmp_usize size,
                                            int status, int status_code,
                                            void *user);

/**
 * @brief Fetch a URL asynchronously.
 * @param env The environment interface.
 * @param tasks The tasks interface for background work.
 * @param loop The event loop for returning the result to the main thread.
 * @param allocator Allocator for the buffer and internal state.
 * @param url URL to fetch.
 * @param callback Callback to invoke on the main event loop upon completion.
 * @param user User context to pass to the callback.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_net_fetch_async(CMPEnv *env, CMPTasks *tasks,
                                         CMPEventLoop *loop,
                                         const CMPAllocator *allocator,
                                         const char *url,
                                         CMPNetFetchCallback callback,
                                         void *user);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_NET_H */