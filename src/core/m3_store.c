#include "m3/m3_store.h"

#include <string.h>

static m3_usize m3_usize_max_value(void)
{
    return (m3_usize)~(m3_usize)0;
}

static int m3_store_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    m3_usize max_value;

    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    max_value = m3_usize_max_value();
    if (a != 0 && b > max_value / a) {
        return M3_ERR_OVERFLOW;
    }

    *out_value = a * b;
    return M3_OK;
}

static int m3_store_history_push(m3_u8 *buffer, m3_usize capacity, m3_usize state_size, m3_usize *io_count, const void *state)
{
    m3_usize offset;
    m3_usize copy_size;

    if (capacity == 0) {
        return M3_OK;
    }
    if (buffer == NULL || io_count == NULL || state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (*io_count < capacity) {
        offset = (*io_count) * state_size;
        memcpy(buffer + offset, state, (size_t)state_size);
        *io_count += 1;
        return M3_OK;
    }

    if (capacity > 1) {
        copy_size = (capacity - 1) * state_size;
        memmove(buffer, buffer + state_size, (size_t)copy_size);
    }

    offset = (capacity - 1) * state_size;
    memcpy(buffer + offset, state, (size_t)state_size);
    *io_count = capacity;
    return M3_OK;
}

static int m3_store_history_pop(const m3_u8 *buffer, m3_usize capacity, m3_usize state_size, m3_usize *io_count, void *out_state)
{
    m3_usize offset;

    if (buffer == NULL || io_count == NULL || out_state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (capacity == 0 || *io_count == 0) {
        return M3_ERR_NOT_FOUND;
    }

    offset = (*io_count - 1) * state_size;
    memcpy(out_state, buffer + offset, (size_t)state_size);
    *io_count -= 1;
    return M3_OK;
}

static int m3_store_copy_history(const m3_u8 *buffer, m3_usize capacity, m3_usize state_size, m3_usize count, m3_usize index_from_latest, void *out_state)
{
    m3_usize slot_index;
    m3_usize offset;

    if (buffer == NULL || out_state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (capacity == 0 || count == 0) {
        return M3_ERR_NOT_FOUND;
    }
    if (index_from_latest >= count) {
        return M3_ERR_NOT_FOUND;
    }

    slot_index = count - 1 - index_from_latest;
    offset = slot_index * state_size;
    memcpy(out_state, buffer + offset, (size_t)state_size);
    return M3_OK;
}

int M3_CALL m3_store_init(M3Store *store, const M3StoreConfig *config, const void *initial_state)
{
    M3Allocator allocator;
    m3_usize history_bytes;
    int rc;

    if (store == NULL || config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (config->state_size == 0 || config->reducer == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (config->allocator == NULL) {
        rc = m3_get_default_allocator(&allocator);
        if (rc != M3_OK) {
            return rc;
        }
    } else {
        allocator = *config->allocator;
    }

    if (allocator.alloc == NULL || allocator.realloc == NULL || allocator.free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
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
        rc = m3_store_mul_overflow(store->state_size, store->undo_capacity, &history_bytes);
        if (rc != M3_OK) {
            memset(store, 0, sizeof(*store));
            return rc;
        }
    }

    rc = store->allocator.alloc(store->allocator.ctx, store->state_size, (void **)&store->state);
    if (rc != M3_OK) {
        return rc;
    }

    rc = store->allocator.alloc(store->allocator.ctx, store->state_size, (void **)&store->scratch);
    if (rc != M3_OK) {
        store->allocator.free(store->allocator.ctx, store->state);
        memset(store, 0, sizeof(*store));
        return rc;
    }

    if (store->undo_capacity > 0) {
        rc = store->allocator.alloc(store->allocator.ctx, history_bytes, (void **)&store->undo_buffer);
        if (rc != M3_OK) {
            store->allocator.free(store->allocator.ctx, store->scratch);
            store->allocator.free(store->allocator.ctx, store->state);
            memset(store, 0, sizeof(*store));
            return rc;
        }

        rc = store->allocator.alloc(store->allocator.ctx, history_bytes, (void **)&store->redo_buffer);
        if (rc != M3_OK) {
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
    return M3_OK;
}

int M3_CALL m3_store_shutdown(M3Store *store)
{
    if (store == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (store->state == NULL) {
        return M3_ERR_STATE;
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
    return M3_OK;
}

int M3_CALL m3_store_dispatch(M3Store *store, const M3Action *action)
{
    int rc;

    if (store == NULL || action == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL || store->scratch == NULL || store->reducer == NULL) {
        return M3_ERR_STATE;
    }
    if (action->size != 0 && action->data == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = store->reducer(store->reducer_ctx, action, store->state, store->scratch);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_store_history_push(store->undo_buffer, store->undo_capacity, store->state_size, &store->undo_count, store->state);
    if (rc != M3_OK) {
        return rc;
    }

    memcpy(store->state, store->scratch, (size_t)store->state_size);
    store->redo_count = 0;
    return M3_OK;
}

int M3_CALL m3_store_get_state_ptr(const M3Store *store, const void **out_state, m3_usize *out_size)
{
    if (store == NULL || out_state == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }

    *out_state = store->state;
    *out_size = store->state_size;
    return M3_OK;
}

int M3_CALL m3_store_get_state(const M3Store *store, void *out_state, m3_usize state_size)
{
    if (store == NULL || out_state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }
    if (state_size != store->state_size) {
        return M3_ERR_RANGE;
    }

    memcpy(out_state, store->state, (size_t)state_size);
    return M3_OK;
}

int M3_CALL m3_store_can_undo(const M3Store *store, M3Bool *out_can_undo)
{
    if (store == NULL || out_can_undo == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }

    *out_can_undo = (store->undo_count > 0) ? M3_TRUE : M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_store_can_redo(const M3Store *store, M3Bool *out_can_redo)
{
    if (store == NULL || out_can_redo == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }

    *out_can_redo = (store->redo_count > 0) ? M3_TRUE : M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_store_undo(M3Store *store)
{
    int rc;

    if (store == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }
    if (store->undo_count == 0) {
        return M3_ERR_NOT_FOUND;
    }

    rc = m3_store_history_push(store->redo_buffer, store->redo_capacity, store->state_size, &store->redo_count, store->state);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_store_history_pop(store->undo_buffer, store->undo_capacity, store->state_size, &store->undo_count, store->state);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

int M3_CALL m3_store_redo(M3Store *store)
{
    int rc;

    if (store == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }
    if (store->redo_count == 0) {
        return M3_ERR_NOT_FOUND;
    }

    rc = m3_store_history_push(store->undo_buffer, store->undo_capacity, store->state_size, &store->undo_count, store->state);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_store_history_pop(store->redo_buffer, store->redo_capacity, store->state_size, &store->redo_count, store->state);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

int M3_CALL m3_store_clear_history(M3Store *store)
{
    if (store == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }

    store->undo_count = 0;
    store->redo_count = 0;
    return M3_OK;
}

int M3_CALL m3_store_get_undo_count(const M3Store *store, m3_usize *out_count)
{
    if (store == NULL || out_count == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }

    *out_count = store->undo_count;
    return M3_OK;
}

int M3_CALL m3_store_get_redo_count(const M3Store *store, m3_usize *out_count)
{
    if (store == NULL || out_count == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }

    *out_count = store->redo_count;
    return M3_OK;
}

int M3_CALL m3_store_copy_undo_state(const M3Store *store, m3_usize index_from_latest, void *out_state, m3_usize state_size)
{
    if (store == NULL || out_state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }
    if (state_size != store->state_size) {
        return M3_ERR_RANGE;
    }

    return m3_store_copy_history(store->undo_buffer, store->undo_capacity, store->state_size, store->undo_count, index_from_latest, out_state);
}

int M3_CALL m3_store_copy_redo_state(const M3Store *store, m3_usize index_from_latest, void *out_state, m3_usize state_size)
{
    if (store == NULL || out_state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (store->state == NULL) {
        return M3_ERR_STATE;
    }
    if (state_size != store->state_size) {
        return M3_ERR_RANGE;
    }

    return m3_store_copy_history(store->redo_buffer, store->redo_capacity, store->state_size, store->redo_count, index_from_latest, out_state);
}
