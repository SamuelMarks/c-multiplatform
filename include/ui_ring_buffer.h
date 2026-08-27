/**
 * @file ui_ring_buffer.h
 */
#ifndef UI_RING_BUFFER_H
#define UI_RING_BUFFER_H

/**
 * \file ui_ring_buffer.h
 * \brief UI Ring Buffer component.
 *
 * This file contains definitions for a lock-free Single-Producer
 * Single-Consumer (SPSC) ring buffer, alongside a multi-producer thread-safe
 * push mechanism.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Opaque handle for a ring buffer.
 */
struct ui_ring_buffer;

/**
 * @brief Creates a new lock-free ring buffer.
 *
 * @param item_size Size of each element in bytes.
 * @param capacity Maximum number of elements the buffer can hold.
 * @param out_buffer Pointer to receive the ring buffer handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_ring_buffer_create(size_t item_size, size_t capacity,
                                 struct ui_ring_buffer **out_buffer);

/**
 * @brief Destroys a ring buffer and frees its memory.
 *
 * @param buffer The buffer to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_ring_buffer_destroy(struct ui_ring_buffer *buffer);

/**
 * @brief Pushes an item into the ring buffer.
 *        Safe to call from a single producer thread.
 *
 * @param buffer The ring buffer.
 * @param item Pointer to the item data to copy into the buffer.
 * @return UI_ERROR_NONE on success, UI_ERROR_QUEUE_FULL if there is no space,
 *         or another appropriate error code.
 */
ui_error_t ui_ring_buffer_push(struct ui_ring_buffer *buffer, const void *item);

/**
 * @brief Pops an item from the ring buffer.
 *        Safe to call from a single consumer thread.
 *
 * @param buffer The ring buffer.
 * @param out_item Pointer to a pre-allocated buffer where the popped item will
 * be copied.
 * @return UI_ERROR_NONE on success, UI_ERROR_QUEUE_EMPTY if there are no items,
 *         or another appropriate error code.
 */
ui_error_t ui_ring_buffer_pop(struct ui_ring_buffer *buffer, void *out_item);

/**
 * @brief Pushes an item into the ring buffer safely from multiple threads.
 *        Uses a spinlock to ensure thread-safe multi-producer access.
 *
 * @param buffer The ring buffer.
 * @param item Pointer to the item data to copy into the buffer.
 * @return UI_ERROR_NONE on success, UI_ERROR_QUEUE_FULL if there is no space,
 *         or another appropriate error code.
 */
ui_error_t ui_ring_buffer_push_mp(struct ui_ring_buffer *buffer,
                                  const void *item);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RING_BUFFER_H */
