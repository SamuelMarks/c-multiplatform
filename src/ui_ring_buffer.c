/**
 * @file ui_ring_buffer.c
 * @brief ui_ring_buffer.c implementation.
 */
/*
 * \file ui_ring_buffer.c
 * \brief Implementation of the UI Ring Buffer component.
 */

/* clang-format off */
#include "ui_ring_buffer.h"
#include "ui_atomic.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/**
 * @struct ui_ring_buffer
 * \brief Internal structure representing a ring buffer.
 */
struct ui_ring_buffer {
  ui_atomic_t head; /**< Head index (write position) */
  ui_atomic_t tail; /**< Tail index (read position) */
  ui_atomic_t lock; /**< Spinlock for multi-producer writes */
  size_t capacity;  /**< Actual capacity + 1 for empty/full distinction */
  size_t item_size; /**< Size of each item in bytes */
  void *buffer;     /**< Pointer to backing memory array */
};

/**
 * \brief Creates a new lock-free ring buffer.
 *
 * \param item_size Size of each element in bytes.
 * \param capacity Maximum number of elements the buffer can hold.
 * \param out_buffer Pointer to receive the ring buffer handle.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_ring_buffer_create(size_t item_size, size_t capacity,
                                 struct ui_ring_buffer **out_buffer) {
  struct ui_ring_buffer *rb = NULL;
  size_t actual_capacity;

  if (item_size == 0 || capacity == 0 || !out_buffer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* To distinguish between empty and full, the actual backing array must be 1
   * larger than requested */
  actual_capacity = capacity + 1;

  rb = (struct ui_ring_buffer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_ring_buffer));
  if (!rb) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rb->head = 0;
  rb->tail = 0;
  rb->lock = 0;
  rb->capacity = actual_capacity;
  rb->item_size = item_size;
  rb->buffer = C_MULTIPLATFORM_MALLOC(item_size * actual_capacity);

  if (!rb->buffer) {
    C_MULTIPLATFORM_FREE(rb);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  *out_buffer = rb;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a ring buffer and frees its memory.
 *
 * \param buffer The buffer to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_ring_buffer_destroy(struct ui_ring_buffer *buffer) {
  if (!buffer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(buffer->buffer);
  C_MULTIPLATFORM_FREE(buffer);
  return UI_ERROR_NONE;
}

/**
 * \brief Pushes an item into the ring buffer.
 *        Safe to call from a single producer thread.
 *
 * \param buffer The ring buffer.
 * \param item Pointer to the item data to copy into the buffer.
 * \return UI_ERROR_NONE on success, UI_ERROR_QUEUE_FULL if there is no space,
 *         or another appropriate error code.
 */
ui_error_t ui_ring_buffer_push(struct ui_ring_buffer *buffer,
                               const void *item) {
  long head;
  long tail;
  long next_head;

  if (!buffer || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  {
    ui_error_t _ign_rc = ui_atomic_load(&buffer->head, &head);
    (void)_ign_rc;
  }
  {
    ui_error_t _ign_rc = ui_atomic_load(&buffer->tail, &tail);
    (void)_ign_rc;
  }

  next_head = (head + 1) % (long)buffer->capacity;

  if (next_head == tail) {
    return UI_ERROR_QUEUE_FULL;
  }

  memcpy((char *)buffer->buffer + ((size_t)head * buffer->item_size), item,
         buffer->item_size);
  {
    ui_error_t _ign_rc = ui_atomic_store(&buffer->head, next_head);
    (void)_ign_rc;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Pops an item from the ring buffer.
 *        Safe to call from a single consumer thread.
 *
 * \param buffer The ring buffer.
 * \param out_item Pointer to a pre-allocated buffer where the popped item will
 * be copied.
 * \return UI_ERROR_NONE on success, UI_ERROR_QUEUE_EMPTY if there are no items,
 *         or another appropriate error code.
 */
ui_error_t ui_ring_buffer_pop(struct ui_ring_buffer *buffer, void *out_item) {
  long head;
  long tail;
  long next_tail;

  if (!buffer || !out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  {
    ui_error_t _ign_rc = ui_atomic_load(&buffer->head, &head);
    (void)_ign_rc;
  }
  {
    ui_error_t _ign_rc = ui_atomic_load(&buffer->tail, &tail);
    (void)_ign_rc;
  }

  if (head == tail) {
    return UI_ERROR_QUEUE_EMPTY;
  }

  memcpy(out_item, (char *)buffer->buffer + ((size_t)tail * buffer->item_size),
         buffer->item_size);

  next_tail = (tail + 1) % (long)buffer->capacity;
  {
    ui_error_t _ign_rc = ui_atomic_store(&buffer->tail, next_tail);
    (void)_ign_rc;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Pushes an item into the ring buffer safely from multiple threads.
 *        Uses a spinlock to ensure thread-safe multi-producer access.
 *
 * \param buffer The ring buffer.
 * \param item Pointer to the item data to copy into the buffer.
 * \return UI_ERROR_NONE on success, UI_ERROR_QUEUE_FULL if there is no space,
 *         or another appropriate error code.
 */
ui_error_t ui_ring_buffer_push_mp(struct ui_ring_buffer *buffer,
                                  const void *item) {
  int swapped = 0;
  ui_error_t rc;

  if (!buffer || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Simple spinlock */
  while (1) {
    ui_error_t _ign_rc = ui_atomic_cas(&buffer->lock, 0, 1, &swapped);
    (void)_ign_rc;
    if (swapped != 0)
      break;
  }

  rc = ui_ring_buffer_push(buffer, item);
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t _ign_rc = ui_atomic_store(&buffer->lock, 0);
      (void)_ign_rc;
    }
    return rc;
  }

  {
    ui_error_t _ign_rc = ui_atomic_store(&buffer->lock, 0);
    (void)_ign_rc;
  }

  return rc;
}
