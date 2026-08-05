/* clang-format off */
#include <stddef.h>
#include "../include/ui_arena.h"
#include "../include/ui_types.h"
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @brief Represents a single block of memory in the arena.
 */
struct ui_arena_block {
  struct ui_arena_block *next; /**< Pointer to the next block in the arena */
  size_t size;                 /**< Total size of the block's data segment */
  size_t used; /**< Number of bytes currently used in this block */
  /* Data follows the block header */
};

/**
 * @brief Main arena structure that manages a linked list of memory blocks.
 */
struct ui_arena {
  struct ui_arena_block *head; /**< Pointer to the first block */
  struct ui_arena_block
      *current; /**< Pointer to the block currently being allocated from */
  size_t
      default_block_size; /**< Default size to use when allocating new blocks */
};

/**
 * @brief Allocates a new block of memory for the arena.
 *
 * @param size The size of the data segment for the new block.
 * @param out_block Pointer to receive the allocated block.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t allocate_block(size_t size,
                                 struct ui_arena_block **out_block) {
  struct ui_arena_block *block = NULL;

  block = (struct ui_arena_block *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_arena_block) + size);
  if (!block) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  block->next = NULL;
  block->size = size;
  block->used = 0;

  *out_block = block;
  return UI_ERROR_NONE;
}

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
ui_error_t ui_arena_create(size_t block_size, struct ui_arena **out_arena) {
  struct ui_arena *arena = NULL;

  if (!out_arena || block_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  arena = (struct ui_arena *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_arena));
  if (!arena) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  arena->head = NULL;
  arena->current = NULL;
  arena->default_block_size = block_size;

  *out_arena = arena;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys an arena and frees all its associated memory blocks.
 *
 * @param arena The arena to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if arena is NULL.
 */
ui_error_t ui_arena_destroy(struct ui_arena *arena) {
  struct ui_arena_block *current = NULL;
  struct ui_arena_block *next = NULL;

  if (!arena) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = arena->head;
  while (current) {
    next = current->next;
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }

  C_MULTIPLATFORM_FREE(arena);
  return UI_ERROR_NONE;
}

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
                          void **out_ptr) {
  struct ui_arena_block *block = NULL;
  size_t padding = 0;
  ui_uintptr current_addr = 0;
  ui_uintptr aligned_addr = 0;

  if (!arena || !out_ptr || alignment == 0 ||
      (alignment & (alignment - 1)) != 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  block = arena->current;

  while (block) {
    current_addr = (ui_uintptr)((char *)block + sizeof(struct ui_arena_block) +
                                block->used);
    aligned_addr =
        (current_addr + alignment - 1) & ~(ui_uintptr)(alignment - 1);
    padding = (size_t)(aligned_addr - current_addr);

    if (block->used + padding + size <= block->size) {
      block->used += padding + size;
      arena->current = block;
      *out_ptr = (void *)aligned_addr;
      return UI_ERROR_NONE;
    }

    if (block->next) {
      block = block->next;
    } else {
      break;
    }
  }

  /* Need a new block */
  {
    struct ui_arena_block *new_block = NULL;
    size_t new_block_size = arena->default_block_size;
    size_t required_size = size + alignment; /* safe estimation */
    ui_error_t rc;

    if (required_size > new_block_size) {
      new_block_size = required_size;
    }

    rc = allocate_block(new_block_size, &new_block);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    if (block) {
      block->next = new_block;
    } else {
      arena->head = new_block;
    }

    arena->current = new_block;

    current_addr =
        (ui_uintptr)((char *)new_block + sizeof(struct ui_arena_block));
    aligned_addr =
        (current_addr + alignment - 1) & ~(ui_uintptr)(alignment - 1);
    padding = (size_t)(aligned_addr - current_addr);

    new_block->used = padding + size;
    *out_ptr = (void *)aligned_addr;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Resets the arena, invalidating all allocations but retaining the
 * blocks for reuse.
 *
 * @param arena The arena to reset.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_arena_reset(struct ui_arena *arena) {
  struct ui_arena_block *current = NULL;

  if (!arena) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = arena->head;
  while (current) {
#ifndef NDEBUG
    memset((char *)current + sizeof(struct ui_arena_block), 0, current->size);
#endif
    current->used = 0;
    current = current->next;
  }

  arena->current = arena->head;
  return UI_ERROR_NONE;
}

/**
 * @brief Saves the current allocation state of the arena.
 *
 * @param arena The arena to save.
 * @param out_sp Pointer to receive the savepoint.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_arena_save(struct ui_arena *arena,
                         struct ui_arena_savepoint *out_sp) {
  if (!arena || !out_sp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_sp->block = arena->current;
  if (arena->current) {
    out_sp->used = arena->current->used;
  } else {
    out_sp->used = 0;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Restores the arena to a previously saved state.
 *
 * @param arena The arena to restore.
 * @param sp The savepoint to restore from.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_arena_restore(struct ui_arena *arena,
                            struct ui_arena_savepoint sp) {
  struct ui_arena_block *current = NULL;

  if (!arena) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* If block is NULL, it means the savepoint was taken when arena was
   * completely empty */
  if (!sp.block) {
    return ui_arena_reset(arena);
  }

  arena->current = (struct ui_arena_block *)sp.block;
  arena->current->used = sp.used;

  current = arena->current->next;
  while (current) {
#ifndef NDEBUG
    memset((char *)current + sizeof(struct ui_arena_block), 0, current->size);
#endif
    current->used = 0;
    current = current->next;
  }

#ifndef NDEBUG
  {
    size_t zero_size = arena->current->size - arena->current->used;
    if (zero_size > 0) {
      memset((char *)arena->current + sizeof(struct ui_arena_block) +
                 arena->current->used,
             0, zero_size);
    }
  }
#endif

  return UI_ERROR_NONE;
}
