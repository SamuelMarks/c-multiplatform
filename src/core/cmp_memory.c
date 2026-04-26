/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32)
long _InterlockedCompareExchange(long volatile *Destination, long Exchange, long Comperand);
#pragma intrinsic(_InterlockedCompareExchange)
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#define CMP_MEM_LOCK() do { while (_InterlockedCompareExchange(&g_mem_lock, 1, 0) != 0) Sleep(0); } while (0)
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
  cmp_mem_record_t *record;

  if (out_ptr == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (size == 0) {
    *out_ptr = NULL;
    return CMP_SUCCESS;
  }

  /* Check for overflow before allocating */
  if (size > ((size_t)-1) - sizeof(cmp_mem_record_t)) {
    return CMP_ERROR_OOM;
  }

  record = (cmp_mem_record_t *)malloc(sizeof(cmp_mem_record_t) + size);
  if (record == NULL) {
    return CMP_ERROR_OOM;
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
  return CMP_SUCCESS;
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
  cmp_mem_record_t *curr;
  cmp_mem_record_t *prev = NULL;
  (void)file; /* unused in non-debug mode, useful if we expand */
  (void)line;

  if (ptr == NULL) {
    return CMP_SUCCESS;
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
      return CMP_SUCCESS;
    }
    prev = curr;
    curr = curr->next;
  }
  CMP_MEM_UNLOCK();

  return CMP_ERROR_NOT_FOUND;
}

/**
 * @brief cmp_mem_check_leaks
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mem_check_leaks(void) {
  cmp_mem_record_t *curr = g_mem_head;
  int leak_count = 0;

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

  return leak_count;
}

/**
 * @brief cmp_arena_init
 *
 * @param arena Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_arena_init(cmp_arena_t *arena, size_t size) {
  int rc;

  if (arena == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_arena_init: %s\n", err_str);
 }    return rc;
  }

  if (size == 0) {
    arena->buffer = NULL;
    arena->capacity = 0;
    arena->offset = 0;
    return CMP_SUCCESS;
  }

  rc = CMP_MALLOC(size, (void **)&arena->buffer);
  if (rc != CMP_SUCCESS || arena->buffer == NULL) {
    if (rc == CMP_SUCCESS) rc = CMP_ERROR_OOM;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_arena_init CMP_MALLOC: %s\n", err_str);
 }    return rc;
  }

  arena->capacity = size;
  arena->offset = 0;

  return CMP_SUCCESS;
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
  int rc;

  if (arena == NULL || out_ptr == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_arena_alloc: %s\n", err_str);
 }    return rc;
  }

  if (size == 0) {
    *out_ptr = NULL;
    return CMP_SUCCESS;
  }

  if (arena->offset + size > arena->capacity) {
    rc = CMP_ERROR_OOM;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_arena_alloc (capacity exceeded): %s\n", err_str);
 }    return rc;
  }

  *out_ptr = arena->buffer + arena->offset;
  arena->offset += size;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_arena_free
 *
 * @param arena Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_arena_free(cmp_arena_t *arena) {
  int rc;

  if (arena == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_arena_free: %s\n", err_str);
 }    return rc;
  }

  if (arena->buffer != NULL) {
    CMP_FREE(arena->buffer);
    arena->buffer = NULL;
  }

  arena->capacity = 0;
  arena->offset = 0;

  return CMP_SUCCESS;
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
  int rc;
  size_t i;
  size_t total_size;
  cmp_pool_block_t *block;

  if (pool == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_pool_init: %s\n", err_str);
 }    return rc;
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
    return CMP_SUCCESS;
  }

  total_size = block_size * block_count;
  rc = CMP_MALLOC(total_size, (void **)&pool->buffer);
  if (rc != CMP_SUCCESS || pool->buffer == NULL) {
    if (rc == CMP_SUCCESS) rc = CMP_ERROR_OOM;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_pool_init CMP_MALLOC: %s\n", err_str);
 }    return rc;
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

  return CMP_SUCCESS;
}

/**
 * @brief cmp_pool_alloc
 *
 * @param pool Parameter description.
 * @param out_ptr Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pool_alloc(cmp_pool_t *pool, void **out_ptr) {
  int rc;
  cmp_pool_block_t *block;

  if (pool == NULL || out_ptr == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_pool_alloc: %s\n", err_str);
 }    return rc;
  }

  if (pool->capacity == 0) {
    *out_ptr = NULL;
    return CMP_SUCCESS;
  }

  if (pool->free_list == NULL) {
    rc = CMP_ERROR_OOM;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_pool_alloc (no free blocks): %s\n", err_str);
 }    return rc;
  }

  block = pool->free_list;
  pool->free_list = block->next;

  *out_ptr = (void *)block;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_pool_free
 *
 * @param pool Parameter description.
 * @param ptr Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pool_free(cmp_pool_t *pool, void *ptr) {
  int rc;
  cmp_pool_block_t *block;

  if (pool == NULL || ptr == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_pool_free: %s\n", err_str);
 }    return rc;
  }

  /* check if pointer is within the buffer bounds */
  if ((uint8_t *)ptr < pool->buffer ||
      (uint8_t *)ptr >= pool->buffer + (pool->capacity * pool->block_size)) {
    rc = CMP_ERROR_BOUNDS;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_pool_free (out of bounds): %s\n", err_str);
 }    return rc;
  }

  /* check alignment */
  if (((uint8_t *)ptr - pool->buffer) % pool->block_size != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_pool_free (misaligned): %s\n", err_str);
 }    return rc;
  }

  block = (cmp_pool_block_t *)ptr;
  block->next = pool->free_list;
  pool->free_list = block;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_pool_destroy
 *
 * @param pool Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pool_destroy(cmp_pool_t *pool) {
  int rc;

  if (pool == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    { const char *err_str; cmp_strerror(rc, &err_str); LOG_DEBUG("cmp_pool_destroy: %s\n", err_str);
 }    return rc;
  }

  if (pool->buffer != NULL) {
    CMP_FREE(pool->buffer);
    pool->buffer = NULL;
  }

  pool->free_list = NULL;
  pool->capacity = 0;
  pool->block_size = 0;

  return CMP_SUCCESS;
}
