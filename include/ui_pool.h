#ifndef UI_POOL_H
#define UI_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stddef.h>
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Opaque handle representing an object pool.
 */
struct ui_pool;

/**
 * @brief Creates a new object pool.
 *
 * @param element_size The size of each element in the pool (must be > 0).
 * @param chunk_capacity Number of elements to allocate per chunk.
 * @param out_pool Pointer to receive the new pool handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_pool_create(size_t element_size, size_t chunk_capacity,
                             /** \brief ui_pool */
                             struct ui_pool **out_pool);

/**
 * @brief Destroys a pool and frees all its associated memory chunks.
 *
 * @param pool The pool to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if pool is NULL.
 */
enum ui_error ui_pool_destroy(struct ui_pool *pool);

/**
 * @brief Allocates an element from the pool.
 *
 * @param pool The pool to allocate from.
 * @param out_ptr Pointer to receive the allocated element's memory address.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_pool_alloc(struct ui_pool *pool, void **out_ptr);

/**
 * @brief Frees an element back to the pool, making it available for reuse.
 *
 * @param pool The pool the element belongs to.
 * @param ptr The element to free.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * inputs.
 */
enum ui_error ui_pool_free(struct ui_pool *pool, void *ptr);

/**
 * @brief Gets fragmentation metrics (e.g., number of available free elements).
 *
 * @param pool The pool to query.
 * @param out_free_count Pointer to receive the number of free elements.
 * @param out_total_capacity Pointer to receive the total capacity across all
 * chunks.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_pool_get_metrics(struct ui_pool *pool, size_t *out_free_count,
                                  size_t *out_total_capacity);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_POOL_H */
