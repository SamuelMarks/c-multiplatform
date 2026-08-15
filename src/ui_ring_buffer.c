/* clang-format off */
#include "../include/ui_ring_buffer.h"
#include "../include/ui_atomic.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

struct ui_ring_buffer {
  ui_atomic_t head;
  ui_atomic_t tail;
  ui_atomic_t lock;
  size_t capacity; /* internally capacity + 1 */
  size_t item_size;
  void *buffer;
};

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

ui_error_t ui_ring_buffer_destroy(struct ui_ring_buffer *buffer) {
  if (!buffer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(buffer->buffer);
  C_MULTIPLATFORM_FREE(buffer);
  return UI_ERROR_NONE;
}

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

  memcpy((char *)buffer->buffer + (head * buffer->item_size), item,
         buffer->item_size);
  {
    ui_error_t _ign_rc = ui_atomic_store(&buffer->head, next_head);
    (void)_ign_rc;
  }

  return UI_ERROR_NONE;
}

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

  memcpy(out_item, (char *)buffer->buffer + (tail * buffer->item_size),
         buffer->item_size);

  next_tail = (tail + 1) % (long)buffer->capacity;
  {
    ui_error_t _ign_rc = ui_atomic_store(&buffer->tail, next_tail);
    (void)_ign_rc;
  }

  return UI_ERROR_NONE;
}

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
