#ifndef CMP_STORE_H
#define CMP_STORE_H

/**
 * @file cmp_store.h
 * @brief State store and time-travel helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"

/**
 * @brief Action payload descriptor.
 */
typedef struct CMPAction {
  cmp_u32 type;     /**< Action type identifier. */
  const void *data; /**< Optional action payload bytes. */
  cmp_usize size;   /**< Payload size in bytes. */
} CMPAction;

/**
 * @brief Reducer function signature.
 * @param ctx Reducer context pointer.
 * @param action Action descriptor.
 * @param prev_state Previous state bytes.
 * @param next_state Destination buffer for the next state.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPReducerFn)(void *ctx, const CMPAction *action,
                                    const void *prev_state, void *next_state);

/**
 * @brief Store configuration.
 */
typedef struct CMPStoreConfig {
  const CMPAllocator *allocator; /**< Allocator for store memory; NULL uses
                                   default allocator. */
  cmp_usize state_size;          /**< Size of the state in bytes. */
  cmp_usize
      history_capacity; /**< Maximum number of undo/redo snapshots to keep. */
  CMPReducerFn reducer; /**< Reducer callback for state transitions. */
  void *reducer_ctx;    /**< Reducer context pointer. */
} CMPStoreConfig;

/**
 * @brief Store instance.
 */
typedef struct CMPStore {
  CMPAllocator allocator;  /**< Allocator used for store memory. */
  CMPReducerFn reducer;    /**< Reducer callback. */
  void *reducer_ctx;       /**< Reducer context pointer. */
  cmp_usize state_size;    /**< State size in bytes. */
  cmp_u8 *state;           /**< Current state buffer. */
  cmp_u8 *scratch;         /**< Scratch buffer for reducer output. */
  cmp_u8 *undo_buffer;     /**< Undo history buffer. */
  cmp_u8 *redo_buffer;     /**< Redo history buffer. */
  cmp_usize undo_capacity; /**< Undo capacity in snapshots. */
  cmp_usize redo_capacity; /**< Redo capacity in snapshots. */
  cmp_usize undo_count;    /**< Undo snapshot count. */
  cmp_usize redo_count;    /**< Redo snapshot count. */
} CMPStore;

/**
 * @brief Initialize a store.
 * @param store Store instance to initialize.
 * @param config Store configuration.
 * @param initial_state Optional initial state bytes (NULL zeros the state).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_init(CMPStore *store,
                                    const CMPStoreConfig *config,
                                    const void *initial_state);

/**
 * @brief Shut down a store and release its resources.
 * @param store Store instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_shutdown(CMPStore *store);

/**
 * @brief Dispatch an action through the reducer.
 * @param store Store instance.
 * @param action Action descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_dispatch(CMPStore *store,
                                        const CMPAction *action);

/**
 * @brief Retrieve a pointer to the current state.
 * @param store Store instance.
 * @param out_state Receives the state pointer.
 * @param out_size Receives the state size in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_get_state_ptr(const CMPStore *store,
                                             const void **out_state,
                                             cmp_usize *out_size);

/**
 * @brief Copy the current state into a caller-provided buffer.
 * @param store Store instance.
 * @param out_state Destination buffer.
 * @param state_size Size of the destination buffer in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_get_state(const CMPStore *store, void *out_state,
                                         cmp_usize state_size);

/**
 * @brief Test whether an undo snapshot is available.
 * @param store Store instance.
 * @param out_can_undo Receives CMP_TRUE if undo is available.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_can_undo(const CMPStore *store,
                                        CMPBool *out_can_undo);

/**
 * @brief Test whether a redo snapshot is available.
 * @param store Store instance.
 * @param out_can_redo Receives CMP_TRUE if redo is available.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_can_redo(const CMPStore *store,
                                        CMPBool *out_can_redo);

/**
 * @brief Undo the most recent state transition.
 * @param store Store instance.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if no undo is available, or a
 * failure code.
 */
CMP_API int CMP_CALL cmp_store_undo(CMPStore *store);

/**
 * @brief Redo the most recent undone transition.
 * @param store Store instance.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if no redo is available, or a
 * failure code.
 */
CMP_API int CMP_CALL cmp_store_redo(CMPStore *store);

/**
 * @brief Clear undo/redo history.
 * @param store Store instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_clear_history(CMPStore *store);

/**
 * @brief Get the number of undo snapshots.
 * @param store Store instance.
 * @param out_count Receives the undo count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_get_undo_count(const CMPStore *store,
                                              cmp_usize *out_count);

/**
 * @brief Get the number of redo snapshots.
 * @param store Store instance.
 * @param out_count Receives the redo count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_get_redo_count(const CMPStore *store,
                                              cmp_usize *out_count);

/**
 * @brief Copy an undo snapshot for debugging.
 * @param store Store instance.
 * @param index_from_latest 0-based index from the most recent undo snapshot.
 * @param out_state Destination buffer.
 * @param state_size Size of the destination buffer in bytes.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if the index is out of range, or
 * a failure code.
 */
CMP_API int CMP_CALL cmp_store_copy_undo_state(const CMPStore *store,
                                               cmp_usize index_from_latest,
                                               void *out_state,
                                               cmp_usize state_size);

/**
 * @brief Copy a redo snapshot for debugging.
 * @param store Store instance.
 * @param index_from_latest 0-based index from the most recent redo snapshot.
 * @param out_state Destination buffer.
 * @param state_size Size of the destination buffer in bytes.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if the index is out of range, or
 * a failure code.
 */
CMP_API int CMP_CALL cmp_store_copy_redo_state(const CMPStore *store,
                                               cmp_usize index_from_latest,
                                               void *out_state,
                                               cmp_usize state_size);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for overflow-safe multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_test_mul_overflow(cmp_usize a, cmp_usize b,
                                                 cmp_usize *out_value);

/**
 * @brief Test wrapper for pushing history snapshots.
 * @param buffer History buffer.
 * @param capacity Snapshot capacity.
 * @param state_size State size in bytes.
 * @param io_count In/out snapshot count.
 * @param state State bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_test_history_push(cmp_u8 *buffer,
                                                 cmp_usize capacity,
                                                 cmp_usize state_size,
                                                 cmp_usize *io_count,
                                                 const void *state);

/**
 * @brief Test wrapper for popping history snapshots.
 * @param buffer History buffer.
 * @param capacity Snapshot capacity.
 * @param state_size State size in bytes.
 * @param io_count In/out snapshot count.
 * @param out_state Receives state bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_test_history_pop(const cmp_u8 *buffer,
                                                cmp_usize capacity,
                                                cmp_usize state_size,
                                                cmp_usize *io_count,
                                                void *out_state);

/**
 * @brief Test wrapper for copying history snapshots.
 * @param buffer History buffer.
 * @param capacity Snapshot capacity.
 * @param state_size State size in bytes.
 * @param count Snapshot count.
 * @param index_from_latest Snapshot index.
 * @param out_state Receives state bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_store_test_copy_history(
    const cmp_u8 *buffer, cmp_usize capacity, cmp_usize state_size,
    cmp_usize count, cmp_usize index_from_latest, void *out_state);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_STORE_H */
