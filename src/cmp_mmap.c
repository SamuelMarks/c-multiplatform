/* clang-format off */
#include "cmp_mmap.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_mmap {
  void *dummy_data;
  size_t size;
};

/**
 * @brief Opens a file and memory-maps its contents.
 *
 * @param out_mmap Pointer to store the mapped instance.
 * @param filepath Path to the file.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mmap_open(cmp_mmap_t **out_mmap, const char *filepath) {
  int rc = CMP_SUCCESS;
  cmp_mmap_t *m = NULL;
  int free_rc;

  if (out_mmap == NULL || filepath == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mmap_open: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_mmap_t), (void **)&m);
  if (rc != CMP_SUCCESS || m == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_mmap_open: CMP_MALLOC failed (OOM)\n");
    return rc;
  }

  m->size = 1024;
  rc = CMP_MALLOC(1024, (void **)&m->dummy_data);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_mmap_open: CMP_MALLOC failed for data (OOM)\n");
    free_rc = CMP_FREE(m);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_mmap_open: CMP_FREE failed during cleanup\n");
    }
    rc = CMP_ERROR_OOM;
    return rc;
  }

  /* Mock filling the mapped file with a recognizable byte */
  memset(m->dummy_data, 42, 1024);

  *out_mmap = m;
  return rc;
}

/**
 * @brief Closes and unmaps a memory-mapped file.
 *
 * @param mmap The instance to close.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mmap_close(cmp_mmap_t *mmap) {
  int rc = CMP_SUCCESS;
  int free_rc;

  if (mmap == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mmap_close: Invalid argument\n");
    return rc;
  }

  if (mmap->dummy_data != NULL) {
    free_rc = CMP_FREE(mmap->dummy_data);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_mmap_close: CMP_FREE failed for dummy_data\n");
    }
  }

  rc = CMP_FREE(mmap);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_mmap_close: CMP_FREE failed for mmap\n");
    return rc;
  }

  return rc;
}

/**
 * @brief Retrieves the pointer to the mapped data and its size.
 *
 * @param mmap The instance.
 * @param out_data Pointer to store the mapped memory address.
 * @param out_size Pointer to store the size of the mapping in bytes.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_mmap_get_data(cmp_mmap_t *mmap, void **out_data, size_t *out_size) {
  int rc = CMP_SUCCESS;

  if (mmap == NULL || out_data == NULL || out_size == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mmap_get_data: Invalid argument\n");
    return rc;
  }

  *out_data = mmap->dummy_data;
  *out_size = mmap->size;
  return rc;
}
