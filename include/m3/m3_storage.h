#ifndef M3_STORAGE_H
#define M3_STORAGE_H

/**
 * @file m3_storage.h
 * @brief Key-value storage plugin for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"

/** @brief Default entry capacity for storage instances. */
#define M3_STORAGE_DEFAULT_CAPACITY 16u

/**
 * @brief Storage entry description.
 */
typedef struct M3StorageEntry {
    char *key; /**< UTF-8 key bytes (null-terminated copy). */
    m3_usize key_len; /**< Key length in bytes (excluding null terminator). */
    void *value; /**< Value bytes (may be NULL when value_len is 0). */
    m3_usize value_len; /**< Value length in bytes. */
} M3StorageEntry;

/**
 * @brief Storage configuration.
 */
typedef struct M3StorageConfig {
    const M3Allocator *allocator; /**< Allocator for storage memory; NULL uses default allocator. */
    m3_usize entry_capacity; /**< Initial entry capacity (> 0). */
} M3StorageConfig;

/**
 * @brief Storage instance.
 */
typedef struct M3Storage {
    M3Allocator allocator; /**< Allocator used for storage memory. */
    M3StorageEntry *entries; /**< Entry array (capacity-sized). */
    m3_usize entry_count; /**< Number of active entries. */
    m3_usize entry_capacity; /**< Allocated entry capacity. */
} M3Storage;

/**
 * @brief Initialize a storage configuration with defaults.
 * @param config Configuration to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_config_init(M3StorageConfig *config);

/**
 * @brief Initialize a storage instance.
 * @param storage Storage instance to initialize.
 * @param config Storage configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_init(M3Storage *storage, const M3StorageConfig *config);

/**
 * @brief Shut down a storage instance and release resources.
 * @param storage Storage instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_shutdown(M3Storage *storage);

/**
 * @brief Insert or update a key/value pair.
 * @param storage Storage instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param value Value bytes (may be NULL when value_len is 0).
 * @param value_len Value length in bytes.
 * @param overwrite M3_TRUE to overwrite existing values.
 * @return M3_OK on success, M3_ERR_BUSY if the key exists and overwrite is false, or a failure code.
 */
M3_API int M3_CALL m3_storage_put(M3Storage *storage, const char *utf8_key, m3_usize key_len, const void *value,
    m3_usize value_len, M3Bool overwrite);

/**
 * @brief Retrieve a value into a caller-provided buffer.
 * @param storage Storage instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param out_value Destination buffer (may be NULL when value_capacity is 0).
 * @param value_capacity Size of the destination buffer in bytes.
 * @param out_value_size Receives the stored value length in bytes.
 * @return M3_OK on success, M3_ERR_NOT_FOUND if missing, M3_ERR_RANGE if the buffer is too small, or a failure code.
 */
M3_API int M3_CALL m3_storage_get(const M3Storage *storage, const char *utf8_key, m3_usize key_len, void *out_value,
    m3_usize value_capacity, m3_usize *out_value_size);

/**
 * @brief Check whether a key exists in storage.
 * @param storage Storage instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param out_exists Receives M3_TRUE if the key exists.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_contains(const M3Storage *storage, const char *utf8_key, m3_usize key_len, M3Bool *out_exists);

/**
 * @brief Remove a key/value pair.
 * @param storage Storage instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @return M3_OK on success, M3_ERR_NOT_FOUND if missing, or a failure code.
 */
M3_API int M3_CALL m3_storage_remove(M3Storage *storage, const char *utf8_key, m3_usize key_len);

/**
 * @brief Remove all key/value pairs.
 * @param storage Storage instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_clear(M3Storage *storage);

/**
 * @brief Get the number of stored entries.
 * @param storage Storage instance.
 * @param out_count Receives the entry count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_count(const M3Storage *storage, m3_usize *out_count);

/**
 * @brief Save storage contents to a file using the provided IO backend.
 * @param storage Storage instance.
 * @param io IO backend.
 * @param utf8_path Output file path in UTF-8.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_save(const M3Storage *storage, M3IO *io, const char *utf8_path);

/**
 * @brief Load storage contents from a file produced by m3_storage_save.
 * @param storage Storage instance.
 * @param io IO backend.
 * @param utf8_path Input file path in UTF-8.
 * @return M3_OK on success, M3_ERR_CORRUPT for invalid data, M3_ERR_UNSUPPORTED for version mismatch, or a failure code.
 */
M3_API int M3_CALL m3_storage_load(M3Storage *storage, M3IO *io, const char *utf8_path);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for overflow-safe multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_test_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value);

/**
 * @brief Test wrapper for overflow-safe addition.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives sum.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_test_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value);

/**
 * @brief Test wrapper for converting sizes to u32.
 * @param value Input value.
 * @param out_value Receives converted value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_test_u32_from_usize(m3_usize value, m3_u32 *out_value);

/**
 * @brief Test wrapper for growing storage capacity.
 * @param storage Storage instance.
 * @param min_capacity Minimum required capacity.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_storage_test_grow(M3Storage *storage, m3_usize min_capacity);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_STORAGE_H */
