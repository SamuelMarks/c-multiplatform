#ifndef M3_ARENA_H
#define M3_ARENA_H

/**
 * @file m3_arena.h
 * @brief Linear arena allocator for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_core.h"

/**
 * @brief Forward declaration for arena blocks.
 */
typedef struct M3ArenaBlock M3ArenaBlock;

/**
 * @brief Arena allocator state.
 */
typedef struct M3Arena {
    M3Allocator allocator; /**< Allocator used for arena blocks. */
    m3_usize block_size; /**< Default block size for new allocations. */
    M3ArenaBlock *head; /**< First block in the arena. */
    M3ArenaBlock *current; /**< Current block used for allocations. */
} M3Arena;

/**
 * @brief Arena memory usage statistics.
 */
typedef struct M3ArenaStats {
    m3_usize block_count; /**< Number of blocks currently allocated. */
    m3_usize total_capacity; /**< Total capacity across blocks in bytes. */
    m3_usize total_used; /**< Total used bytes across blocks. */
} M3ArenaStats;

/**
 * @brief Initialize an arena allocator.
 * @param arena Arena instance to initialize.
 * @param allocator Allocator used for arena blocks; NULL uses the default allocator.
 * @param block_size Default block size for allocations; 0 selects an internal default.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_arena_init(M3Arena *arena, const M3Allocator *allocator, m3_usize block_size);

/**
 * @brief Reset the arena to reuse allocated blocks.
 * @param arena Arena instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_arena_reset(M3Arena *arena);

/**
 * @brief Release all memory held by the arena.
 * @param arena Arena instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_arena_shutdown(M3Arena *arena);

/**
 * @brief Allocate memory from the arena.
 * @param arena Arena instance.
 * @param size Number of bytes to allocate.
 * @param alignment Alignment in bytes (power of two).
 * @param out_ptr Receives the aligned allocation pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_arena_alloc(M3Arena *arena, m3_usize size, m3_usize alignment, void **out_ptr);

/**
 * @brief Retrieve arena usage statistics.
 * @param arena Arena instance.
 * @param out_stats Receives current usage statistics.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_arena_get_stats(const M3Arena *arena, M3ArenaStats *out_stats);

#ifdef M3_TESTING
/**
 * @brief Force arena alignment failure during growth (tests only).
 * @param enable M3_TRUE to force failure, M3_FALSE to restore normal behavior.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_arena_test_set_force_align_fail(M3Bool enable);

/**
 * @brief Force arena offset overflow during allocation (tests only).
 * @param enable M3_TRUE to force failure, M3_FALSE to restore normal behavior.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_arena_test_set_force_add_overflow(M3Bool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_ARENA_H */
