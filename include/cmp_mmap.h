#ifndef CMP_MMAP_H
#define CMP_MMAP_H

/* clang-format off */
#include "cmp_ffi.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> /* For size_t */
/* clang-format on */

/**
 * @file cmp_mmap.h
 * @brief Cross-platform memory-mapped file primitives.
 */

typedef struct cmp_mmap cmp_mmap_t;

/**
 * @brief Opens a file and memory-maps its contents.
 * @param out_mmap Pointer to receive the mapped instance.
 * @param filepath Path to the file.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_mmap_open(cmp_mmap_t **out_mmap, const char *filepath);

/**
 * @brief Closes and unmaps a memory-mapped file.
 * @param mmap The instance to close.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_mmap_close(cmp_mmap_t *mmap);

/**
 * @brief Retrieves the pointer to the mapped data and its size.
 * @param mmap The instance.
 * @param out_data Pointer to receive the mapped memory address.
 * @param out_size Pointer to receive the size of the mapping in bytes.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_mmap_get_data(cmp_mmap_t *mmap, void **out_data,
                              size_t *out_size);
#ifdef __cplusplus
}
#endif

#endif /* CMP_MMAP_H */
