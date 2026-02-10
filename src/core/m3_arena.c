#include "m3/m3_arena.h"

#define M3_ARENA_DEFAULT_BLOCK_SIZE 4096

#ifdef M3_TESTING
static M3Bool g_m3_arena_force_align_fail = M3_FALSE;
static M3Bool g_m3_arena_force_add_overflow = M3_FALSE;
#endif

struct M3ArenaBlock {
  struct M3ArenaBlock *next;
  m3_usize capacity;
  m3_usize offset;
  unsigned char data[1];
};

static m3_usize m3_usize_max_value(void) { return (m3_usize) ~(m3_usize)0; }

static int m3_arena_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  max_value = m3_usize_max_value();
  if (a > max_value - b) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return M3_OK;
}

static int m3_arena_align_offset(m3_usize offset, m3_usize alignment,
                                 m3_usize *out_aligned) {
  m3_usize mask;
  m3_usize max_value;

  mask = alignment - 1;
  max_value = m3_usize_max_value();
  if (offset > max_value - mask) {
    return M3_ERR_OVERFLOW;
  }

  *out_aligned = (offset + mask) & ~mask;
  return M3_OK;
}

static int m3_arena_allocate_block(M3Arena *arena, m3_usize capacity,
                                   M3ArenaBlock **out_block) {
  m3_usize header_size;
  m3_usize total_size;
  void *mem;
  M3ArenaBlock *block;
  int rc;

  header_size = sizeof(M3ArenaBlock) - 1;
  rc = m3_arena_add_overflow(header_size, capacity, &total_size);
  if (rc != M3_OK) {
    return rc;
  }

  rc = arena->allocator.alloc(arena->allocator.ctx, total_size, &mem);
  if (rc != M3_OK) {
    return rc;
  }

  block = (M3ArenaBlock *)mem;
  block->next = NULL;
  block->capacity = capacity;
  block->offset = 0;
  *out_block = block;
  return M3_OK;
}

int M3_CALL m3_arena_init(M3Arena *arena, const M3Allocator *allocator,
                          m3_usize block_size) {
  M3Allocator default_alloc;
  M3ArenaBlock *block;
  int rc;

  if (arena == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (arena->head != NULL || arena->current != NULL) {
    return M3_ERR_STATE;
  }

  if (allocator == NULL) {
    rc = m3_get_default_allocator(&default_alloc);
    if (rc != M3_OK) {
      return rc;
    }
    allocator = &default_alloc;
  }

  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (block_size == 0) {
    block_size = M3_ARENA_DEFAULT_BLOCK_SIZE;
  }

  arena->allocator = *allocator;
  arena->block_size = block_size;
  arena->head = NULL;
  arena->current = NULL;

  rc = m3_arena_allocate_block(arena, block_size, &block);
  if (rc != M3_OK) {
    arena->block_size = 0;
    return rc;
  }

  arena->head = block;
  arena->current = block;
  return M3_OK;
}

int M3_CALL m3_arena_reset(M3Arena *arena) {
  M3ArenaBlock *block;
  M3ArenaBlock *next;
  int rc;
  int first_error;

  if (arena == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (arena->head == NULL) {
    return M3_ERR_STATE;
  }

  first_error = M3_OK;
  block = arena->head->next;
  while (block != NULL) {
    next = block->next;
    rc = arena->allocator.free(arena->allocator.ctx, block);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
    block = next;
  }

  arena->head->next = NULL;
  arena->head->offset = 0;
  arena->current = arena->head;

  if (first_error != M3_OK) {
    return first_error;
  }

  return M3_OK;
}

int M3_CALL m3_arena_shutdown(M3Arena *arena) {
  M3ArenaBlock *block;
  M3ArenaBlock *next;
  int rc;
  int first_error;

  if (arena == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (arena->head == NULL) {
    arena->current = NULL;
    arena->block_size = 0;
    return M3_OK;
  }

  first_error = M3_OK;
  block = arena->head;
  while (block != NULL) {
    next = block->next;
    rc = arena->allocator.free(arena->allocator.ctx, block);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
    block = next;
  }

  arena->head = NULL;
  arena->current = NULL;
  arena->block_size = 0;

  if (first_error != M3_OK) {
    return first_error;
  }

  return M3_OK;
}

int M3_CALL m3_arena_alloc(M3Arena *arena, m3_usize size, m3_usize alignment,
                           void **out_ptr) {
  M3ArenaBlock *block;
  M3ArenaBlock *new_block;
  m3_usize aligned_offset;
  m3_usize new_offset;
  m3_usize remaining;
  m3_usize capacity;
  int rc;

  if (arena == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (arena->current == NULL) {
    return M3_ERR_STATE;
  }

  block = arena->current;
  rc = m3_arena_align_offset(block->offset, alignment, &aligned_offset);
  if (rc != M3_OK) {
    return rc;
  }

  if (aligned_offset > block->capacity) {
    remaining = 0;
  } else {
    remaining = block->capacity - aligned_offset;
  }

  if (remaining < size) {
    capacity = arena->block_size;
    if (capacity < size) {
      capacity = size;
    }

    rc = m3_arena_allocate_block(arena, capacity, &new_block);
    if (rc != M3_OK) {
      return rc;
    }

    block->next = new_block;
    arena->current = new_block;
    block = new_block;
    rc = m3_arena_align_offset(block->offset, alignment, &aligned_offset);
#ifdef M3_TESTING
    if (g_m3_arena_force_align_fail) {
      rc = M3_ERR_OVERFLOW;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_arena_add_overflow(aligned_offset, size, &new_offset);
#ifdef M3_TESTING
  if (g_m3_arena_force_add_overflow) {
    rc = M3_ERR_OVERFLOW;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  *out_ptr = (void *)(block->data + aligned_offset);
  block->offset = new_offset;
  return M3_OK;
}

int M3_CALL m3_arena_get_stats(const M3Arena *arena, M3ArenaStats *out_stats) {
  const M3ArenaBlock *block;
  m3_usize total_capacity;
  m3_usize total_used;
  m3_usize block_count;
  m3_usize temp_value;
  int rc;

  if (arena == NULL || out_stats == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (arena->head == NULL) {
    return M3_ERR_STATE;
  }

  total_capacity = 0;
  total_used = 0;
  block_count = 0;

  block = arena->head;
  while (block != NULL) {
    rc = m3_arena_add_overflow(total_capacity, block->capacity, &temp_value);
    if (rc != M3_OK) {
      return rc;
    }
    total_capacity = temp_value;

    rc = m3_arena_add_overflow(total_used, block->offset, &temp_value);
    if (rc != M3_OK) {
      return rc;
    }
    total_used = temp_value;

    block_count += 1;
    block = block->next;
  }

  out_stats->block_count = block_count;
  out_stats->total_capacity = total_capacity;
  out_stats->total_used = total_used;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_arena_test_set_force_align_fail(M3Bool enable) {
  g_m3_arena_force_align_fail = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_arena_test_set_force_add_overflow(M3Bool enable) {
  g_m3_arena_force_add_overflow = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}
#endif
