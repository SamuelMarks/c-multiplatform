#include "cmpc/cmp_store.h"

#include <string.h>

static cmp_usize cmp_usize_max_value(void) { return (cmp_usize) ~(cmp_usize)0; }

static int cmp_store_mul_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = cmp_usize_max_value();
  if (a != 0 && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}

static int cmp_store_history_push(cmp_u8 *buffer, cmp_usize capacity,
                                 cmp_usize state_size, cmp_usize *io_count,
                                 const void *state) {
  cmp_usize offset;
  cmp_usize copy_size;

  if (capacity == 0) {
    return CMP_OK;
  }
  if (buffer == NULL || io_count == NULL || state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (*io_count < capacity) {
    offset = (*io_count) * state_size;
    memcpy(buffer + offset, state, (size_t)state_size);
    *io_count += 1;
    return CMP_OK;
  }

  if (capacity > 1) {
    copy_size = (capacity - 1) * state_size;
    memmove(buffer, buffer + state_size, (size_t)copy_size);
  }

  offset = (capacity - 1) * state_size;
  memcpy(buffer + offset, state, (size_t)state_size);
  *io_count = capacity;
  return CMP_OK;
}

static int cmp_store_history_pop(const cmp_u8 *buffer, cmp_usize capacity,
                                cmp_usize state_size, cmp_usize *io_count,
                                void *out_state) {
  cmp_usize offset;

  if (buffer == NULL || io_count == NULL || out_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (capacity == 0 || *io_count == 0) {
    return CMP_ERR_NOT_FOUND;
  }

  offset = (*io_count - 1) * state_size;
  memcpy(out_state, buffer + offset, (size_t)state_size);
  *io_count -= 1;
  return CMP_OK;
}

static int cmp_store_copy_history(const cmp_u8 *buffer, cmp_usize capacity,
                                 cmp_usize state_size, cmp_usize count,
                                 cmp_usize index_from_latest, void *out_state) {
  cmp_usize slot_index;
  cmp_usize offset;

  if (buffer == NULL || out_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (capacity == 0 || count == 0) {
    return CMP_ERR_NOT_FOUND;
  }
  if (index_from_latest >= count) {
    return CMP_ERR_NOT_FOUND;
  }

  slot_index = count - 1 - index_from_latest;
  offset = slot_index * state_size;
  memcpy(out_state, buffer + offset, (size_t)state_size);
  return CMP_OK;
}

int CMP_CALL cmp_store_init(CMPStore *store, const CMPStoreConfig *config,
                          const void *initial_state) {
  CMPAllocator allocator;
  cmp_usize history_bytes;
  int rc;

  if (store == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (config->state_size == 0 || config->reducer == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(store, 0, sizeof(*store));
  store->allocator = allocator;
  store->reducer = config->reducer;
  store->reducer_ctx = config->reducer_ctx;
  store->state_size = config->state_size;
  store->undo_capacity = config->history_capacity;
  store->redo_capacity = config->history_capacity;

  history_bytes = 0;
  if (store->undo_capacity > 0) {
    rc = cmp_store_mul_overflow(store->state_size, store->undo_capacity,
                               &history_bytes);
    if (rc != CMP_OK) {
      memset(store, 0, sizeof(*store));
      return rc;
    }
  }

  rc = store->allocator.alloc(store->allocator.ctx, store->state_size,
                              (void **)&store->state);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = store->allocator.alloc(store->allocator.ctx, store->state_size,
                              (void **)&store->scratch);
  if (rc != CMP_OK) {
    store->allocator.free(store->allocator.ctx, store->state);
    memset(store, 0, sizeof(*store));
    return rc;
  }

  if (store->undo_capacity > 0) {
    rc = store->allocator.alloc(store->allocator.ctx, history_bytes,
                                (void **)&store->undo_buffer);
    if (rc != CMP_OK) {
      store->allocator.free(store->allocator.ctx, store->scratch);
      store->allocator.free(store->allocator.ctx, store->state);
      memset(store, 0, sizeof(*store));
      return rc;
    }

    rc = store->allocator.alloc(store->allocator.ctx, history_bytes,
                                (void **)&store->redo_buffer);
    if (rc != CMP_OK) {
      store->allocator.free(store->allocator.ctx, store->undo_buffer);
      store->allocator.free(store->allocator.ctx, store->scratch);
      store->allocator.free(store->allocator.ctx, store->state);
      memset(store, 0, sizeof(*store));
      return rc;
    }
  }

  if (initial_state != NULL) {
    memcpy(store->state, initial_state, (size_t)store->state_size);
  } else {
    memset(store->state, 0, (size_t)store->state_size);
  }

  store->undo_count = 0;
  store->redo_count = 0;
  return CMP_OK;
}

int CMP_CALL cmp_store_shutdown(CMPStore *store) {
  if (store == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }

  if (store->undo_buffer != NULL) {
    store->allocator.free(store->allocator.ctx, store->undo_buffer);
    store->undo_buffer = NULL;
  }

  if (store->redo_buffer != NULL) {
    store->allocator.free(store->allocator.ctx, store->redo_buffer);
    store->redo_buffer = NULL;
  }

  if (store->scratch != NULL) {
    store->allocator.free(store->allocator.ctx, store->scratch);
    store->scratch = NULL;
  }

  store->allocator.free(store->allocator.ctx, store->state);
  store->state = NULL;

  store->state_size = 0;
  store->undo_capacity = 0;
  store->redo_capacity = 0;
  store->undo_count = 0;
  store->redo_count = 0;
  store->reducer = NULL;
  store->reducer_ctx = NULL;
  store->allocator.ctx = NULL;
  store->allocator.alloc = NULL;
  store->allocator.realloc = NULL;
  store->allocator.free = NULL;
  return CMP_OK;
}

int CMP_CALL cmp_store_dispatch(CMPStore *store, const CMPAction *action) {
  int rc;

  if (store == NULL || action == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL || store->scratch == NULL ||
      store->reducer == NULL) {
    return CMP_ERR_STATE;
  }
  if (action->size != 0 && action->data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = store->reducer(store->reducer_ctx, action, store->state, store->scratch);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_store_history_push(store->undo_buffer, store->undo_capacity,
                             store->state_size, &store->undo_count,
                             store->state);
  if (rc != CMP_OK) {
    return rc;
  }

  memcpy(store->state, store->scratch, (size_t)store->state_size);
  store->redo_count = 0;
  return CMP_OK;
}

int CMP_CALL cmp_store_get_state_ptr(const CMPStore *store, const void **out_state,
                                   cmp_usize *out_size) {
  if (store == NULL || out_state == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }

  *out_state = store->state;
  *out_size = store->state_size;
  return CMP_OK;
}

int CMP_CALL cmp_store_get_state(const CMPStore *store, void *out_state,
                               cmp_usize state_size) {
  if (store == NULL || out_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }
  if (state_size != store->state_size) {
    return CMP_ERR_RANGE;
  }

  memcpy(out_state, store->state, (size_t)state_size);
  return CMP_OK;
}

int CMP_CALL cmp_store_can_undo(const CMPStore *store, CMPBool *out_can_undo) {
  if (store == NULL || out_can_undo == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }

  *out_can_undo = (store->undo_count > 0) ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_store_can_redo(const CMPStore *store, CMPBool *out_can_redo) {
  if (store == NULL || out_can_redo == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }

  *out_can_redo = (store->redo_count > 0) ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_store_undo(CMPStore *store) {
  int rc;

  if (store == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }
  if (store->undo_count == 0) {
    return CMP_ERR_NOT_FOUND;
  }

  rc = cmp_store_history_push(store->redo_buffer, store->redo_capacity,
                             store->state_size, &store->redo_count,
                             store->state);
  if (rc != CMP_OK) {
    return rc;
  }

  rc =
      cmp_store_history_pop(store->undo_buffer, store->undo_capacity,
                           store->state_size, &store->undo_count, store->state);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_store_redo(CMPStore *store) {
  int rc;

  if (store == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }
  if (store->redo_count == 0) {
    return CMP_ERR_NOT_FOUND;
  }

  rc = cmp_store_history_push(store->undo_buffer, store->undo_capacity,
                             store->state_size, &store->undo_count,
                             store->state);
  if (rc != CMP_OK) {
    return rc;
  }

  rc =
      cmp_store_history_pop(store->redo_buffer, store->redo_capacity,
                           store->state_size, &store->redo_count, store->state);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_store_clear_history(CMPStore *store) {
  if (store == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }

  store->undo_count = 0;
  store->redo_count = 0;
  return CMP_OK;
}

int CMP_CALL cmp_store_get_undo_count(const CMPStore *store, cmp_usize *out_count) {
  if (store == NULL || out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }

  *out_count = store->undo_count;
  return CMP_OK;
}

int CMP_CALL cmp_store_get_redo_count(const CMPStore *store, cmp_usize *out_count) {
  if (store == NULL || out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }

  *out_count = store->redo_count;
  return CMP_OK;
}

int CMP_CALL cmp_store_copy_undo_state(const CMPStore *store,
                                     cmp_usize index_from_latest,
                                     void *out_state, cmp_usize state_size) {
  if (store == NULL || out_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }
  if (state_size != store->state_size) {
    return CMP_ERR_RANGE;
  }

  return cmp_store_copy_history(store->undo_buffer, store->undo_capacity,
                               store->state_size, store->undo_count,
                               index_from_latest, out_state);
}

int CMP_CALL cmp_store_copy_redo_state(const CMPStore *store,
                                     cmp_usize index_from_latest,
                                     void *out_state, cmp_usize state_size) {
  if (store == NULL || out_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (store->state == NULL) {
    return CMP_ERR_STATE;
  }
  if (state_size != store->state_size) {
    return CMP_ERR_RANGE;
  }

  return cmp_store_copy_history(store->redo_buffer, store->redo_capacity,
                               store->state_size, store->redo_count,
                               index_from_latest, out_state);
}

#ifdef CMP_TESTING
int CMP_CALL cmp_store_test_mul_overflow(cmp_usize a, cmp_usize b,
                                       cmp_usize *out_value) {
  return cmp_store_mul_overflow(a, b, out_value);
}

int CMP_CALL cmp_store_test_history_push(cmp_u8 *buffer, cmp_usize capacity,
                                       cmp_usize state_size, cmp_usize *io_count,
                                       const void *state) {
  return cmp_store_history_push(buffer, capacity, state_size, io_count, state);
}

int CMP_CALL cmp_store_test_history_pop(const cmp_u8 *buffer, cmp_usize capacity,
                                      cmp_usize state_size, cmp_usize *io_count,
                                      void *out_state) {
  return cmp_store_history_pop(buffer, capacity, state_size, io_count,
                              out_state);
}

int CMP_CALL cmp_store_test_copy_history(const cmp_u8 *buffer, cmp_usize capacity,
                                       cmp_usize state_size, cmp_usize count,
                                       cmp_usize index_from_latest,
                                       void *out_state) {
  return cmp_store_copy_history(buffer, capacity, state_size, count,
                               index_from_latest, out_state);
}
#endif
