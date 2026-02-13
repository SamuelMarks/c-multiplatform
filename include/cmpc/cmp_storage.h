#ifndef CMP_STORAGE_H
#define CMP_STORAGE_H

/**
 * @file cmp_storage.h
 * @brief Key-value storage plugin for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"

/** @brief Default entry capacity for storage instances. */
#define CMP_STORAGE_DEFAULT_CAPACITY 16u

/**
 * @brief Storage entry description.
 */
typedef struct CMPStorageEntry {
  char *key;           /**< UTF-8 key bytes (null-terminated copy). */
  cmp_usize key_len;   /**< Key length in bytes (excluding null terminator). */
  void *value;         /**< Value bytes (may be NULL when value_len is 0). */
  cmp_usize value_len; /**< Value length in bytes. */
} CMPStorageEntry;

/**
 * @brief Storage configuration.
 */
typedef struct CMPStorageConfig {
  const CMPAllocator *allocator; /**< Allocator for storage memory; NULL uses
                                   default allocator. */
  cmp_usize entry_capacity;      /**< Initial entry capacity (> 0). */
} CMPStorageConfig;

/**
 * @brief Storage instance.
 */
typedef struct CMPStorage {
  CMPAllocator allocator;   /**< Allocator used for storage memory. */
  CMPStorageEntry *entries; /**< Entry array (capacity-sized). */
  cmp_usize entry_count;    /**< Number of active entries. */
  cmp_usize entry_capacity; /**< Allocated entry capacity. */
} CMPStorage;

/**
 * @brief Initialize a storage configuration with defaults.
 * @param config Configuration to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_config_init(CMPStorageConfig *config);

/**
 * @brief Initialize a storage instance.
 * @param storage Storage instance to initialize.
 * @param config Storage configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_init(CMPStorage *storage,
                                      const CMPStorageConfig *config);

/**
 * @brief Shut down a storage instance and release resources.
 * @param storage Storage instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_shutdown(CMPStorage *storage);

/**
 * @brief Insert or update a key/value pair.
 * @param storage Storage instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param value Value bytes (may be NULL when value_len is 0).
 * @param value_len Value length in bytes.
 * @param overwrite CMP_TRUE to overwrite existing values.
 * @return CMP_OK on success, CMP_ERR_BUSY if the key exists and overwrite is
 * false, or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_put(CMPStorage *storage, const char *utf8_key,
                                     cmp_usize key_len, const void *value,
                                     cmp_usize value_len, CMPBool overwrite);

/**
 * @brief Retrieve a value into a caller-provided buffer.
 * @param storage Storage instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param out_value Destination buffer (may be NULL when value_capacity is 0).
 * @param value_capacity Size of the destination buffer in bytes.
 * @param out_value_size Receives the stored value length in bytes.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if missing, CMP_ERR_RANGE if the
 * buffer is too small, or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_get(const CMPStorage *storage,
                                     const char *utf8_key, cmp_usize key_len,
                                     void *out_value, cmp_usize value_capacity,
                                     cmp_usize *out_value_size);

/**
 * @brief Check whether a key exists in storage.
 * @param storage Storage instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param out_exists Receives CMP_TRUE if the key exists.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_contains(const CMPStorage *storage,
                                          const char *utf8_key,
                                          cmp_usize key_len,
                                          CMPBool *out_exists);

/**
 * @brief Remove a key/value pair.
 * @param storage Storage instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if missing, or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_remove(CMPStorage *storage,
                                        const char *utf8_key,
                                        cmp_usize key_len);

/**
 * @brief Remove all key/value pairs.
 * @param storage Storage instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_clear(CMPStorage *storage);

/**
 * @brief Get the number of stored entries.
 * @param storage Storage instance.
 * @param out_count Receives the entry count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_count(const CMPStorage *storage,
                                       cmp_usize *out_count);

/**
 * @brief Save storage contents to a file using the provided IO backend.
 * @param storage Storage instance.
 * @param io IO backend.
 * @param utf8_path Output file path in UTF-8.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_save(const CMPStorage *storage, CMPIO *io,
                                      const char *utf8_path);

/**
 * @brief Load storage contents from a file produced by cmp_storage_save.
 * @param storage Storage instance.
 * @param io IO backend.
 * @param utf8_path Input file path in UTF-8.
 * @return CMP_OK on success, CMP_ERR_CORRUPT for invalid data,
 * CMP_ERR_UNSUPPORTED for version mismatch, or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_load(CMPStorage *storage, CMPIO *io,
                                      const char *utf8_path);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for overflow-safe multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_test_mul_overflow(cmp_usize a, cmp_usize b,
                                                   cmp_usize *out_value);

/**
 * @brief Test wrapper for overflow-safe addition.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives sum.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_test_add_overflow(cmp_usize a, cmp_usize b,
                                                   cmp_usize *out_value);

/**
 * @brief Test wrapper for converting sizes to u32.
 * @param value Input value.
 * @param out_value Receives converted value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_test_u32_from_usize(cmp_usize value,
                                                     cmp_u32 *out_value);

/**
 * @brief Test wrapper for growing storage capacity.
 * @param storage Storage instance.
 * @param min_capacity Minimum required capacity.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_storage_test_grow(CMPStorage *storage,
                                           cmp_usize min_capacity);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_STORAGE_H */
