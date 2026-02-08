#ifndef M3_STORE_H
#define M3_STORE_H

/**
 * @file m3_store.h
 * @brief State store and time-travel helpers for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_core.h"

/**
 * @brief Action payload descriptor.
 */
typedef struct M3Action {
    m3_u32 type; /**< Action type identifier. */
    const void *data; /**< Optional action payload bytes. */
    m3_usize size; /**< Payload size in bytes. */
} M3Action;

/**
 * @brief Reducer function signature.
 * @param ctx Reducer context pointer.
 * @param action Action descriptor.
 * @param prev_state Previous state bytes.
 * @param next_state Destination buffer for the next state.
 * @return M3_OK on success or a failure code.
 */
typedef int (M3_CALL *M3ReducerFn)(void *ctx, const M3Action *action, const void *prev_state, void *next_state);

/**
 * @brief Store configuration.
 */
typedef struct M3StoreConfig {
    const M3Allocator *allocator; /**< Allocator for store memory; NULL uses default allocator. */
    m3_usize state_size; /**< Size of the state in bytes. */
    m3_usize history_capacity; /**< Maximum number of undo/redo snapshots to keep. */
    M3ReducerFn reducer; /**< Reducer callback for state transitions. */
    void *reducer_ctx; /**< Reducer context pointer. */
} M3StoreConfig;

/**
 * @brief Store instance.
 */
typedef struct M3Store {
    M3Allocator allocator; /**< Allocator used for store memory. */
    M3ReducerFn reducer; /**< Reducer callback. */
    void *reducer_ctx; /**< Reducer context pointer. */
    m3_usize state_size; /**< State size in bytes. */
    m3_u8 *state; /**< Current state buffer. */
    m3_u8 *scratch; /**< Scratch buffer for reducer output. */
    m3_u8 *undo_buffer; /**< Undo history buffer. */
    m3_u8 *redo_buffer; /**< Redo history buffer. */
    m3_usize undo_capacity; /**< Undo capacity in snapshots. */
    m3_usize redo_capacity; /**< Redo capacity in snapshots. */
    m3_usize undo_count; /**< Undo snapshot count. */
    m3_usize redo_count; /**< Redo snapshot count. */
} M3Store;

/**
 * @brief Initialize a store.
 * @param store Store instance to initialize.
 * @param config Store configuration.
 * @param initial_state Optional initial state bytes (NULL zeros the state).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_init(M3Store *store, const M3StoreConfig *config, const void *initial_state);

/**
 * @brief Shut down a store and release its resources.
 * @param store Store instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_shutdown(M3Store *store);

/**
 * @brief Dispatch an action through the reducer.
 * @param store Store instance.
 * @param action Action descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_dispatch(M3Store *store, const M3Action *action);

/**
 * @brief Retrieve a pointer to the current state.
 * @param store Store instance.
 * @param out_state Receives the state pointer.
 * @param out_size Receives the state size in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_get_state_ptr(const M3Store *store, const void **out_state, m3_usize *out_size);

/**
 * @brief Copy the current state into a caller-provided buffer.
 * @param store Store instance.
 * @param out_state Destination buffer.
 * @param state_size Size of the destination buffer in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_get_state(const M3Store *store, void *out_state, m3_usize state_size);

/**
 * @brief Test whether an undo snapshot is available.
 * @param store Store instance.
 * @param out_can_undo Receives M3_TRUE if undo is available.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_can_undo(const M3Store *store, M3Bool *out_can_undo);

/**
 * @brief Test whether a redo snapshot is available.
 * @param store Store instance.
 * @param out_can_redo Receives M3_TRUE if redo is available.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_can_redo(const M3Store *store, M3Bool *out_can_redo);

/**
 * @brief Undo the most recent state transition.
 * @param store Store instance.
 * @return M3_OK on success, M3_ERR_NOT_FOUND if no undo is available, or a failure code.
 */
M3_API int M3_CALL m3_store_undo(M3Store *store);

/**
 * @brief Redo the most recent undone transition.
 * @param store Store instance.
 * @return M3_OK on success, M3_ERR_NOT_FOUND if no redo is available, or a failure code.
 */
M3_API int M3_CALL m3_store_redo(M3Store *store);

/**
 * @brief Clear undo/redo history.
 * @param store Store instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_clear_history(M3Store *store);

/**
 * @brief Get the number of undo snapshots.
 * @param store Store instance.
 * @param out_count Receives the undo count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_get_undo_count(const M3Store *store, m3_usize *out_count);

/**
 * @brief Get the number of redo snapshots.
 * @param store Store instance.
 * @param out_count Receives the redo count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_get_redo_count(const M3Store *store, m3_usize *out_count);

/**
 * @brief Copy an undo snapshot for debugging.
 * @param store Store instance.
 * @param index_from_latest 0-based index from the most recent undo snapshot.
 * @param out_state Destination buffer.
 * @param state_size Size of the destination buffer in bytes.
 * @return M3_OK on success, M3_ERR_NOT_FOUND if the index is out of range, or a failure code.
 */
M3_API int M3_CALL m3_store_copy_undo_state(const M3Store *store, m3_usize index_from_latest, void *out_state, m3_usize state_size);

/**
 * @brief Copy a redo snapshot for debugging.
 * @param store Store instance.
 * @param index_from_latest 0-based index from the most recent redo snapshot.
 * @param out_state Destination buffer.
 * @param state_size Size of the destination buffer in bytes.
 * @return M3_OK on success, M3_ERR_NOT_FOUND if the index is out of range, or a failure code.
 */
M3_API int M3_CALL m3_store_copy_redo_state(const M3Store *store, m3_usize index_from_latest, void *out_state, m3_usize state_size);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for overflow-safe multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_test_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value);

/**
 * @brief Test wrapper for pushing history snapshots.
 * @param buffer History buffer.
 * @param capacity Snapshot capacity.
 * @param state_size State size in bytes.
 * @param io_count In/out snapshot count.
 * @param state State bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_test_history_push(m3_u8 *buffer, m3_usize capacity, m3_usize state_size, m3_usize *io_count, const void *state);

/**
 * @brief Test wrapper for popping history snapshots.
 * @param buffer History buffer.
 * @param capacity Snapshot capacity.
 * @param state_size State size in bytes.
 * @param io_count In/out snapshot count.
 * @param out_state Receives state bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_test_history_pop(const m3_u8 *buffer, m3_usize capacity, m3_usize state_size, m3_usize *io_count, void *out_state);

/**
 * @brief Test wrapper for copying history snapshots.
 * @param buffer History buffer.
 * @param capacity Snapshot capacity.
 * @param state_size State size in bytes.
 * @param count Snapshot count.
 * @param index_from_latest Snapshot index.
 * @param out_state Receives state bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_store_test_copy_history(const m3_u8 *buffer, m3_usize capacity, m3_usize state_size, m3_usize count,
    m3_usize index_from_latest, void *out_state);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_STORE_H */
