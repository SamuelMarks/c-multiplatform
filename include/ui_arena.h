#ifndef UI_ARENA_H
#define UI_ARENA_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stddef.h>
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a memory arena.
 */
struct ui_arena;

/**
 * @brief Creates a new memory arena.
 *
 * @param block_size The size of each contiguous memory block the arena
 * allocates.
 * @param out_arena Pointer to receive the new arena handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_arena_create(size_t block_size, struct ui_arena **out_arena);

/**
 * @brief Destroys an arena and frees all its associated memory blocks.
 *
 * @param arena The arena to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if arena is NULL.
 */
ui_error_t ui_arena_destroy(struct ui_arena *arena);

/**
 * @brief Allocates memory from the arena.
 *
 * @param arena The arena to allocate from.
 * @param size The number of bytes to allocate.
 * @param alignment The alignment requirement (must be a power of two).
 * @param out_ptr Pointer to receive the allocated memory address.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_arena_alloc(struct ui_arena *arena, size_t size, size_t alignment,
                          void **out_ptr);

/**
 * @brief Resets the arena, invalidating all allocations but retaining the
 * blocks for reuse.
 *
 * @param arena The arena to reset.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_arena_reset(struct ui_arena *arena);

/**
 * @brief Represents a savepoint in the arena for rollback.
 */
struct ui_arena_savepoint {
  struct ui_arena_block *block; /**< The block active at the savepoint. */
  size_t used; /**< The amount of memory used in the block at the savepoint. */
};
/**
 * @brief Saves the current allocation state of the arena.
 *
 * @param arena The arena to save.
 * @param out_sp Pointer to receive the savepoint.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_arena_save(struct ui_arena *arena,
                         struct ui_arena_savepoint *out_sp);
/**
 * @brief Restores the arena to a previously saved state.
 *
 * @param arena The arena to restore.
 * @param sp The savepoint to restore from.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_arena_restore(struct ui_arena *arena,
                            struct ui_arena_savepoint sp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ARENA_H */
