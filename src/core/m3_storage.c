#include "m3/m3_storage.h"

#include <string.h>

#define M3_STORAGE_MAGIC_0 'M'
#define M3_STORAGE_MAGIC_1 '3'
#define M3_STORAGE_MAGIC_2 'S'
#define M3_STORAGE_MAGIC_3 'T'
#define M3_STORAGE_VERSION 1u
#define M3_STORAGE_HEADER_SIZE 12u
#define M3_STORAGE_U32_MAX ((m3_u32)~(m3_u32)0)

#define M3_STORAGE_RETURN_IF_ERROR(rc) do { if ((rc) != M3_OK) { return (rc); } } while (0)

static int m3_storage_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    m3_usize max_value;

    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    max_value = (m3_usize)~(m3_usize)0;
    if (a != 0 && b > max_value / a) {
        return M3_ERR_OVERFLOW;
    }

    *out_value = a * b;
    return M3_OK;
}

static int m3_storage_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    m3_usize max_value;

    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    max_value = (m3_usize)~(m3_usize)0;
    if (b > max_value - a) {
        return M3_ERR_OVERFLOW;
    }

    *out_value = a + b;
    return M3_OK;
}

static int m3_storage_validate_bool(M3Bool value)
{
    if (value != M3_FALSE && value != M3_TRUE) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_OK;
}

static int m3_storage_validate_key(const char *key, m3_usize key_len)
{
    if (key == NULL || key_len == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_OK;
}

static int m3_storage_u32_from_usize(m3_usize value, m3_u32 *out_value)
{
    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (value > (m3_usize)M3_STORAGE_U32_MAX) {
        return M3_ERR_RANGE;
    }
    *out_value = (m3_u32)value;
    return M3_OK;
}

static int m3_storage_write_u32_le(m3_u8 *dst, m3_u32 value)
{
    dst[0] = (m3_u8)(value & 0xFFu);
    dst[1] = (m3_u8)((value >> 8) & 0xFFu);
    dst[2] = (m3_u8)((value >> 16) & 0xFFu);
    dst[3] = (m3_u8)((value >> 24) & 0xFFu);
    return M3_OK;
}

static int m3_storage_read_u32_le(const m3_u8 *src, m3_u32 *out_value)
{
    *out_value = (m3_u32)src[0]
        | ((m3_u32)src[1] << 8)
        | ((m3_u32)src[2] << 16)
        | ((m3_u32)src[3] << 24);
    return M3_OK;
}

static int m3_storage_find_entry(const M3Storage *storage, const char *key, m3_usize key_len, m3_usize *out_index, M3Bool *out_found)
{
    m3_usize i;

    *out_index = 0;
    *out_found = M3_FALSE;

    for (i = 0; i < storage->entry_count; ++i) {
        const M3StorageEntry *entry = &storage->entries[i];

        if (entry->key_len == key_len && entry->key != NULL && memcmp(entry->key, key, (size_t)key_len) == 0) {
            *out_index = i;
            *out_found = M3_TRUE;
            return M3_OK;
        }
    }

    return M3_OK;
}

static int m3_storage_entry_release(M3Storage *storage, M3StorageEntry *entry)
{
    int rc;
    int free_rc;

    rc = M3_OK;

    if (entry->key != NULL) {
        free_rc = storage->allocator.free(storage->allocator.ctx, entry->key);
        if (free_rc != M3_OK && rc == M3_OK) {
            rc = free_rc;
        }
        entry->key = NULL;
    }

    if (entry->value != NULL) {
        free_rc = storage->allocator.free(storage->allocator.ctx, entry->value);
        if (free_rc != M3_OK && rc == M3_OK) {
            rc = free_rc;
        }
        entry->value = NULL;
    }

    entry->key_len = 0;
    entry->value_len = 0;
    return rc;
}

static int m3_storage_grow(M3Storage *storage, m3_usize min_capacity)
{
    m3_usize new_capacity;
    m3_usize alloc_size;
    m3_usize clear_count;
    void *new_entries;
    int rc;

    if (storage->allocator.realloc == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    new_capacity = storage->entry_capacity;
    while (new_capacity < min_capacity) {
        rc = m3_storage_mul_overflow(new_capacity, (m3_usize)2, &new_capacity);
        if (rc != M3_OK) {
            return rc;
        }
    }

    rc = m3_storage_mul_overflow(new_capacity, (m3_usize)sizeof(M3StorageEntry), &alloc_size);
    if (rc != M3_OK) {
        return rc;
    }

    rc = storage->allocator.realloc(storage->allocator.ctx, storage->entries, alloc_size, &new_entries);
    if (rc != M3_OK) {
        return rc;
    }

    clear_count = new_capacity - storage->entry_capacity;
    if (clear_count > 0) {
        memset((M3StorageEntry *)new_entries + storage->entry_capacity, 0, (size_t)(clear_count * sizeof(M3StorageEntry)));
    }

    storage->entries = (M3StorageEntry *)new_entries;
    storage->entry_capacity = new_capacity;
    return M3_OK;
}

int M3_CALL m3_storage_config_init(M3StorageConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    config->allocator = NULL;
    config->entry_capacity = (m3_usize)M3_STORAGE_DEFAULT_CAPACITY;
    return M3_OK;
}

int M3_CALL m3_storage_init(M3Storage *storage, const M3StorageConfig *config)
{
    M3Allocator allocator;
    M3StorageEntry *entries;
    m3_usize alloc_size;
    int rc;

    if (storage == NULL || config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries != NULL) {
        return M3_ERR_STATE;
    }
    if (config->entry_capacity == 0) {
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

    rc = m3_storage_mul_overflow(config->entry_capacity, (m3_usize)sizeof(M3StorageEntry), &alloc_size);
    if (rc != M3_OK) {
        return rc;
    }

    rc = allocator.alloc(allocator.ctx, alloc_size, (void **)&entries);
    if (rc != M3_OK) {
        return rc;
    }

    memset(entries, 0, (size_t)alloc_size);

    storage->allocator = allocator;
    storage->entries = entries;
    storage->entry_capacity = config->entry_capacity;
    storage->entry_count = 0;
    return M3_OK;
}

int M3_CALL m3_storage_shutdown(M3Storage *storage)
{
    m3_usize i;
    int rc;
    int free_rc;

    if (storage == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries == NULL) {
        return M3_ERR_STATE;
    }
    if (storage->allocator.free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = M3_OK;

    for (i = 0; i < storage->entry_count; ++i) {
        free_rc = m3_storage_entry_release(storage, &storage->entries[i]);
        if (free_rc != M3_OK && rc == M3_OK) {
            rc = free_rc;
        }
    }

    free_rc = storage->allocator.free(storage->allocator.ctx, storage->entries);
    if (free_rc != M3_OK && rc == M3_OK) {
        rc = free_rc;
    }

    storage->entries = NULL;
    storage->entry_capacity = 0;
    storage->entry_count = 0;
    storage->allocator.ctx = NULL;
    storage->allocator.alloc = NULL;
    storage->allocator.realloc = NULL;
    storage->allocator.free = NULL;
    return rc;
}

int M3_CALL m3_storage_put(M3Storage *storage, const char *utf8_key, m3_usize key_len, const void *value, m3_usize value_len,
    M3Bool overwrite)
{
    M3StorageEntry *entry;
    m3_usize index;
    M3Bool found;
    m3_usize key_bytes;
    void *key_copy;
    void *value_copy;
    int rc;
    int free_rc;

    if (storage == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_storage_validate_bool(overwrite);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    rc = m3_storage_validate_key(utf8_key, key_len);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    if (value == NULL && value_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_storage_find_entry(storage, utf8_key, key_len, &index, &found);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    if (found == M3_TRUE) {
        if (overwrite == M3_FALSE) {
            return M3_ERR_BUSY;
        }

        entry = &storage->entries[index];
        if (value_len == 0) {
            if (entry->value != NULL) {
                rc = storage->allocator.free(storage->allocator.ctx, entry->value);
                if (rc != M3_OK) {
                    return rc;
                }
            }
            entry->value = NULL;
            entry->value_len = 0;
            return M3_OK;
        }

        rc = storage->allocator.alloc(storage->allocator.ctx, value_len, &value_copy);
        if (rc != M3_OK) {
            return rc;
        }
        memcpy(value_copy, value, (size_t)value_len);

        if (entry->value != NULL) {
            rc = storage->allocator.free(storage->allocator.ctx, entry->value);
            if (rc != M3_OK) {
                storage->allocator.free(storage->allocator.ctx, value_copy);
                return rc;
            }
        }

        entry->value = value_copy;
        entry->value_len = value_len;
        return M3_OK;
    }

    if (storage->entry_count >= storage->entry_capacity) {
        rc = m3_storage_grow(storage, storage->entry_count + 1u);
        M3_STORAGE_RETURN_IF_ERROR(rc);
    }

    rc = m3_storage_add_overflow(key_len, (m3_usize)1, &key_bytes);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    rc = storage->allocator.alloc(storage->allocator.ctx, key_bytes, &key_copy);
    if (rc != M3_OK) {
        return rc;
    }

    memcpy(key_copy, utf8_key, (size_t)key_len);
    ((char *)key_copy)[key_len] = '\0';

    value_copy = NULL;
    if (value_len > 0) {
        rc = storage->allocator.alloc(storage->allocator.ctx, value_len, &value_copy);
        if (rc != M3_OK) {
            free_rc = storage->allocator.free(storage->allocator.ctx, key_copy);
            M3_UNUSED(free_rc);
            return rc;
        }
        memcpy(value_copy, value, (size_t)value_len);
    }

    entry = &storage->entries[storage->entry_count];
    entry->key = (char *)key_copy;
    entry->key_len = key_len;
    entry->value = value_copy;
    entry->value_len = value_len;
    storage->entry_count += 1;
    return M3_OK;
}

int M3_CALL m3_storage_get(const M3Storage *storage, const char *utf8_key, m3_usize key_len, void *out_value,
    m3_usize value_capacity, m3_usize *out_value_size)
{
    m3_usize index;
    M3Bool found;
    int rc;

    if (storage == NULL || out_value_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries == NULL) {
        return M3_ERR_STATE;
    }
    if (out_value == NULL && value_capacity != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_storage_validate_key(utf8_key, key_len);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    rc = m3_storage_find_entry(storage, utf8_key, key_len, &index, &found);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    if (found == M3_FALSE) {
        *out_value_size = 0;
        return M3_ERR_NOT_FOUND;
    }

    *out_value_size = storage->entries[index].value_len;
    if (value_capacity < storage->entries[index].value_len) {
        return M3_ERR_RANGE;
    }

    if (storage->entries[index].value_len > 0) {
        memcpy(out_value, storage->entries[index].value, (size_t)storage->entries[index].value_len);
    }

    return M3_OK;
}

int M3_CALL m3_storage_contains(const M3Storage *storage, const char *utf8_key, m3_usize key_len, M3Bool *out_exists)
{
    m3_usize index;
    M3Bool found;
    int rc;

    if (storage == NULL || out_exists == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_storage_validate_key(utf8_key, key_len);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    rc = m3_storage_find_entry(storage, utf8_key, key_len, &index, &found);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    *out_exists = found;
    return M3_OK;
}

int M3_CALL m3_storage_remove(M3Storage *storage, const char *utf8_key, m3_usize key_len)
{
    m3_usize index;
    m3_usize tail_count;
    M3Bool found;
    int rc;

    if (storage == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_storage_validate_key(utf8_key, key_len);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    rc = m3_storage_find_entry(storage, utf8_key, key_len, &index, &found);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    if (found == M3_FALSE) {
        return M3_ERR_NOT_FOUND;
    }

    rc = m3_storage_entry_release(storage, &storage->entries[index]);

    tail_count = storage->entry_count - index - 1u;
    if (tail_count > 0) {
        memmove(&storage->entries[index], &storage->entries[index + 1u], (size_t)(tail_count * sizeof(M3StorageEntry)));
    }

    storage->entry_count -= 1u;
    memset(&storage->entries[storage->entry_count], 0, sizeof(M3StorageEntry));
    return rc;
}

int M3_CALL m3_storage_clear(M3Storage *storage)
{
    m3_usize i;
    int rc;
    int free_rc;

    if (storage == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries == NULL) {
        return M3_ERR_STATE;
    }

    rc = M3_OK;
    for (i = 0; i < storage->entry_count; ++i) {
        free_rc = m3_storage_entry_release(storage, &storage->entries[i]);
        if (free_rc != M3_OK && rc == M3_OK) {
            rc = free_rc;
        }
    }

    memset(storage->entries, 0, (size_t)(storage->entry_capacity * sizeof(M3StorageEntry)));
    storage->entry_count = 0;
    return rc;
}

int M3_CALL m3_storage_count(const M3Storage *storage, m3_usize *out_count)
{
    if (storage == NULL || out_count == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries == NULL) {
        return M3_ERR_STATE;
    }

    *out_count = storage->entry_count;
    return M3_OK;
}

int M3_CALL m3_storage_save(const M3Storage *storage, M3IO *io, const char *utf8_path)
{
    m3_u8 *buffer;
    m3_u32 entry_count_u32;
    m3_u32 key_len_u32;
    m3_u32 value_len_u32;
    m3_usize total_size;
    m3_usize offset;
    int rc;
    int free_rc;
    m3_usize i;

    if (storage == NULL || io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries == NULL) {
        return M3_ERR_STATE;
    }
    if (io->vtable == NULL || io->vtable->write_file == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->allocator.alloc == NULL || storage->allocator.free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_storage_u32_from_usize(storage->entry_count, &entry_count_u32);
    M3_STORAGE_RETURN_IF_ERROR(rc);

    total_size = (m3_usize)M3_STORAGE_HEADER_SIZE;
    for (i = 0; i < storage->entry_count; ++i) {
        const M3StorageEntry *entry = &storage->entries[i];

        if (entry->key == NULL || (entry->value == NULL && entry->value_len != 0)) {
            return M3_ERR_STATE;
        }

        rc = m3_storage_u32_from_usize(entry->key_len, &key_len_u32);
        M3_STORAGE_RETURN_IF_ERROR(rc);
        rc = m3_storage_u32_from_usize(entry->value_len, &value_len_u32);
        M3_STORAGE_RETURN_IF_ERROR(rc);

        rc = m3_storage_add_overflow(total_size, (m3_usize)8, &total_size);
        M3_STORAGE_RETURN_IF_ERROR(rc);
        rc = m3_storage_add_overflow(total_size, entry->key_len, &total_size);
        M3_STORAGE_RETURN_IF_ERROR(rc);
        rc = m3_storage_add_overflow(total_size, entry->value_len, &total_size);
        M3_STORAGE_RETURN_IF_ERROR(rc);
    }

    rc = storage->allocator.alloc(storage->allocator.ctx, total_size, (void **)&buffer);
    if (rc != M3_OK) {
        return rc;
    }

    buffer[0] = (m3_u8)M3_STORAGE_MAGIC_0;
    buffer[1] = (m3_u8)M3_STORAGE_MAGIC_1;
    buffer[2] = (m3_u8)M3_STORAGE_MAGIC_2;
    buffer[3] = (m3_u8)M3_STORAGE_MAGIC_3;

    m3_storage_write_u32_le(buffer + 4, (m3_u32)M3_STORAGE_VERSION);
    m3_storage_write_u32_le(buffer + 8, entry_count_u32);

    offset = (m3_usize)M3_STORAGE_HEADER_SIZE;
    for (i = 0; i < storage->entry_count; ++i) {
        const M3StorageEntry *entry = &storage->entries[i];

        rc = m3_storage_u32_from_usize(entry->key_len, &key_len_u32);
        if (rc != M3_OK) {
            free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
            M3_UNUSED(free_rc);
            return rc;
        }

        rc = m3_storage_u32_from_usize(entry->value_len, &value_len_u32);
        if (rc != M3_OK) {
            free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
            M3_UNUSED(free_rc);
            return rc;
        }

        m3_storage_write_u32_le(buffer + offset, key_len_u32);
        m3_storage_write_u32_le(buffer + offset + 4u, value_len_u32);

        offset += 8u;
        if (entry->key_len > 0) {
            memcpy(buffer + offset, entry->key, (size_t)entry->key_len);
            offset += entry->key_len;
        }

        if (entry->value_len > 0) {
            memcpy(buffer + offset, entry->value, (size_t)entry->value_len);
            offset += entry->value_len;
        }
    }

    rc = io->vtable->write_file(io->ctx, utf8_path, buffer, total_size, M3_TRUE);
    free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
    if (free_rc != M3_OK && rc == M3_OK) {
        rc = free_rc;
    }
    return rc;
}

int M3_CALL m3_storage_load(M3Storage *storage, M3IO *io, const char *utf8_path)
{
    M3StorageConfig config;
    M3Storage temp_storage;
    M3Storage swap_storage;
    m3_u8 *buffer;
    m3_u32 version;
    m3_u32 entry_count_u32;
    m3_u32 key_len_u32;
    m3_u32 value_len_u32;
    m3_usize buffer_size;
    m3_usize offset;
    m3_usize i;
    int rc;
    int free_rc;

    if (storage == NULL || io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->entries == NULL) {
        return M3_ERR_STATE;
    }
    if (io->vtable == NULL || io->vtable->read_file_alloc == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (storage->allocator.alloc == NULL || storage->allocator.free == NULL || storage->allocator.realloc == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    buffer = NULL;
    buffer_size = 0;
    rc = io->vtable->read_file_alloc(io->ctx, utf8_path, &storage->allocator, (void **)&buffer, &buffer_size);
    if (rc != M3_OK) {
        return rc;
    }
    if (buffer == NULL || buffer_size < (m3_usize)M3_STORAGE_HEADER_SIZE) {
        if (buffer != NULL) {
            free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
            M3_UNUSED(free_rc);
        }
        return M3_ERR_CORRUPT;
    }

    if (buffer[0] != (m3_u8)M3_STORAGE_MAGIC_0
        || buffer[1] != (m3_u8)M3_STORAGE_MAGIC_1
        || buffer[2] != (m3_u8)M3_STORAGE_MAGIC_2
        || buffer[3] != (m3_u8)M3_STORAGE_MAGIC_3) {
        free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
        M3_UNUSED(free_rc);
        return M3_ERR_CORRUPT;
    }

    m3_storage_read_u32_le(buffer + 4, &version);

    if (version != (m3_u32)M3_STORAGE_VERSION) {
        free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
        M3_UNUSED(free_rc);
        return M3_ERR_UNSUPPORTED;
    }

    m3_storage_read_u32_le(buffer + 8, &entry_count_u32);

    config.allocator = &storage->allocator;
    config.entry_capacity = entry_count_u32 == 0u ? (m3_usize)M3_STORAGE_DEFAULT_CAPACITY : (m3_usize)entry_count_u32;
    memset(&temp_storage, 0, sizeof(temp_storage));

    rc = m3_storage_init(&temp_storage, &config);
    if (rc != M3_OK) {
        free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
        M3_UNUSED(free_rc);
        return rc;
    }

    offset = (m3_usize)M3_STORAGE_HEADER_SIZE;
    for (i = 0; i < (m3_usize)entry_count_u32; ++i) {
        if (offset + 8u > buffer_size) {
            rc = M3_ERR_CORRUPT;
            break;
        }

        m3_storage_read_u32_le(buffer + offset, &key_len_u32);
        m3_storage_read_u32_le(buffer + offset + 4u, &value_len_u32);
        offset += 8u;

        if (key_len_u32 == 0u) {
            rc = M3_ERR_CORRUPT;
            break;
        }
        if (offset + (m3_usize)key_len_u32 + (m3_usize)value_len_u32 > buffer_size) {
            rc = M3_ERR_CORRUPT;
            break;
        }

        rc = m3_storage_put(&temp_storage, (const char *)(buffer + offset), (m3_usize)key_len_u32,
            buffer + offset + key_len_u32, (m3_usize)value_len_u32, M3_FALSE);
        if (rc != M3_OK) {
            break;
        }

        offset += (m3_usize)key_len_u32 + (m3_usize)value_len_u32;
    }

    free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
    if (free_rc != M3_OK && rc == M3_OK) {
        rc = free_rc;
    }

    if (rc != M3_OK) {
        free_rc = m3_storage_shutdown(&temp_storage);
        M3_UNUSED(free_rc);
        return rc;
    }

    swap_storage = *storage;
    *storage = temp_storage;
    temp_storage = swap_storage;

    rc = m3_storage_shutdown(&temp_storage);
    return rc;
}

#ifdef M3_TESTING
int M3_CALL m3_storage_test_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    return m3_storage_mul_overflow(a, b, out_value);
}

int M3_CALL m3_storage_test_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    return m3_storage_add_overflow(a, b, out_value);
}

int M3_CALL m3_storage_test_u32_from_usize(m3_usize value, m3_u32 *out_value)
{
    return m3_storage_u32_from_usize(value, out_value);
}

int M3_CALL m3_storage_test_grow(M3Storage *storage, m3_usize min_capacity)
{
    return m3_storage_grow(storage, min_capacity);
}
#endif
