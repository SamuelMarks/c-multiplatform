/* clang-format off */
#include "cmp.h"
#include <stdlib.h>

#if defined(_WIN32)
long _InterlockedCompareExchange(long volatile *Destination, long Exchange, long Comperand);
#pragma intrinsic(_InterlockedCompareExchange)
#endif
/* clang-format on */

int cmp_ring_buffer_init(cmp_ring_buffer_t *rb, size_t capacity) {
  if (rb == NULL || capacity == 0) {
    return CMP_ERROR_INVALID_ARG;
  }

  rb->buffer = (void **)calloc(capacity, sizeof(void *));
  if (rb->buffer == NULL) {
    return CMP_ERROR_OOM;
  }

  rb->capacity = capacity;
  rb->head = 0;
  rb->tail = 0;

  return CMP_SUCCESS;
}

int cmp_ring_buffer_push(cmp_ring_buffer_t *rb, void *item) {
  size_t current_tail;
  size_t next_tail;
  size_t current_head;

  if (rb == NULL || item == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  do {
#if defined(_WIN32)
    current_tail = (size_t)rb->tail;
    current_head = (size_t)rb->head;
#else
    current_tail = __atomic_load_n(&rb->tail, __ATOMIC_RELAXED);
    current_head = __atomic_load_n(&rb->head, __ATOMIC_ACQUIRE);
#endif
    next_tail = (current_tail + 1) % rb->capacity;

    if (next_tail == current_head) {
      /* Buffer is full */
      return CMP_ERROR_BOUNDS;
    }

    /* Set the item BEFORE publishing the tail update */
    rb->buffer[current_tail] = item;

#if defined(_WIN32)
    if (_InterlockedCompareExchange(&rb->tail, (long)next_tail,
                                    (long)current_tail) == (long)current_tail) {
      break;
    }
#else
    if (__atomic_compare_exchange_n(&rb->tail, &current_tail, next_tail, 0,
                                    __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
      break;
    }
#endif
  } while (1);

  return CMP_SUCCESS;
}

int cmp_ring_buffer_pop(cmp_ring_buffer_t *rb, void **out_item) {
  size_t current_head;
  size_t next_head;
  size_t current_tail;
  void *item;

  if (rb == NULL || out_item == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  do {
#if defined(_WIN32)
    current_head = (size_t)rb->head;
    current_tail = (size_t)rb->tail;
#else
    current_head = __atomic_load_n(&rb->head, __ATOMIC_RELAXED);
    current_tail = __atomic_load_n(&rb->tail, __ATOMIC_ACQUIRE);
#endif

    if (current_head == current_tail) {
      /* Buffer is empty */
      return CMP_ERROR_NOT_FOUND;
    }

    next_head = (current_head + 1) % rb->capacity;

    /* Read the item BEFORE publishing the head update */
    item = rb->buffer[current_head];

#if defined(_WIN32)
    if (_InterlockedCompareExchange(&rb->head, (long)next_head,
                                    (long)current_head) == (long)current_head) {
      break;
    }
#else
    if (__atomic_compare_exchange_n(&rb->head, &current_head, next_head, 0,
                                    __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
      break;
    }
#endif
  } while (1);

  *out_item = item;

  return CMP_SUCCESS;
}

int cmp_ring_buffer_destroy(cmp_ring_buffer_t *rb) {
  if (rb == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (rb->buffer != NULL) {
    free(rb->buffer);
    rb->buffer = NULL;
  }

  rb->capacity = 0;
  rb->head = 0;
  rb->tail = 0;

  return CMP_SUCCESS;
}
