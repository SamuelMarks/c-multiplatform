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

int cmp_mmap_open(cmp_mmap_t **out_mmap, const char *filepath) {
  int rc = CMP_SUCCESS;
  cmp_mmap_t *m = NULL;

  if (!out_mmap || !filepath) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mmap_open: Invalid argument\n");
    return rc;
  }

  m = (cmp_mmap_t *)malloc(sizeof(cmp_mmap_t));
  if (!m) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_mmap_open: Out of memory\n");
    return rc;
  }

  m->size = 1024;
  m->dummy_data = malloc(1024);
  if (!m->dummy_data) {
    free(m);
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_mmap_open: Out of memory for dummy data\n");
    return rc;
  }

  /* Mock filling the mapped file with a recognizable byte */
  memset(m->dummy_data, 42, 1024);

  *out_mmap = m;
  return rc;
}

int cmp_mmap_close(cmp_mmap_t *mmap) {
  int rc = CMP_SUCCESS;

  if (!mmap) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_mmap_close: Invalid argument\n");
    return rc;
  }
  if (mmap->dummy_data) {
    free(mmap->dummy_data);
  }
  free(mmap);
  return rc;
}

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
