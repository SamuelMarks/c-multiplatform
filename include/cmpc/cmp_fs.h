#ifndef CMP_FS_H
#define CMP_FS_H

/**
 * @file cmp_fs.h
 * @brief Asynchronous file system operations for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_core.h"
/* clang-format on */

/**
 * @brief File read asynchronous callback signature.
 * @param ctx The context for the callback.
 * @param utf8_path The file path.
 * @param data The file data buffer (allocated, must be freed by user) or NULL.
 * @param size The size of the file data.
 * @param status CMP_OK on success or an error code.
 * @param user User context passed to cmp_fs_read_async.
 */
typedef void(CMP_CALL *CMPFSReadCallback)(void *ctx, const char *utf8_path,
                                          void *data, cmp_usize size,
                                          int status, void *user);

/**
 * @brief Read a file asynchronously.
 * @param env The environment interface for IO operations.
 * @param tasks The tasks interface for background work.
 * @param loop The event loop for returning the result to the main thread.
 * @param allocator Allocator for the buffer and internal state.
 * @param utf8_path File path to read.
 * @param callback Callback to invoke on the main event loop upon completion.
 * @param user User context to pass to the callback.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_fs_read_async(CMPEnv *env, CMPTasks *tasks,
                                       CMPEventLoop *loop,
                                       const CMPAllocator *allocator,
                                       const char *utf8_path,
                                       CMPFSReadCallback callback, void *user);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_FS_H */