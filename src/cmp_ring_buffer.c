/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
long _InterlockedCompareExchange(long volatile *Destination, long Exchange, long Comperand);
#pragma intrinsic(_InterlockedCompareExchange)
#endif
/* clang-format on */

/**
 * @brief cmp_ring_buffer_init
 *
 * @param rb Parameter description.
 * @param capacity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ring_buffer_init(cmp_ring_buffer_t *rb, size_t capacity) {
  int rc = CMP_SUCCESS;

  if (rb == NULL || capacity == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ring_buffer_init: invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(capacity * sizeof(void *), (void **)&rb->buffer);
  if (rc != CMP_SUCCESS) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_ring_buffer_init: CMP_MALLOC failed\n");
    return rc;
  }
  memset(rb->buffer, 0, capacity * sizeof(void *));

  rb->capacity = capacity;
  rb->head = 0;
  rb->tail = 0;
  return rc;
}

#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
/**
 * @brief cmp_ring_buffer_push
 *
 * @param rb Parameter description.
 * @param item Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ring_buffer_push(cmp_ring_buffer_t *rb, void *item) {
  int rc = CMP_SUCCESS;
  size_t current_tail;
  size_t current_head;
  size_t next_head;

  if (rb == NULL || item == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ring_buffer_push: invalid argument\n");
    return rc;
  }

  current_tail = rb->tail;
  current_head = rb->head;
  next_head = (current_head + 1) % rb->capacity;

  if (next_head == current_tail) {
    rc = CMP_ERROR_BOUNDS;
    LOG_DEBUG("cmp_ring_buffer_push: buffer full\n");
    return rc;
  }

  rb->buffer[current_head] = item;
  rb->head = next_head;

  return rc;
}
#else
/**
 * @brief cmp_ring_buffer_push
 *
 * @param rb Parameter description.
 * @param item Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ring_buffer_push(cmp_ring_buffer_t *rb, void *item) {
  int rc = CMP_SUCCESS;
  size_t current_tail;
  size_t next_tail;
  size_t current_head;

  if (rb == NULL || item == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ring_buffer_push: invalid argument\n");
    return rc;
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
      rc = CMP_ERROR_BOUNDS;
      LOG_DEBUG("cmp_ring_buffer_push: buffer full\n");
      return rc;
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

  return rc;
}
#endif

#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
/**
 * @brief cmp_ring_buffer_pop
 *
 * @param rb Parameter description.
 * @param out_item Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ring_buffer_pop(cmp_ring_buffer_t *rb, void **out_item) {
  int rc = CMP_SUCCESS;
  size_t current_tail;
  size_t current_head;

  if (rb == NULL || out_item == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ring_buffer_pop: invalid argument\n");
    return rc;
  }

  current_tail = rb->tail;
  current_head = rb->head;

  if (current_head == current_tail) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_ring_buffer_pop: buffer empty\n");
    return rc;
  }

  *out_item = rb->buffer[current_tail];
  rb->tail = (current_tail + 1) % rb->capacity;

  return rc;
}
#else
/**
 * @brief cmp_ring_buffer_pop
 *
 * @param rb Parameter description.
 * @param out_item Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ring_buffer_pop(cmp_ring_buffer_t *rb, void **out_item) {
  int rc = CMP_SUCCESS;
  size_t current_head;
  size_t next_head;
  size_t current_tail;
  void *item;

  if (rb == NULL || out_item == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ring_buffer_pop: invalid argument\n");
    return rc;
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
      rc = CMP_ERROR_NOT_FOUND;
      LOG_DEBUG("cmp_ring_buffer_pop: buffer empty\n");
      return rc;
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
  return rc;
}
#endif

/**
 * @brief cmp_ring_buffer_destroy
 *
 * @param rb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ring_buffer_destroy(cmp_ring_buffer_t *rb) {
  int rc = CMP_SUCCESS;

  if (rb == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ring_buffer_destroy: invalid argument\n");
    return rc;
  }

  if (rb->buffer != NULL) {
    rc = CMP_FREE(rb->buffer);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ring_buffer_destroy: CMP_FREE failed\n");
      return rc;
    }
    rb->buffer = NULL;
  }

  rb->capacity = 0;
  rb->head = 0;
  rb->tail = 0;
  return rc;
}
