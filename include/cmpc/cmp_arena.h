#ifndef CMP_ARENA_H
#define CMP_ARENA_H

/**
 * @file cmp_arena.h
 * @brief Linear arena allocator for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"

/**
 * @brief Forward declaration for arena blocks.
 */
typedef struct CMPArenaBlock CMPArenaBlock;

/**
 * @brief Arena allocator state.
 */
typedef struct CMPArena {
  CMPAllocator allocator; /**< Allocator used for arena blocks. */
  cmp_usize block_size;   /**< Default block size for new allocations. */
  CMPArenaBlock *head;    /**< First block in the arena. */
  CMPArenaBlock *current; /**< Current block used for allocations. */
} CMPArena;

/**
 * @brief Arena memory usage statistics.
 */
typedef struct CMPArenaStats {
  cmp_usize block_count;    /**< Number of blocks currently allocated. */
  cmp_usize total_capacity; /**< Total capacity across blocks in bytes. */
  cmp_usize total_used;     /**< Total used bytes across blocks. */
} CMPArenaStats;

/**
 * @brief Initialize an arena allocator.
 * @param arena Arena instance to initialize.
 * @param allocator Allocator used for arena blocks; NULL uses the default
 * allocator.
 * @param block_size Default block size for allocations; 0 selects an internal
 * default.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_arena_init(CMPArena *arena, const CMPAllocator *allocator,
                                 cmp_usize block_size);

/**
 * @brief Reset the arena to reuse allocated blocks.
 * @param arena Arena instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_arena_reset(CMPArena *arena);

/**
 * @brief Release all memory held by the arena.
 * @param arena Arena instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_arena_shutdown(CMPArena *arena);

/**
 * @brief Allocate memory from the arena.
 * @param arena Arena instance.
 * @param size Number of bytes to allocate.
 * @param alignment Alignment in bytes (power of two).
 * @param out_ptr Receives the aligned allocation pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_arena_alloc(CMPArena *arena, cmp_usize size,
                                  cmp_usize alignment, void **out_ptr);

/**
 * @brief Retrieve arena usage statistics.
 * @param arena Arena instance.
 * @param out_stats Receives current usage statistics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_arena_get_stats(const CMPArena *arena,
                                      CMPArenaStats *out_stats);

#ifdef CMP_TESTING
/**
 * @brief Force arena alignment failure during growth (tests only).
 * @param enable CMP_TRUE to force failure, CMP_FALSE to restore normal behavior.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_arena_test_set_force_align_fail(CMPBool enable);

/**
 * @brief Force arena offset overflow during allocation (tests only).
 * @param enable CMP_TRUE to force failure, CMP_FALSE to restore normal behavior.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_arena_test_set_force_add_overflow(CMPBool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_ARENA_H */
