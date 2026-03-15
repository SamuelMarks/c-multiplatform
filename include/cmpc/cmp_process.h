#ifndef CMP_PROCESS_H
#define CMP_PROCESS_H

/**
 * @file cmp_process.h
 * @brief Multiprocess execution and IPC channels for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_message.h"
/* clang-format on */

/**
 * @brief Opaque handle for a child process.
 */
typedef struct CMPProcess CMPProcess;

/**
 * @brief Opaque handle for an IPC channel.
 */
typedef struct CMPIPCChannel CMPIPCChannel;

/**
 * @brief Configuration for spawning a child process.
 */
typedef struct CMPProcessConfig {
  const char *executable_path; /**< Path to the executable to spawn. */
  const char **args;           /**< Null-terminated array of arguments. */
  CMPBool establish_ipc;       /**< If true, establish an IPC channel. */
} CMPProcessConfig;

/**
 * @brief Spawn a new child process.
 * @param alloc Allocator to use for internal state.
 * @param config Configuration for the process.
 * @param out_process Output process handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_process_spawn(CMPAllocator *alloc,
                                       const CMPProcessConfig *config,
                                       CMPProcess **out_process);

/**
 * @brief Check if a child process is still running.
 * @param process The process handle.
 * @param out_running Receives true if running, false otherwise.
 * @param out_exit_code Receives the exit code if finished.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_process_check(CMPProcess *process,
                                       CMPBool *out_running,
                                       int *out_exit_code);

/**
 * @brief Kill/terminate a child process.
 * @param process The process handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_process_kill(CMPProcess *process);

/**
 * @brief Destroy a process handle (does not kill the process if it's running).
 * @param alloc Allocator used during creation.
 * @param process The process handle to destroy.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_process_destroy(CMPAllocator *alloc,
                                         CMPProcess *process);

/**
 * @brief Get the IPC channel associated with a spawned process (if any).
 * @param process The process handle.
 * @return IPC channel handle, or NULL if none established.
 */
CMP_API int CMP_CALL cmp_process_get_ipc(CMPProcess *process,
                                         CMPIPCChannel **out_channel);

/**
 * @brief Send a message over an IPC channel.
 * @param channel The IPC channel.
 * @param msg The message to send.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_ipc_send(CMPIPCChannel *channel,
                                  const CMPMessage *msg);

/**
 * @brief Receive a message from an IPC channel.
 * @param channel The IPC channel.
 * @param alloc Allocator to allocate the received payload (caller must free).
 * @param out_msg Output message structure.
 * @return CMP_OK on success, CMP_ERR_NOT_READY if no message pending.
 */
CMP_API int CMP_CALL cmp_ipc_receive(CMPIPCChannel *channel,
                                     CMPAllocator *alloc, CMPMessage *out_msg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_PROCESS_H */