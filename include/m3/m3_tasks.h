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
  const M3Allocator *allocator; /**< Allocator used for task runner memory (NULL
                                   uses default). */
  m3_usize worker_count;        /**< Number of worker threads. */
  m3_usize queue_capacity;      /**< Maximum number of queued tasks. */
  m3_usize handle_capacity;     /**< Capacity for thread/mutex handles. */
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
M3_API int M3_CALL m3_tasks_default_create(const M3TasksDefaultConfig *config,
                                           M3Tasks *out_tasks);

/**
 * @brief Destroy a task runner created by the default implementation.
 * @param tasks Task interface to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_default_destroy(M3Tasks *tasks);

#ifdef M3_TESTING
/* Failure injection points for task tests. */
#define M3_TASKS_TEST_FAIL_NONE 0u
#define M3_TASKS_TEST_FAIL_TIME_NOW 1u
#define M3_TASKS_TEST_FAIL_MUTEX_INIT 2u
#define M3_TASKS_TEST_FAIL_MUTEX_DESTROY 3u
#define M3_TASKS_TEST_FAIL_MUTEX_LOCK 4u
#define M3_TASKS_TEST_FAIL_MUTEX_UNLOCK 5u
#define M3_TASKS_TEST_FAIL_COND_INIT 6u
#define M3_TASKS_TEST_FAIL_COND_DESTROY 7u
#define M3_TASKS_TEST_FAIL_COND_SIGNAL 8u
#define M3_TASKS_TEST_FAIL_COND_BROADCAST 9u
#define M3_TASKS_TEST_FAIL_COND_WAIT 10u
#define M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_TIMEOUT 11u
#define M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR 12u
#define M3_TASKS_TEST_FAIL_THREAD_CREATE 13u
#define M3_TASKS_TEST_FAIL_THREAD_JOIN 14u
#define M3_TASKS_TEST_FAIL_SLEEP 15u
#define M3_TASKS_TEST_FAIL_CONFIG_INIT 16u
#define M3_TASKS_TEST_FAIL_OBJECT_INIT 17u
#define M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_INVALID 18u
#define M3_TASKS_TEST_FAIL_TEST_MALLOC 19u
#define M3_TASKS_TEST_FAIL_ALLOC_EX_NULL 20u
#define M3_TASKS_TEST_FAIL_REALLOC_EX_NULL 21u

/* Internal native-call probes for tests. */
#define M3_TASKS_TEST_NATIVE_TIME_NOW 1u
#define M3_TASKS_TEST_NATIVE_MUTEX_INIT 2u
#define M3_TASKS_TEST_NATIVE_MUTEX_DESTROY 3u
#define M3_TASKS_TEST_NATIVE_MUTEX_LOCK 4u
#define M3_TASKS_TEST_NATIVE_MUTEX_UNLOCK 5u
#define M3_TASKS_TEST_NATIVE_COND_INIT 6u
#define M3_TASKS_TEST_NATIVE_COND_DESTROY 7u
#define M3_TASKS_TEST_NATIVE_COND_SIGNAL 8u
#define M3_TASKS_TEST_NATIVE_COND_BROADCAST 9u
#define M3_TASKS_TEST_NATIVE_COND_WAIT 10u
#define M3_TASKS_TEST_NATIVE_COND_TIMEDWAIT 11u
#define M3_TASKS_TEST_NATIVE_THREAD_CREATE 12u
#define M3_TASKS_TEST_NATIVE_THREAD_JOIN 13u
#define M3_TASKS_TEST_NATIVE_SLEEP 14u

/* Task object destroy probes for tests. */
#define M3_TASKS_TEST_THREAD_DESTROY_NULL 1u
#define M3_TASKS_TEST_THREAD_DESTROY_NOT_JOINED 2u
#define M3_TASKS_TEST_THREAD_DESTROY_UNREGISTER_FAIL 3u
#define M3_TASKS_TEST_THREAD_DESTROY_FREE_FAIL 4u
#define M3_TASKS_TEST_THREAD_DESTROY_OK 5u
#define M3_TASKS_TEST_THREAD_DESTROY_UNREGISTER_NULL_CTX 6u
#define M3_TASKS_TEST_THREAD_DESTROY_FREE_NULL_CTX 7u

#define M3_TASKS_TEST_MUTEX_DESTROY_NULL 1u
#define M3_TASKS_TEST_MUTEX_DESTROY_NATIVE_FAIL 2u
#define M3_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_FAIL 3u
#define M3_TASKS_TEST_MUTEX_DESTROY_FREE_FAIL 4u
#define M3_TASKS_TEST_MUTEX_DESTROY_OK 5u
#define M3_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_NULL_CTX 6u
#define M3_TASKS_TEST_MUTEX_DESTROY_FREE_NULL_CTX 7u

#define M3_TASKS_TEST_QUEUE_CASE_EMPTY 1u
#define M3_TASKS_TEST_QUEUE_CASE_INVALID 2u
#define M3_TASKS_TEST_QUEUE_CASE_NOT_READY 3u
#define M3_TASKS_TEST_QUEUE_CASE_READY_EARLY 4u
#define M3_TASKS_TEST_QUEUE_CASE_READY_LATE 5u
#define M3_TASKS_TEST_QUEUE_CASE_TIME_FAIL 6u
#define M3_TASKS_TEST_QUEUE_CASE_SWAP 7u
#define M3_TASKS_TEST_QUEUE_CASE_BUSY 8u

#define M3_TASKS_TEST_POST_CASE_LOCK_FAIL 1u
#define M3_TASKS_TEST_POST_CASE_SIGNAL_FAIL 2u
#define M3_TASKS_TEST_POST_CASE_UNLOCK_FAIL 3u
#define M3_TASKS_TEST_POST_CASE_BUSY 4u

#define M3_TASKS_TEST_POST_DELAYED_CASE_TIME_FAIL 1u
#define M3_TASKS_TEST_POST_DELAYED_CASE_LOCK_FAIL 2u
#define M3_TASKS_TEST_POST_DELAYED_CASE_SIGNAL_FAIL 3u
#define M3_TASKS_TEST_POST_DELAYED_CASE_UNLOCK_FAIL 4u
#define M3_TASKS_TEST_POST_DELAYED_CASE_BUSY 5u
#define M3_TASKS_TEST_POST_DELAYED_CASE_OVERFLOW 6u

#define M3_TASKS_TEST_WORKER_CASE_LOCK_FAIL 1u
#define M3_TASKS_TEST_WORKER_CASE_WAIT_FAIL 2u
#define M3_TASKS_TEST_WORKER_CASE_TIMEDWAIT_TIMEOUT 3u
#define M3_TASKS_TEST_WORKER_CASE_TASK_ERROR 4u
#define M3_TASKS_TEST_WORKER_CASE_PICK_ERROR 5u
#define M3_TASKS_TEST_WORKER_CASE_UNLOCK_FAIL 6u
#define M3_TASKS_TEST_WORKER_CASE_NULL 7u
#define M3_TASKS_TEST_WORKER_CASE_TIMEDWAIT_SIGNAL 8u

#define M3_TASKS_TEST_THREAD_ENTRY_NULL 1u
#define M3_TASKS_TEST_THREAD_ENTRY_NO_ENTRY 2u

#define M3_TASKS_TEST_THREAD_CREATE_STATE 1u
#define M3_TASKS_TEST_THREAD_CREATE_ALLOC_FAIL 2u
#define M3_TASKS_TEST_THREAD_CREATE_OBJECT_FAIL 3u
#define M3_TASKS_TEST_THREAD_CREATE_NATIVE_FAIL 4u
#define M3_TASKS_TEST_THREAD_CREATE_REGISTER_FAIL 5u

#define M3_TASKS_TEST_THREAD_JOIN_STATE 1u
#define M3_TASKS_TEST_THREAD_JOIN_RESOLVE_FAIL 2u
#define M3_TASKS_TEST_THREAD_JOIN_ALREADY_JOINED 3u
#define M3_TASKS_TEST_THREAD_JOIN_NATIVE_FAIL 4u
#define M3_TASKS_TEST_THREAD_JOIN_UNREGISTER_FAIL 5u
#define M3_TASKS_TEST_THREAD_JOIN_FREE_FAIL 6u

#define M3_TASKS_TEST_MUTEX_CREATE_STATE 1u
#define M3_TASKS_TEST_MUTEX_CREATE_ALLOC_FAIL 2u
#define M3_TASKS_TEST_MUTEX_CREATE_OBJECT_FAIL 3u
#define M3_TASKS_TEST_MUTEX_CREATE_NATIVE_FAIL 4u
#define M3_TASKS_TEST_MUTEX_CREATE_REGISTER_FAIL 5u

#define M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_STATE 1u
#define M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_RESOLVE_FAIL 2u
#define M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_NATIVE_FAIL 3u
#define M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_UNREGISTER_FAIL 4u
#define M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_FREE_FAIL 5u

#define M3_TASKS_TEST_MUTEX_LOCK_STATE 1u
#define M3_TASKS_TEST_MUTEX_LOCK_RESOLVE_FAIL 2u
#define M3_TASKS_TEST_MUTEX_LOCK_NOT_INITIALIZED 3u

#define M3_TASKS_TEST_MUTEX_UNLOCK_STATE 1u
#define M3_TASKS_TEST_MUTEX_UNLOCK_RESOLVE_FAIL 2u
#define M3_TASKS_TEST_MUTEX_UNLOCK_NOT_INITIALIZED 3u

#define M3_TASKS_TEST_POST_CASE_STATE 5u
#define M3_TASKS_TEST_POST_DELAYED_CASE_STATE 7u

#define M3_TASKS_TEST_OBJECT_OP_INIT_FAIL 6u
#define M3_TASKS_TEST_OBJECT_OP_INIT_EARLY_FAIL 7u

#define M3_TASKS_TEST_SLEEP_STATE 1u

#define M3_TASKS_TEST_TIMEDWAIT_SIGNAL_NULL 6u

#define M3_TASKS_TEST_OBJECT_OP_RETAIN 1u
#define M3_TASKS_TEST_OBJECT_OP_RELEASE 2u
#define M3_TASKS_TEST_OBJECT_OP_GET_TYPE 3u
#define M3_TASKS_TEST_OBJECT_OP_RETAIN_NULL 4u
#define M3_TASKS_TEST_OBJECT_OP_RELEASE_NULL 5u

#define M3_TASKS_TEST_TIMEDWAIT_TIMEOUT 1u
#define M3_TASKS_TEST_TIMEDWAIT_ERROR 2u
#define M3_TASKS_TEST_TIMEDWAIT_TIME_FAIL 3u
#define M3_TASKS_TEST_TIMEDWAIT_OK 4u
#define M3_TASKS_TEST_TIMEDWAIT_NSEC_ADJUST 5u

M3_API int M3_CALL m3_tasks_test_set_fail_point(m3_u32 point);
M3_API int M3_CALL m3_tasks_test_clear_fail_point(void);
M3_API int M3_CALL m3_tasks_test_set_thread_create_fail_after(m3_u32 count);
M3_API int M3_CALL m3_tasks_test_mul_overflow(m3_usize a, m3_usize b,
                                              m3_usize *out_value);
M3_API int M3_CALL m3_tasks_test_call_native(m3_u32 op);
/**
 * @brief Test wrapper for thread entry handling.
 * @param mode Thread entry test mode.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_thread_entry_case(m3_u32 mode);
/**
 * @brief Test wrapper for thread creation error handling.
 * @param mode Thread create test mode.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_thread_create_case(m3_u32 mode);
/**
 * @brief Test wrapper for thread join error handling.
 * @param mode Thread join test mode.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_thread_join_case(m3_u32 mode);
/**
 * @brief Test wrapper for mutex creation error handling.
 * @param mode Mutex create test mode.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_mutex_create_case(m3_u32 mode);
/**
 * @brief Test wrapper for mutex destroy error handling.
 * @param mode Mutex destroy test mode.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_default_mutex_destroy_case(m3_u32 mode);
/**
 * @brief Test wrapper for mutex lock error handling.
 * @param mode Mutex lock test mode.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_mutex_lock_case(m3_u32 mode);
/**
 * @brief Test wrapper for mutex unlock error handling.
 * @param mode Mutex unlock test mode.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_mutex_unlock_case(m3_u32 mode);
/**
 * @brief Test wrapper for sleep error handling.
 * @param mode Sleep test mode.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_sleep_case(m3_u32 mode);
/**
 * @brief Test wrapper for destroy state handling.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_destroy_state(void);
/**
 * @brief Exercise test-only stubs for coverage.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_stub_exercise(void);
/**
 * @brief Test wrapper for signal-stop thread entry null handling.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_signal_stop_null(void);
M3_API int M3_CALL m3_tasks_test_thread_destroy_case(m3_u32 mode);
M3_API int M3_CALL m3_tasks_test_mutex_destroy_case(m3_u32 mode);
M3_API int M3_CALL m3_tasks_test_queue_case(m3_u32 mode, m3_u32 *out_wait_ms,
                                            m3_u32 *out_due_time,
                                            m3_usize *out_count);
M3_API int M3_CALL m3_tasks_test_post_case(m3_u32 mode);
M3_API int M3_CALL m3_tasks_test_post_delayed_case(m3_u32 mode);
M3_API int M3_CALL m3_tasks_test_worker_case(m3_u32 mode);
M3_API int M3_CALL m3_tasks_test_object_op(m3_u32 mode, m3_u32 *out_type_id);
M3_API int M3_CALL m3_tasks_test_timedwait_case(m3_u32 mode);
/**
 * @brief Invoke the internal noop task used by task posting helpers.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tasks_test_call_noop(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_TASKS_H */
