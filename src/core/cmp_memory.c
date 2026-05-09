/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32)
long _InterlockedCompareExchange(long volatile *Destination, long Exchange, long Comperand);
#pragma intrinsic(_InterlockedCompareExchange)
#define CMP_MEM_LOCK() do { while (_InterlockedCompareExchange(&g_mem_lock, 1, 0) != 0) { } } while (0)   
#define CMP_MEM_UNLOCK() do { g_mem_lock = 0; } while (0)
#else
#include <unistd.h>
#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
#define CMP_MEM_LOCK() do { } while (0)
#else
#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
#define CMP_MEM_LOCK() do { } while (0)
#else
#define CMP_MEM_LOCK() do { while (__atomic_test_and_set(&g_mem_lock, __ATOMIC_ACQUIRE)) usleep(10); } while (0)
#endif
#endif
#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
#define CMP_MEM_UNLOCK() do { } while (0)
#else
#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
#define CMP_MEM_UNLOCK() do { } while (0)
#else
#define CMP_MEM_UNLOCK() do { __atomic_clear(&g_mem_lock, __ATOMIC_RELEASE); } while (0)
#endif
#endif
#endif
/* clang-format on */

static cmp_mem_record_t *g_mem_head = NULL;
#if defined(_WIN32)
static volatile long g_mem_lock = 0;
#else
static volatile char g_mem_lock = 0;
#endif

/**
 * @brief cmp_mem_alloc_tracked
 *
 * @param size Parameter description.
 * @param file Parameter description.
 * @param line Parameter description.
 * @param out_ptr Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mem_alloc_tracked(size_t size, const char *file, int line,
                          void **out_ptr) {
  int rc = CMP_SUCCESS;
  cmp_mem_record_t *record;

  if (out_ptr == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mem_alloc_tracked: out_ptr is NULL\n");
    return rc;
  }

  if (size == 0) {
    *out_ptr = NULL;
    return rc;
  }

  /* Check for overflow before allocating */
  if (size > ((size_t)-1) - sizeof(cmp_mem_record_t)) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_mem_alloc_tracked: size overflow\n");
    return rc;
  }

  record = (cmp_mem_record_t *)malloc(sizeof(cmp_mem_record_t) + size);
  if (record == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_mem_alloc_tracked: malloc failed\n");
    return rc;
  }

  record->ptr = (void *)(record + 1);
  record->size = size;
  record->file = file;
  record->line = line;

  CMP_MEM_LOCK();
  record->next = g_mem_head;
  g_mem_head = record;
  CMP_MEM_UNLOCK();

  *out_ptr = record->ptr;
  return rc;
}

/**
 * @brief cmp_mem_free_tracked
 *
 * @param ptr Parameter description.
 * @param file Parameter description.
 * @param line Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mem_free_tracked(void *ptr, const char *file, int line) {
  int rc = CMP_SUCCESS;
  cmp_mem_record_t *curr;
  cmp_mem_record_t *prev = NULL;
  (void)file; /* unused in non-debug mode, useful if we expand */
  (void)line;

  if (ptr == NULL) {
    return rc;
  }

  CMP_MEM_LOCK();
  curr = g_mem_head;
  while (curr != NULL) {
    if (curr->ptr == ptr) {
      if (prev == NULL) {
        g_mem_head = curr->next;
      } else {
        prev->next = curr->next;
      }
      CMP_MEM_UNLOCK();
      free(curr);
      return rc;
    }
    prev = curr;
    curr = curr->next;
  }
  CMP_MEM_UNLOCK();

  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG("cmp_mem_free_tracked: ptr not found in tracked allocations\n");
  return rc;
}

/**
 * @brief cmp_mem_check_leaks
 *
 * @return Returns leak count (which is an int, not an error code)
 */
int cmp_mem_check_leaks(void) {
  int rc = CMP_SUCCESS;
  cmp_mem_record_t *curr = g_mem_head;
  int leak_count = 0;
  (void)rc;

  while (curr != NULL) {
    fprintf(stderr, "CMP Memory Leak: %u bytes at %p (allocated in %s:%d)\n",
            (unsigned int)curr->size, curr->ptr, curr->file, curr->line);
    leak_count++;
    curr = curr->next;
  }

  if (leak_count == 0) {
    fprintf(stdout, "CMP Memory Check: No leaks detected.\n");
  } else {
    fprintf(stderr, "CMP Memory Check: %d leaks detected.\n", leak_count);
  }

  /* This function returns the leak count, which diverges from the "return rc"
     rule for errors, but it returns an int representing the count */
  rc = leak_count;
  return rc;
}

/**
 * @brief cmp_arena_init
 *
 * @param arena Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_arena_init(cmp_arena_t *arena, size_t size) {
  int rc = CMP_SUCCESS;

  if (arena == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_arena_init: arena is NULL\n");
    return rc;
  }

  if (size == 0) {
    arena->buffer = NULL;
    arena->capacity = 0;
    arena->offset = 0;
    return rc;
  }

  rc = CMP_MALLOC(size, (void **)&arena->buffer);
  if (rc != CMP_SUCCESS || arena->buffer == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_arena_init CMP_MALLOC: out of memory\n");
    return rc;
  }

  arena->capacity = size;
  arena->offset = 0;

  return rc;
}

/**
 * @brief cmp_arena_alloc
 *
 * @param arena Parameter description.
 * @param size Parameter description.
 * @param out_ptr Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_arena_alloc(cmp_arena_t *arena, size_t size, void **out_ptr) {
  int rc = CMP_SUCCESS;

  if (arena == NULL || out_ptr == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_arena_alloc: arena or out_ptr is NULL\n");
    return rc;
  }

  if (size == 0) {
    *out_ptr = NULL;
    return rc;
  }

  if (arena->offset + size > arena->capacity) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_arena_alloc (capacity exceeded)\n");
    return rc;
  }

  *out_ptr = arena->buffer + arena->offset;
  arena->offset += size;

  return rc;
}

/**
 * @brief cmp_arena_free
 *
 * @param arena Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_arena_free(cmp_arena_t *arena) {
  int rc = CMP_SUCCESS;

  if (arena == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_arena_free: arena is NULL\n");
    return rc;
  }

  if (arena->buffer != NULL) {
    rc = CMP_FREE(arena->buffer);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_arena_free: CMP_FREE failed\n");
      return rc;
    }
    arena->buffer = NULL;
  }

  arena->capacity = 0;
  arena->offset = 0;

  return rc;
}

/**
 * @brief cmp_pool_init
 *
 * @param pool Parameter description.
 * @param block_size Parameter description.
 * @param block_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pool_init(cmp_pool_t *pool, size_t block_size, size_t block_count) {
  int rc = CMP_SUCCESS;
  size_t i;
  size_t total_size;
  cmp_pool_block_t *block;

  if (pool == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_pool_init: pool is NULL\n");
    return rc;
  }

  if (block_size < sizeof(cmp_pool_block_t)) {
    block_size = sizeof(cmp_pool_block_t);
  }

  /* align block size to pointer size */
  if (block_size % sizeof(void *) != 0) {
    block_size += sizeof(void *) - (block_size % sizeof(void *));
  }

  if (block_size == 0 || block_count == 0) {
    pool->buffer = NULL;
    pool->free_list = NULL;
    pool->capacity = 0;
    pool->block_size = 0;
    return rc;
  }

  total_size = block_size * block_count;
  rc = CMP_MALLOC(total_size, (void **)&pool->buffer);
  if (rc != CMP_SUCCESS || pool->buffer == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_pool_init CMP_MALLOC: out of memory\n");
    return rc;
  }

  pool->capacity = block_count;
  pool->block_size = block_size;
  pool->free_list = NULL;

  /* initialize the free list */
  for (i = 0; i < block_count; ++i) {
    block = (cmp_pool_block_t *)(void *)(pool->buffer + (i * block_size));
    block->next = pool->free_list;
    pool->free_list = block;
  }

  return rc;
}

/**
 * @brief cmp_pool_alloc
 *
 * @param pool Parameter description.
 * @param out_ptr Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pool_alloc(cmp_pool_t *pool, void **out_ptr) {
  int rc = CMP_SUCCESS;
  cmp_pool_block_t *block;

  if (pool == NULL || out_ptr == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_pool_alloc: pool or out_ptr is NULL\n");
    return rc;
  }

  if (pool->capacity == 0) {
    *out_ptr = NULL;
    return rc;
  }

  if (pool->free_list == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_pool_alloc: no free blocks\n");
    return rc;
  }

  block = pool->free_list;
  pool->free_list = block->next;

  *out_ptr = (void *)block;
  return rc;
}

/**
 * @brief cmp_pool_free
 *
 * @param pool Parameter description.
 * @param ptr Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pool_free(cmp_pool_t *pool, void *ptr) {
  int rc = CMP_SUCCESS;
  cmp_pool_block_t *block;

  if (pool == NULL || ptr == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_pool_free: pool or ptr is NULL\n");
    return rc;
  }

  /* check if pointer is within the buffer bounds */
  if ((uint8_t *)ptr < pool->buffer ||
      (uint8_t *)ptr >= pool->buffer + (pool->capacity * pool->block_size)) {
    rc = CMP_ERROR_BOUNDS;
    LOG_DEBUG("cmp_pool_free: pointer out of bounds\n");
    return rc;
  }

  /* check alignment */
  if (((uint8_t *)ptr - pool->buffer) % pool->block_size != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_pool_free: pointer misaligned\n");
    return rc;
  }

  block = (cmp_pool_block_t *)ptr;
  block->next = pool->free_list;
  pool->free_list = block;

  return rc;
}

/**
 * @brief cmp_pool_destroy
 *
 * @param pool Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pool_destroy(cmp_pool_t *pool) {
  int rc = CMP_SUCCESS;

  if (pool == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_pool_destroy: pool is NULL\n");
    return rc;
  }

  if (pool->buffer != NULL) {
    rc = CMP_FREE(pool->buffer);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_pool_destroy: CMP_FREE failed\n");
      return rc;
    }
    pool->buffer = NULL;
  }

  pool->free_list = NULL;
  pool->capacity = 0;
  pool->block_size = 0;

  return rc;
}
