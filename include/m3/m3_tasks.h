#ifndef M3_TASKS_H
#define M3_TASKS_H

/**
 * @file m3_tasks.h
 * @brief Default task runner implementation for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"

/**
 * @brief Configuration for the default task runner.
 */
typedef struct M3TasksDefaultConfig {
    const M3Allocator *allocator; /**< Allocator used for task runner memory (NULL uses default). */
    m3_usize worker_count; /**< Number of worker threads. */
    m3_usize queue_capacity; /**< Maximum number of queued tasks. */
    m3_usize handle_capacity; /**< Capacity for thread/mutex handles. */
} M3TasksDefaultConfig;

/**
 * @brief Initialize a task runner config with defaults.
 * @param config Config to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_default_config_init(M3TasksDefaultConfig *config);

/**
 * @brief Create the default task runner.
 * @param config Task runner configuration (NULL uses defaults).
 * @param out_tasks Receives the task interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_default_create(const M3TasksDefaultConfig *config, M3Tasks *out_tasks);

/**
 * @brief Destroy a task runner created by the default implementation.
 * @param tasks Task interface to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_default_destroy(M3Tasks *tasks);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_TASKS_H */
