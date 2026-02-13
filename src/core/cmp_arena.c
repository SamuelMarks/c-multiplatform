#include "cmpc/cmp_arena.h"

#define CMP_ARENA_DEFAULT_BLOCK_SIZE 4096

#ifdef CMP_TESTING
static CMPBool g_cmp_arena_force_align_fail = CMP_FALSE;
static CMPBool g_cmp_arena_force_add_overflow = CMP_FALSE;
#endif

struct CMPArenaBlock {
  struct CMPArenaBlock *next;
  cmp_usize capacity;
  cmp_usize offset;
  unsigned char data[1];
};

static cmp_usize cmp_usize_max_value(void) { return (cmp_usize) ~(cmp_usize)0; }

static int cmp_arena_add_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  cmp_usize max_value;

  max_value = cmp_usize_max_value();
  if (a > max_value - b) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return CMP_OK;
}

static int cmp_arena_align_offset(cmp_usize offset, cmp_usize alignment,
                                 cmp_usize *out_aligned) {
  cmp_usize mask;
  cmp_usize max_value;

  mask = alignment - 1;
  max_value = cmp_usize_max_value();
  if (offset > max_value - mask) {
    return CMP_ERR_OVERFLOW;
  }

  *out_aligned = (offset + mask) & ~mask;
  return CMP_OK;
}

static int cmp_arena_allocate_block(CMPArena *arena, cmp_usize capacity,
                                   CMPArenaBlock **out_block) {
  cmp_usize header_size;
  cmp_usize total_size;
  void *mem;
  CMPArenaBlock *block;
  int rc;

  header_size = sizeof(CMPArenaBlock) - 1;
  rc = cmp_arena_add_overflow(header_size, capacity, &total_size);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = arena->allocator.alloc(arena->allocator.ctx, total_size, &mem);
  if (rc != CMP_OK) {
    return rc;
  }

  block = (CMPArenaBlock *)mem;
  block->next = NULL;
  block->capacity = capacity;
  block->offset = 0;
  *out_block = block;
  return CMP_OK;
}

int CMP_CALL cmp_arena_init(CMPArena *arena, const CMPAllocator *allocator,
                          cmp_usize block_size) {
  CMPAllocator default_alloc;
  CMPArenaBlock *block;
  int rc;

  if (arena == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (arena->head != NULL || arena->current != NULL) {
    return CMP_ERR_STATE;
  }

  if (allocator == NULL) {
    rc = cmp_get_default_allocator(&default_alloc);
    if (rc != CMP_OK) {
      return rc;
    }
    allocator = &default_alloc;
  }

  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (block_size == 0) {
    block_size = CMP_ARENA_DEFAULT_BLOCK_SIZE;
  }

  arena->allocator = *allocator;
  arena->block_size = block_size;
  arena->head = NULL;
  arena->current = NULL;

  rc = cmp_arena_allocate_block(arena, block_size, &block);
  if (rc != CMP_OK) {
    arena->block_size = 0;
    return rc;
  }

  arena->head = block;
  arena->current = block;
  return CMP_OK;
}

int CMP_CALL cmp_arena_reset(CMPArena *arena) {
  CMPArenaBlock *block;
  CMPArenaBlock *next;
  int rc;
  int first_error;

  if (arena == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (arena->head == NULL) {
    return CMP_ERR_STATE;
  }

  first_error = CMP_OK;
  block = arena->head->next;
  while (block != NULL) {
    next = block->next;
    rc = arena->allocator.free(arena->allocator.ctx, block);
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
    block = next;
  }

  arena->head->next = NULL;
  arena->head->offset = 0;
  arena->current = arena->head;

  if (first_error != CMP_OK) {
    return first_error;
  }

  return CMP_OK;
}

int CMP_CALL cmp_arena_shutdown(CMPArena *arena) {
  CMPArenaBlock *block;
  CMPArenaBlock *next;
  int rc;
  int first_error;

  if (arena == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (arena->head == NULL) {
    arena->current = NULL;
    arena->block_size = 0;
    return CMP_OK;
  }

  first_error = CMP_OK;
  block = arena->head;
  while (block != NULL) {
    next = block->next;
    rc = arena->allocator.free(arena->allocator.ctx, block);
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
    block = next;
  }

  arena->head = NULL;
  arena->current = NULL;
  arena->block_size = 0;

  if (first_error != CMP_OK) {
    return first_error;
  }

  return CMP_OK;
}

int CMP_CALL cmp_arena_alloc(CMPArena *arena, cmp_usize size, cmp_usize alignment,
                           void **out_ptr) {
  CMPArenaBlock *block;
  CMPArenaBlock *new_block;
  cmp_usize aligned_offset;
  cmp_usize new_offset;
  cmp_usize remaining;
  cmp_usize capacity;
  int rc;

  if (arena == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (arena->current == NULL) {
    return CMP_ERR_STATE;
  }

  block = arena->current;
  rc = cmp_arena_align_offset(block->offset, alignment, &aligned_offset);
  if (rc != CMP_OK) {
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

    rc = cmp_arena_allocate_block(arena, capacity, &new_block);
    if (rc != CMP_OK) {
      return rc;
    }

    block->next = new_block;
    arena->current = new_block;
    block = new_block;
    rc = cmp_arena_align_offset(block->offset, alignment, &aligned_offset);
#ifdef CMP_TESTING
    if (g_cmp_arena_force_align_fail) {
      rc = CMP_ERR_OVERFLOW;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = cmp_arena_add_overflow(aligned_offset, size, &new_offset);
#ifdef CMP_TESTING
  if (g_cmp_arena_force_add_overflow) {
    rc = CMP_ERR_OVERFLOW;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  *out_ptr = (void *)(block->data + aligned_offset);
  block->offset = new_offset;
  return CMP_OK;
}

int CMP_CALL cmp_arena_get_stats(const CMPArena *arena, CMPArenaStats *out_stats) {
  const CMPArenaBlock *block;
  cmp_usize total_capacity;
  cmp_usize total_used;
  cmp_usize block_count;
  cmp_usize temp_value;
  int rc;

  if (arena == NULL || out_stats == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (arena->head == NULL) {
    return CMP_ERR_STATE;
  }

  total_capacity = 0;
  total_used = 0;
  block_count = 0;

  block = arena->head;
  while (block != NULL) {
    rc = cmp_arena_add_overflow(total_capacity, block->capacity, &temp_value);
    if (rc != CMP_OK) {
      return rc;
    }
    total_capacity = temp_value;

    rc = cmp_arena_add_overflow(total_used, block->offset, &temp_value);
    if (rc != CMP_OK) {
      return rc;
    }
    total_used = temp_value;

    block_count += 1;
    block = block->next;
  }

  out_stats->block_count = block_count;
  out_stats->total_capacity = total_capacity;
  out_stats->total_used = total_used;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_arena_test_set_force_align_fail(CMPBool enable) {
  g_cmp_arena_force_align_fail = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_arena_test_set_force_add_overflow(CMPBool enable) {
  g_cmp_arena_force_add_overflow = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif
