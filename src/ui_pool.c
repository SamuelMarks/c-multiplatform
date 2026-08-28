/**
 * @file ui_pool.c
 * @brief Implementation of fixed-size block memory pooling.
 */

/* clang-format off */
#include <stddef.h>
#include "../include/ui_pool.h"
#include "../include/ui_types.h"
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct ui_pool_chunk
 * @brief Represents a single chunk of memory in the pool.
 */
struct ui_pool_chunk {
  struct ui_pool_chunk *next; /**< Pointer to the next chunk in the pool. */
  /* Data follows the chunk header */
};

/**
 * @struct ui_pool_free_node
 * @brief Represents a node in the free list.
 */
struct ui_pool_free_node {
  struct ui_pool_free_node *next; /**< Pointer to the next free node. */
};

/**
 * @struct ui_pool
 * @brief Main pool structure.
 */
struct ui_pool {
  struct ui_pool_chunk *chunk_head; /**< Pointer to the first chunk. */
  struct ui_pool_free_node
      *free_head;        /**< Pointer to the first free element. */
  size_t element_size;   /**< Size of each element. */
  size_t chunk_capacity; /**< Number of elements per chunk. */
  size_t total_capacity; /**< Total elements across all chunks. */
  size_t free_count;     /**< Number of free elements. */
};

/**
 * @brief Allocates a new chunk of memory for the pool and populates the free
 * list.
 *
 * @param[in,out] pool The pool to allocate a chunk for.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t allocate_chunk(struct ui_pool *pool) {
  struct ui_pool_chunk *chunk = NULL;
  size_t chunk_size = 0;
  char *data = NULL;
  size_t i;

  chunk_size = sizeof(struct ui_pool_chunk) +
               (pool->element_size * pool->chunk_capacity);
  chunk = (struct ui_pool_chunk *)C_MULTIPLATFORM_MALLOC(chunk_size);
  if (!chunk) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  chunk->next = pool->chunk_head;
  pool->chunk_head = chunk;

  data = (char *)chunk + sizeof(struct ui_pool_chunk);

  /* Add all elements in the new chunk to the free list */
  for (i = 0; i < pool->chunk_capacity; ++i) {
    struct ui_pool_free_node *node =
        (struct ui_pool_free_node *)(void *)(data + (i * pool->element_size));
    node->next = pool->free_head;
    pool->free_head = node;
  }

  pool->total_capacity += pool->chunk_capacity;
  pool->free_count += pool->chunk_capacity;

  return UI_ERROR_NONE;
}

/**
 * @brief Creates a new memory pool.
 * @param[in] element_size The size in bytes of each element.
 * @param[in] chunk_capacity The number of elements allocated per chunk.
 * @param[out] out_pool Pointer to store the created pool.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pool_create(size_t element_size, size_t chunk_capacity,
                          struct ui_pool **out_pool) {
  struct ui_pool *pool = NULL;

  if (!out_pool || element_size == 0 || chunk_capacity == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Ensure element size is at least as large as the free node structure */
  if (element_size < sizeof(struct ui_pool_free_node)) {
    element_size = sizeof(struct ui_pool_free_node);
  }

  /* Ensure element size is aligned to pointer size to prevent unaligned access
   * in free list */
  if (element_size % sizeof(void *) != 0) {
    element_size += sizeof(void *) - (element_size % sizeof(void *));
  }

  pool = (struct ui_pool *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_pool));
  if (!pool) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  pool->chunk_head = NULL;
  pool->free_head = NULL;
  pool->element_size = element_size;
  pool->chunk_capacity = chunk_capacity;
  pool->total_capacity = 0;
  pool->free_count = 0;

  *out_pool = pool;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a memory pool and frees all its chunks.
 * @param[in,out] pool The pool to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pool_destroy(struct ui_pool *pool) {
  struct ui_pool_chunk *current = NULL;
  struct ui_pool_chunk *next = NULL;

  if (!pool) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = pool->chunk_head;
  while (current) {
    next = current->next;
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }

  C_MULTIPLATFORM_FREE(pool);
  return UI_ERROR_NONE;
}

/**
 * @brief Allocates an element from the pool.
 * @param[in,out] pool The pool.
 * @param[out] out_ptr Pointer to store the allocated element address.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pool_alloc(struct ui_pool *pool, void **out_ptr) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!pool || !out_ptr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!pool->free_head) {
    rc = allocate_chunk(pool);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  *out_ptr = (void *)pool->free_head;
  pool->free_head = pool->free_head->next;
  pool->free_count--;

  return UI_ERROR_NONE;
}

/**
 * @brief Frees an element back to the pool.
 * @param[in,out] pool The pool.
 * @param[in] ptr The pointer to free.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pool_free(struct ui_pool *pool, void *ptr) {
  struct ui_pool_free_node *node = NULL;

  if (!pool || !ptr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  node = (struct ui_pool_free_node *)(void *)ptr;
  node->next = pool->free_head;
  pool->free_head = node;
  pool->free_count++;

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves current metrics for the memory pool.
 * @param[in] pool The pool.
 * @param[out] out_free_count Pointer to store the number of currently free
 * elements.
 * @param[out] out_total_capacity Pointer to store the total capacity across all
 * chunks.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_pool_get_metrics(struct ui_pool *pool, size_t *out_free_count,
                               size_t *out_total_capacity) {
  if (!pool) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (out_free_count) {
    *out_free_count = pool->free_count;
  }

  if (out_total_capacity) {
    *out_total_capacity = pool->total_capacity;
  }

  return UI_ERROR_NONE;
}
