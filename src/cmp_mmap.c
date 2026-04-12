/* clang-format off */
#include "cmp_mmap.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_mmap {
  void *dummy_data;
  size_t size;
};

int cmp_mmap_open(cmp_mmap_t **out_mmap, const char *filepath) {
  cmp_mmap_t *m;
  if (!out_mmap || !filepath) {
    return -1;
  }

  m = (cmp_mmap_t *)malloc(sizeof(cmp_mmap_t));
  if (!m) {
    return -2;
  }

  m->size = 1024;
  m->dummy_data = malloc(1024);
  if (!m->dummy_data) {
    free(m);
    return -2;
  }

  /* Mock filling the mapped file with a recognizable byte */
  memset(m->dummy_data, 42, 1024);

  *out_mmap = m;
  return 0;
}

int cmp_mmap_close(cmp_mmap_t *mmap) {
  if (!mmap) {
    return -1;
  }
  if (mmap->dummy_data) {
    free(mmap->dummy_data);
  }
  free(mmap);
  return 0;
}

int cmp_mmap_get_data(cmp_mmap_t *mmap, void **out_data, size_t *out_size) {
  if (!mmap || !out_data || !out_size) {
    return -1;
  }
  *out_data = mmap->dummy_data;
  *out_size = mmap->size;
  return 0;
}
