#include "cmpc/cmp_storage.h"

#include <string.h>

#define CMP_STORAGE_MAGIC_0 'M'
#define CMP_STORAGE_MAGIC_1 '3'
#define CMP_STORAGE_MAGIC_2 'S'
#define CMP_STORAGE_MAGIC_3 'T'
#define CMP_STORAGE_VERSION 1u
#define CMP_STORAGE_HEADER_SIZE 12u
#define CMP_STORAGE_U32_MAX ((cmp_u32) ~(cmp_u32)0)

#define CMP_STORAGE_RETURN_IF_ERROR(rc)                                        \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                      \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

static int cmp_storage_mul_overflow(cmp_usize a, cmp_usize b,
                                    cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = (cmp_usize) ~(cmp_usize)0;
  if (a != 0 && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}

static int cmp_storage_add_overflow(cmp_usize a, cmp_usize b,
                                    cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = (cmp_usize) ~(cmp_usize)0;
  if (b > max_value - a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return CMP_OK;
}

static int cmp_storage_validate_bool(CMPBool value) {
  if (value != CMP_FALSE && value != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int cmp_storage_validate_key(const char *key, cmp_usize key_len) {
  if (key == NULL || key_len == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int cmp_storage_u32_from_usize(cmp_usize value, cmp_u32 *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (value > (cmp_usize)CMP_STORAGE_U32_MAX) {
    return CMP_ERR_RANGE;
  }
  *out_value = (cmp_u32)value;
  return CMP_OK;
}

static int cmp_storage_write_u32_le(cmp_u8 *dst, cmp_u32 value) {
  dst[0] = (cmp_u8)(value & 0xFFu);
  dst[1] = (cmp_u8)((value >> 8) & 0xFFu);
  dst[2] = (cmp_u8)((value >> 16) & 0xFFu);
  dst[3] = (cmp_u8)((value >> 24) & 0xFFu);
  return CMP_OK;
}

static int cmp_storage_read_u32_le(const cmp_u8 *src, cmp_u32 *out_value) {
  *out_value = (cmp_u32)src[0] | ((cmp_u32)src[1] << 8) |
               ((cmp_u32)src[2] << 16) | ((cmp_u32)src[3] << 24);
  return CMP_OK;
}

static int cmp_storage_find_entry(const CMPStorage *storage, const char *key,
                                  cmp_usize key_len, cmp_usize *out_index,
                                  CMPBool *out_found) {
  cmp_usize i;

  *out_index = 0;
  *out_found = CMP_FALSE;

  for (i = 0; i < storage->entry_count; ++i) {
    const CMPStorageEntry *entry = &storage->entries[i];

    if (entry->key_len == key_len && entry->key != NULL &&
        memcmp(entry->key, key, (size_t)key_len) == 0) {
      *out_index = i;
      *out_found = CMP_TRUE;
      return CMP_OK;
    }
  }

  return CMP_OK;
}

static int cmp_storage_entry_release(CMPStorage *storage,
                                     CMPStorageEntry *entry) {
  int rc;
  int free_rc;

  rc = CMP_OK;

  if (entry->key != NULL) {
    free_rc = storage->allocator.free(storage->allocator.ctx, entry->key);
    if (free_rc != CMP_OK && rc == CMP_OK) {
      rc = free_rc;
    }
    entry->key = NULL;
  }

  if (entry->value != NULL) {
    free_rc = storage->allocator.free(storage->allocator.ctx, entry->value);
    if (free_rc != CMP_OK && rc == CMP_OK) {
      rc = free_rc;
    }
    entry->value = NULL;
  }

  entry->key_len = 0;
  entry->value_len = 0;
  return rc;
}

static int cmp_storage_grow(CMPStorage *storage, cmp_usize min_capacity) {
  cmp_usize new_capacity;
  cmp_usize alloc_size;
  cmp_usize clear_count;
  void *new_entries;
  int rc;

  if (storage->allocator.realloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  new_capacity = storage->entry_capacity;
  while (new_capacity < min_capacity) {
    rc = cmp_storage_mul_overflow(new_capacity, (cmp_usize)2, &new_capacity);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = cmp_storage_mul_overflow(
      new_capacity, (cmp_usize)sizeof(CMPStorageEntry), &alloc_size);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = storage->allocator.realloc(storage->allocator.ctx, storage->entries,
                                  alloc_size, &new_entries);
  if (rc != CMP_OK) {
    return rc;
  }

  clear_count = new_capacity - storage->entry_capacity;
  if (clear_count > 0) {
    memset((CMPStorageEntry *)new_entries + storage->entry_capacity, 0,
           (size_t)(clear_count * sizeof(CMPStorageEntry)));
  }

  storage->entries = (CMPStorageEntry *)new_entries;
  storage->entry_capacity = new_capacity;
  return CMP_OK;
}

int CMP_CALL cmp_storage_config_init(CMPStorageConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->entry_capacity = (cmp_usize)CMP_STORAGE_DEFAULT_CAPACITY;
  return CMP_OK;
}

int CMP_CALL cmp_storage_init(CMPStorage *storage,
                              const CMPStorageConfig *config) {
  CMPAllocator allocator;
  CMPStorageEntry *entries;
  cmp_usize alloc_size;
  int rc;

  if (storage == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries != NULL) {
    return CMP_ERR_STATE;
  }
  if (config->entry_capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (config->allocator != NULL) {
    allocator = *config->allocator;
  } else {
    rc = cmp_get_default_allocator(&allocator);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_storage_mul_overflow(
      config->entry_capacity, (cmp_usize)sizeof(CMPStorageEntry), &alloc_size);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = allocator.alloc(allocator.ctx, alloc_size, (void **)&entries);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(entries, 0, (size_t)alloc_size);

  storage->allocator = allocator;
  storage->entries = entries;
  storage->entry_capacity = config->entry_capacity;
  storage->entry_count = 0;
  return CMP_OK;
}

int CMP_CALL cmp_storage_shutdown(CMPStorage *storage) {
  cmp_usize i;
  int rc;
  int free_rc;

  if (storage == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries == NULL) {
    return CMP_ERR_STATE;
  }
  if (storage->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = CMP_OK;

  for (i = 0; i < storage->entry_count; ++i) {
    free_rc = cmp_storage_entry_release(storage, &storage->entries[i]);
    if (free_rc != CMP_OK && rc == CMP_OK) {
      rc = free_rc;
    }
  }

  free_rc = storage->allocator.free(storage->allocator.ctx, storage->entries);
  if (free_rc != CMP_OK && rc == CMP_OK) {
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

int CMP_CALL cmp_storage_put(CMPStorage *storage, const char *utf8_key,
                             cmp_usize key_len, const void *value,
                             cmp_usize value_len, CMPBool overwrite) {
  CMPStorageEntry *entry;
  cmp_usize index;
  CMPBool found;
  cmp_usize key_bytes;
  void *key_copy;
  void *value_copy;
  int rc;
  int free_rc;

  if (storage == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_storage_validate_bool(overwrite);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  rc = cmp_storage_validate_key(utf8_key, key_len);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  if (value == NULL && value_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_storage_find_entry(storage, utf8_key, key_len, &index, &found);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  if (found == CMP_TRUE) {
    if (overwrite == CMP_FALSE) {
      return CMP_ERR_BUSY;
    }

    entry = &storage->entries[index];
    if (value_len == 0) {
      if (entry->value != NULL) {
        rc = storage->allocator.free(storage->allocator.ctx, entry->value);
        if (rc != CMP_OK) {
          return rc;
        }
      }
      entry->value = NULL;
      entry->value_len = 0;
      return CMP_OK;
    }

    rc = storage->allocator.alloc(storage->allocator.ctx, value_len,
                                  &value_copy);
    if (rc != CMP_OK) {
      return rc;
    }
    memcpy(value_copy, value, (size_t)value_len);

    if (entry->value != NULL) {
      rc = storage->allocator.free(storage->allocator.ctx, entry->value);
      if (rc != CMP_OK) {
        storage->allocator.free(storage->allocator.ctx, value_copy);
        return rc;
      }
    }

    entry->value = value_copy;
    entry->value_len = value_len;
    return CMP_OK;
  }

  if (storage->entry_count >= storage->entry_capacity) {
    rc = cmp_storage_grow(storage, storage->entry_count + 1u);
    CMP_STORAGE_RETURN_IF_ERROR(rc);
  }

  rc = cmp_storage_add_overflow(key_len, (cmp_usize)1, &key_bytes);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  rc = storage->allocator.alloc(storage->allocator.ctx, key_bytes, &key_copy);
  if (rc != CMP_OK) {
    return rc;
  }

  memcpy(key_copy, utf8_key, (size_t)key_len);
  ((char *)key_copy)[key_len] = '\0';

  value_copy = NULL;
  if (value_len > 0) {
    rc = storage->allocator.alloc(storage->allocator.ctx, value_len,
                                  &value_copy);
    if (rc != CMP_OK) {
      free_rc = storage->allocator.free(storage->allocator.ctx, key_copy);
      CMP_UNUSED(free_rc);
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
  return CMP_OK;
}

int CMP_CALL cmp_storage_get(const CMPStorage *storage, const char *utf8_key,
                             cmp_usize key_len, void *out_value,
                             cmp_usize value_capacity,
                             cmp_usize *out_value_size) {
  cmp_usize index;
  CMPBool found;
  int rc;

  if (storage == NULL || out_value_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries == NULL) {
    return CMP_ERR_STATE;
  }
  if (out_value == NULL && value_capacity != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_storage_validate_key(utf8_key, key_len);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  rc = cmp_storage_find_entry(storage, utf8_key, key_len, &index, &found);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  if (found == CMP_FALSE) {
    *out_value_size = 0;
    return CMP_ERR_NOT_FOUND;
  }

  *out_value_size = storage->entries[index].value_len;
  if (value_capacity < storage->entries[index].value_len) {
    return CMP_ERR_RANGE;
  }

  if (storage->entries[index].value_len > 0) {
    memcpy(out_value, storage->entries[index].value,
           (size_t)storage->entries[index].value_len);
  }

  return CMP_OK;
}

int CMP_CALL cmp_storage_contains(const CMPStorage *storage,
                                  const char *utf8_key, cmp_usize key_len,
                                  CMPBool *out_exists) {
  cmp_usize index;
  CMPBool found;
  int rc;

  if (storage == NULL || out_exists == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_storage_validate_key(utf8_key, key_len);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  rc = cmp_storage_find_entry(storage, utf8_key, key_len, &index, &found);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  *out_exists = found;
  return CMP_OK;
}

int CMP_CALL cmp_storage_remove(CMPStorage *storage, const char *utf8_key,
                                cmp_usize key_len) {
  cmp_usize index;
  cmp_usize tail_count;
  CMPBool found;
  int rc;

  if (storage == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_storage_validate_key(utf8_key, key_len);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  rc = cmp_storage_find_entry(storage, utf8_key, key_len, &index, &found);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  if (found == CMP_FALSE) {
    return CMP_ERR_NOT_FOUND;
  }

  rc = cmp_storage_entry_release(storage, &storage->entries[index]);

  tail_count = storage->entry_count - index - 1u;
  if (tail_count > 0) {
    memmove(&storage->entries[index], &storage->entries[index + 1u],
            (size_t)(tail_count * sizeof(CMPStorageEntry)));
  }

  storage->entry_count -= 1u;
  memset(&storage->entries[storage->entry_count], 0, sizeof(CMPStorageEntry));
  return rc;
}

int CMP_CALL cmp_storage_clear(CMPStorage *storage) {
  cmp_usize i;
  int rc;
  int free_rc;

  if (storage == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries == NULL) {
    return CMP_ERR_STATE;
  }

  rc = CMP_OK;
  for (i = 0; i < storage->entry_count; ++i) {
    free_rc = cmp_storage_entry_release(storage, &storage->entries[i]);
    if (free_rc != CMP_OK && rc == CMP_OK) {
      rc = free_rc;
    }
  }

  memset(storage->entries, 0,
         (size_t)(storage->entry_capacity * sizeof(CMPStorageEntry)));
  storage->entry_count = 0;
  return rc;
}

int CMP_CALL cmp_storage_count(const CMPStorage *storage,
                               cmp_usize *out_count) {
  if (storage == NULL || out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries == NULL) {
    return CMP_ERR_STATE;
  }

  *out_count = storage->entry_count;
  return CMP_OK;
}

int CMP_CALL cmp_storage_save(const CMPStorage *storage, CMPIO *io,
                              const char *utf8_path) {
  cmp_u8 *buffer;
  cmp_u32 entry_count_u32;
  cmp_u32 key_len_u32;
  cmp_u32 value_len_u32;
  cmp_usize total_size;
  cmp_usize offset;
  int rc;
  int free_rc;
  cmp_usize i;

  if (storage == NULL || io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries == NULL) {
    return CMP_ERR_STATE;
  }
  if (io->vtable == NULL || io->vtable->write_file == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->allocator.alloc == NULL || storage->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_storage_u32_from_usize(storage->entry_count, &entry_count_u32);
  CMP_STORAGE_RETURN_IF_ERROR(rc);

  total_size = (cmp_usize)CMP_STORAGE_HEADER_SIZE;
  for (i = 0; i < storage->entry_count; ++i) {
    const CMPStorageEntry *entry = &storage->entries[i];

    if (entry->key == NULL || (entry->value == NULL && entry->value_len != 0)) {
      return CMP_ERR_STATE;
    }

    rc = cmp_storage_u32_from_usize(entry->key_len, &key_len_u32);
    CMP_STORAGE_RETURN_IF_ERROR(rc);
    rc = cmp_storage_u32_from_usize(entry->value_len, &value_len_u32);
    CMP_STORAGE_RETURN_IF_ERROR(rc);

    rc = cmp_storage_add_overflow(total_size, (cmp_usize)8, &total_size);
    CMP_STORAGE_RETURN_IF_ERROR(rc);
    rc = cmp_storage_add_overflow(total_size, entry->key_len, &total_size);
    CMP_STORAGE_RETURN_IF_ERROR(rc);
    rc = cmp_storage_add_overflow(total_size, entry->value_len, &total_size);
    CMP_STORAGE_RETURN_IF_ERROR(rc);
  }

  rc = storage->allocator.alloc(storage->allocator.ctx, total_size,
                                (void **)&buffer);
  if (rc != CMP_OK) {
    return rc;
  }

  buffer[0] = (cmp_u8)CMP_STORAGE_MAGIC_0;
  buffer[1] = (cmp_u8)CMP_STORAGE_MAGIC_1;
  buffer[2] = (cmp_u8)CMP_STORAGE_MAGIC_2;
  buffer[3] = (cmp_u8)CMP_STORAGE_MAGIC_3;

  cmp_storage_write_u32_le(buffer + 4, (cmp_u32)CMP_STORAGE_VERSION);
  cmp_storage_write_u32_le(buffer + 8, entry_count_u32);

  offset = (cmp_usize)CMP_STORAGE_HEADER_SIZE;
  for (i = 0; i < storage->entry_count; ++i) {
    const CMPStorageEntry *entry = &storage->entries[i];

    rc = cmp_storage_u32_from_usize(entry->key_len, &key_len_u32);
    if (rc != CMP_OK) {
      free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
      CMP_UNUSED(free_rc);
      return rc;
    }

    rc = cmp_storage_u32_from_usize(entry->value_len, &value_len_u32);
    if (rc != CMP_OK) {
      free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
      CMP_UNUSED(free_rc);
      return rc;
    }

    cmp_storage_write_u32_le(buffer + offset, key_len_u32);
    cmp_storage_write_u32_le(buffer + offset + 4u, value_len_u32);

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

  rc = io->vtable->write_file(io->ctx, utf8_path, buffer, total_size, CMP_TRUE);
  free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
  if (free_rc != CMP_OK && rc == CMP_OK) {
    rc = free_rc;
  }
  return rc;
}

int CMP_CALL cmp_storage_load(CMPStorage *storage, CMPIO *io,
                              const char *utf8_path) {
  CMPStorageConfig config;
  CMPStorage temp_storage;
  CMPStorage swap_storage;
  cmp_u8 *buffer;
  cmp_u32 version;
  cmp_u32 entry_count_u32;
  cmp_u32 key_len_u32;
  cmp_u32 value_len_u32;
  cmp_usize buffer_size;
  cmp_usize offset;
  cmp_usize i;
  int rc;
  int free_rc;

  if (storage == NULL || io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->entries == NULL) {
    return CMP_ERR_STATE;
  }
  if (io->vtable == NULL || io->vtable->read_file_alloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (storage->allocator.alloc == NULL || storage->allocator.free == NULL ||
      storage->allocator.realloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  buffer = NULL;
  buffer_size = 0;
  rc = io->vtable->read_file_alloc(io->ctx, utf8_path, &storage->allocator,
                                   (void **)&buffer, &buffer_size);
  if (rc != CMP_OK) {
    return rc;
  }
  if (buffer == NULL || buffer_size < (cmp_usize)CMP_STORAGE_HEADER_SIZE) {
    if (buffer != NULL) {
      free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
      CMP_UNUSED(free_rc);
    }
    return CMP_ERR_CORRUPT;
  }

  if (buffer[0] != (cmp_u8)CMP_STORAGE_MAGIC_0 ||
      buffer[1] != (cmp_u8)CMP_STORAGE_MAGIC_1 ||
      buffer[2] != (cmp_u8)CMP_STORAGE_MAGIC_2 ||
      buffer[3] != (cmp_u8)CMP_STORAGE_MAGIC_3) {
    free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
    CMP_UNUSED(free_rc);
    return CMP_ERR_CORRUPT;
  }

  cmp_storage_read_u32_le(buffer + 4, &version);

  if (version != (cmp_u32)CMP_STORAGE_VERSION) {
    free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
    CMP_UNUSED(free_rc);
    return CMP_ERR_UNSUPPORTED;
  }

  cmp_storage_read_u32_le(buffer + 8, &entry_count_u32);

  config.allocator = &storage->allocator;
  config.entry_capacity = entry_count_u32 == 0u
                              ? (cmp_usize)CMP_STORAGE_DEFAULT_CAPACITY
                              : (cmp_usize)entry_count_u32;
  memset(&temp_storage, 0, sizeof(temp_storage));

  rc = cmp_storage_init(&temp_storage, &config);
  if (rc != CMP_OK) {
    free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
    CMP_UNUSED(free_rc);
    return rc;
  }

  offset = (cmp_usize)CMP_STORAGE_HEADER_SIZE;
  for (i = 0; i < (cmp_usize)entry_count_u32; ++i) {
    if (offset + 8u > buffer_size) {
      rc = CMP_ERR_CORRUPT;
      break;
    }

    cmp_storage_read_u32_le(buffer + offset, &key_len_u32);
    cmp_storage_read_u32_le(buffer + offset + 4u, &value_len_u32);
    offset += 8u;

    if (key_len_u32 == 0u) {
      rc = CMP_ERR_CORRUPT;
      break;
    }
    if (offset + (cmp_usize)key_len_u32 + (cmp_usize)value_len_u32 >
        buffer_size) {
      rc = CMP_ERR_CORRUPT;
      break;
    }

    rc = cmp_storage_put(&temp_storage, (const char *)(buffer + offset),
                         (cmp_usize)key_len_u32, buffer + offset + key_len_u32,
                         (cmp_usize)value_len_u32, CMP_FALSE);
    if (rc != CMP_OK) {
      break;
    }

    offset += (cmp_usize)key_len_u32 + (cmp_usize)value_len_u32;
  }

  free_rc = storage->allocator.free(storage->allocator.ctx, buffer);
  if (free_rc != CMP_OK && rc == CMP_OK) {
    rc = free_rc;
  }

  if (rc != CMP_OK) {
    free_rc = cmp_storage_shutdown(&temp_storage);
    CMP_UNUSED(free_rc);
    return rc;
  }

  swap_storage = *storage;
  *storage = temp_storage;
  temp_storage = swap_storage;

  rc = cmp_storage_shutdown(&temp_storage);
  return rc;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_storage_test_mul_overflow(cmp_usize a, cmp_usize b,
                                           cmp_usize *out_value) {
  return cmp_storage_mul_overflow(a, b, out_value);
}

int CMP_CALL cmp_storage_test_add_overflow(cmp_usize a, cmp_usize b,
                                           cmp_usize *out_value) {
  return cmp_storage_add_overflow(a, b, out_value);
}

int CMP_CALL cmp_storage_test_grow(CMPStorage *storage,
                                   cmp_usize min_capacity) {
  return cmp_storage_grow(storage, min_capacity);
}

int CMP_CALL cmp_storage_test_u32_from_usize(cmp_usize value,
                                             cmp_u32 *out_value) {
  return cmp_storage_u32_from_usize(value, out_value);
}

#endif
