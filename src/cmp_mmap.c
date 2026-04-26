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
 * @brief cmp_mmap_open
 *
 * @param out_mmap Parameter description.
 * @param filepath Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mmap_open(cmp_mmap_t **out_mmap, const char *filepath) {
  int rc = CMP_SUCCESS;
  cmp_mmap_t *m = NULL;

  if (!out_mmap || !filepath) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mmap_open: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_mmap_t), (void **)&(m));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  m->size = 1024;
  rc = CMP_MALLOC(1024, (void **)&(m->dummy_data));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  /* Mock filling the mapped file with a recognizable byte */
  memset(m->dummy_data, 42, 1024);

  *out_mmap = m;
  return rc;
}

/**
 * @brief cmp_mmap_close
 *
 * @param mmap Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mmap_close(cmp_mmap_t *mmap) {
  int rc = CMP_SUCCESS;

  if (!mmap) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mmap_close: Invalid argument\n");
    return rc;
  }
  if (mmap->dummy_data) {
    rc = CMP_FREE(mmap->dummy_data);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  rc = CMP_FREE(mmap);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return rc;
}

/**
 * @brief cmp_mmap_get_data
 *
 * @param mmap Parameter description.
 * @param out_data Parameter description.
 * @param out_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mmap_get_data(cmp_mmap_t *mmap, void **out_data, size_t *out_size) {
  int rc = CMP_SUCCESS;

  if (!mmap || !out_data || !out_size) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mmap_get_data: Invalid argument\n");
    return rc;
  }
  *out_data = mmap->dummy_data;
  *out_size = mmap->size;
  return rc;
}
